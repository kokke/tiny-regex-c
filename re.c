/*
 *
 * Mini regex-module inspired by Rob Pike's regex code described in:
 *
 * http://www.cs.princeton.edu/courses/archive/spr09/cos333/beautiful.html
 *
 *
 *
 * Supports:
 * ---------
 *   '.'        Dot, matches any character
 *   '^'        Start anchor, matches beginning of string
 *   '$'        End anchor, matches end of string
 *   '*'        Asterisk, match zero or more (greedy)
 *   '+'        Plus, match one or more (greedy)
 *   '?'        Question, match zero or one (non-greedy)
 *   '[abc]'    Character class, match if one of {'a', 'b', 'c'}
 *   '[^abc]'   Inverted class, match if NOT one of {'a', 'b', 'c'} -- NOTE: feature is currently broken!
 *   '[a-zA-Z]' Character ranges, the character set of the ranges { a-z | A-Z }
 *   '\s'       Whitespace, \t \f \r \n \v and spaces
 *   '\S'       Non-whitespace
 *   '\w'       Alphanumeric, [a-zA-Z0-9_]
 *   '\W'       Non-alphanumeric
 *   '\d'       Digits, [0-9]
 *   '\D'       Non-digits
 *
 *
 */



#include "re.h"
#include <stdio.h>
//#include <stdlib.h>

/* Definitions: */

#define MAX_REGEXP_OBJECTS      30    /* Max number of regex symbols in expression. */
#define MAX_CHAR_CLASS_LEN      40    /* Max length of character-class buffer in.   */
#define MAX_QUANT              255    /* Max b in {a,b}. 255 since a & b are char   */


#define X_RE_TYPES  X(UNUSED) X(DOT) X(BEGIN) X(END) X(QUANTIFIER) X(QUESTIONMARK) X(STAR) X(PLUS) \
        X(CHAR) X(CHAR_CLASS) X(INV_CHAR_CLASS) X(DIGIT) X(NOT_DIGIT) X(ALPHA) X(NOT_ALPHA) \
        X(WHITESPACE) X(NOT_WHITESPACE) X(BRANCH)

#define X(A) A,
enum { X_RE_TYPES };
#undef X

#define X(A) #A,
static const char* types[] = { X_RE_TYPES };
#undef X


typedef struct regex_t
{
  unsigned char  type;   /* CHAR, STAR, etc.                      */
  union
  {
    unsigned char  ch;   /*      the character itself             */
    unsigned char* ccl;  /*  OR  a pointer to characters in class */
  };
} regex_t;



/* Private function declarations: */
static int matchpattern(regex_t* pattern, const char* text);
static int matchcharclass(char c, const unsigned char* str);
static int matchstar(regex_t p, regex_t* pattern, const char* text);
static int matchplus(regex_t p, regex_t* pattern, const char* text);
static int matchone(regex_t p, char c);
static int matchdigit(char c);
static int matchalpha(char c);
static int matchwhitespace(char c);
static int matchmetachar(char c, const unsigned char* str);
//static int matchrange(char c, const char* str);
//static int ismetachar(char c);



/* Public functions: */
int re_match(const char* pattern, const char* text)
{
  re_t rec = re_compile(pattern);
  if(!rec)
  {
    printf("compiling pattern '%s' failed\n", pattern);
    return -1;
  }
  return re_matchp(rec, text);
}

int re_matchp(re_t pattern, const char* text)
{
  if (pattern != 0)
  {
    if (pattern[0].type == BEGIN)
    {
      return ((matchpattern(&pattern[1], text)) ? 0 : -1);
    }
    else
    {
      int idx = -1;

      do
      {
        idx += 1;
        
        if (matchpattern(pattern, text))
        {
          if (text[0] == '\0')
            return -1;
        
          return idx;
        }
      }
      while (*text++ != '\0');
    }
  }
  return -1;
}

re_t re_compile(const char* pattern)
{
  /* The sizes of the two static arrays below substantiates the static RAM usage of this module.
     MAX_REGEXP_OBJECTS is the max number of symbols in the expression.
     MAX_CHAR_CLASS_LEN determines the size of buffer for chars in all char-classes in the expression. */
  static regex_t re_compiled[MAX_REGEXP_OBJECTS];
  static unsigned char ccl_buf[MAX_CHAR_CLASS_LEN];
  int ccl_bufidx = 1;

  char c;     /* current char in pattern   */
  int i = 0;  /* index into pattern        */
  int j = 0;  /* index into re_compiled    */

  while (pattern[i] != '\0' && (j+1 < MAX_REGEXP_OBJECTS))
  {
    c = pattern[i];

    switch (c)
    {
      /* Meta-characters: */
      case '^': {    re_compiled[j].type = BEGIN;           } break;
      case '$': {    re_compiled[j].type = END;             } break;
      case '.': {    re_compiled[j].type = DOT;             } break;
      case '*': {    re_compiled[j].type = STAR;            } break;
      case '+': {    re_compiled[j].type = PLUS;            } break;
      case '?': {    re_compiled[j].type = QUESTIONMARK;    } break;
/*    case '|': {    re_compiled[j].type = BRANCH;          } break; <-- not working properly */

      /* Escaped character-classes (\s \w ...): */
      case '\\':
      {
        i++;
        if (pattern[i] == '\0')
        {
          // dangling '\'
          return 0;
        }
        
        switch (pattern[i])
        {
          /* Meta-character: */
          case 'd': {    re_compiled[j].type = DIGIT;            } break;
          case 'D': {    re_compiled[j].type = NOT_DIGIT;        } break;
          case 'w': {    re_compiled[j].type = ALPHA;            } break;
          case 'W': {    re_compiled[j].type = NOT_ALPHA;        } break;
          case 's': {    re_compiled[j].type = WHITESPACE;       } break;
          case 'S': {    re_compiled[j].type = NOT_WHITESPACE;   } break;

          /* Escaped character, e.g. '.' or '$' */ 
          default:  
          {
            re_compiled[j].type = CHAR;
            re_compiled[j].ch = pattern[i];
          } break;
        }
      } break;

      /* Character class: */
      case '[':
      {
        /* Remember where the char-buffer starts. */
        int buf_begin = ccl_bufidx;

        /* Look-ahead to determine if negated */
        if (pattern[i+1] == '^')
        {
          re_compiled[j].type = INV_CHAR_CLASS;
          i += 1; /* Increment i to avoid including '^' in the char-buffer */
        }  
        else
        {
          re_compiled[j].type = CHAR_CLASS;
        }

        /* Copy characters inside [..] to buffer */
        while (    (pattern[++i] != ']')
                && (pattern[i]   != '\0')) /* Missing ] */
        {
          if (pattern[i] == '\\')
          {
            if (ccl_bufidx >= MAX_CHAR_CLASS_LEN - 2 || pattern[i + 1] == '\0')
            {
              //fputs("exceeded internal buffer!\n", stderr);
              return 0;
            }
            // probably we can strip leading '\' for regular chars
            ccl_buf[ccl_bufidx++] = pattern[i++];
          }
          else if (ccl_bufidx >= MAX_CHAR_CLASS_LEN - 1)
          {
              //fputs("exceeded internal buffer!\n", stderr);
              return 0;
          }
          ccl_buf[ccl_bufidx++] = pattern[i];
        }
        if (pattern[i] != ']') // pattern[i] == '\0'
        {
          printf("char class missing ] at %i\n", i);
          return 0;
        }
        /* Null-terminate string end */
        ccl_buf[ccl_bufidx++] = 0;
        re_compiled[j].ccl = &ccl_buf[buf_begin];
      } break;

      /* Quantifier: */
      case '{':
      {
        re_compiled[j].type = QUANTIFIER;
        // consumes 2 chars (one char for each min and max <= 255)
        if (ccl_bufidx >= MAX_CHAR_CLASS_LEN - 1)
        {
            return 0;
        }
        i++;
        unsigned val = 0;
        do
        {
          if (pattern[i] < '0' || pattern[i] > '9')
            return 0;
          val = 10 * val + (pattern[i++] - '0');
        } while (pattern[i] != ',' && pattern[i] != '}');
        if (val > MAX_QUANT)
        {
            return 0;
        }
        ccl_buf[ccl_bufidx] = val;
        if (pattern[i] == ',')
        {
          i++;
          if (pattern[i] == '}')
          {
            val = MAX_QUANT;
          }
          else
          {
            val = 0;
            while (pattern[i] != '}')
            {
              if (pattern[i] < '0' || pattern[i] > '9')
                return 0;
              val = 10 * val + (pattern[i++] - '0');
            }

            if (val > MAX_QUANT || val < ccl_buf[ccl_bufidx])
            {
                return 0;
            }
          }
        }
        ccl_buf[ccl_bufidx + 1] = val;
        re_compiled[j].ccl = &ccl_buf[ccl_bufidx];
        ccl_bufidx += 2;
      } break;

      /* Other characters: */
      default:
      {
        re_compiled[j].type = CHAR;
        re_compiled[j].ch = c;
      } break;
    }
    i += 1;
    j += 1;
  }
  /* 'UNUSED' is a sentinel used to indicate end-of-pattern */
  re_compiled[j].type = UNUSED;

  return (re_t) re_compiled;
}

void re_print(regex_t* pattern)
{
  if(!pattern)
  {
    printf("NULL pattern detected\n");
    return;
  }
  int i;
  for (i = 0; i < MAX_REGEXP_OBJECTS; ++i)
  {
    if (pattern[i].type == UNUSED)
    {
      break;
    }

    printf("type: %s", types[pattern[i].type]);
    if (pattern[i].type == CHAR_CLASS || pattern[i].type == INV_CHAR_CLASS)
    {
      printf(" [");
      int j;
      char c;
      for (j = 0; j < MAX_CHAR_CLASS_LEN; ++j)
      {
        c = pattern[i].ccl[j];
        if ((c == '\0') || (c == ']'))
        {
          break;
        }
        printf("%c", c);
      }
      printf("]");
    }
    else if (pattern[i].type == QUANTIFIER)
    {
      printf(" {%d,%d}", pattern[i].ccl[0], pattern[i].ccl[1]);
    }
    else if (pattern[i].type == CHAR)
    {
      printf(" '%c'", pattern[i].ch);
    }
    printf("\n");
  }
}



/* Private functions: */
static int matchdigit(char c)
{
  return ((c >= '0') && (c <= '9'));
}
static int matchalpha(char c)
{
  return ((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z'));
}
static int matchwhitespace(char c)
{
  return ((c == ' ') || (c == '\t') || (c == '\n') || (c == '\r') || (c == '\f') || (c == '\v'));
}
static int matchalphanum(char c)
{
  return ((c == '_') || matchalpha(c) || matchdigit(c));
}

/*
static int matchrange(char c, const char* str)
{
  return ((c != '-') && (str[0] != '\0') && (str[0] != '-') &&
         (str[1] == '-') && (str[1] != '\0') &&
         (str[2] != '\0') && ((c >= str[0]) && (c <= str[2])));
}
static int ismetachar(char c)
{
  return ((c == 's') || (c == 'S') || (c == 'w') || (c == 'W') || (c == 'd') || (c == 'D'));
}
*/

static int matchmetachar(char c, const unsigned char* str)
{
  switch (str[0])
  {
    case 'd': return  matchdigit(c);
    case 'D': return !matchdigit(c);
    case 'w': return  matchalphanum(c);
    case 'W': return !matchalphanum(c);
    case 's': return  matchwhitespace(c);
    case 'S': return !matchwhitespace(c);
    default:  return (c == str[0]);
  }
}

static int matchcharclass(char c, const unsigned char* str)
{
  while (*str != '\0')
  {
    if (*str == '\\')
    {
      // if (str[1] == '\0') { return 0; } // shouldn't happen; compiling would also fail
      if (matchmetachar(c, str + 1))
      {
        return 1;
      }
      str += 2;
    }
    else if (str[1] == '-' && str[2] != '\0')
    {
      if(c >= str[0] && c <= str[2])
      {
        return 1;
      }
      str += 3;
    }
    else
    {
      if (c == *str)
      {
        return 1;
      }
      str += 1;
    }
  }

  return 0;
}

static int matchone(regex_t p, char c)
{
  switch (p.type)
  {
    case DOT:            return 1;
    case CHAR_CLASS:     return  matchcharclass(c, p.ccl);
    case INV_CHAR_CLASS: return !matchcharclass(c, p.ccl);
    case DIGIT:          return  matchdigit(c);
    case NOT_DIGIT:      return !matchdigit(c);
    case ALPHA:          return  matchalphanum(c);
    case NOT_ALPHA:      return !matchalphanum(c);
    case WHITESPACE:     return  matchwhitespace(c);
    case NOT_WHITESPACE: return !matchwhitespace(c);
    default:             return  (p.ch == c);
  }
}

static int matchstar(regex_t p, regex_t* pattern, const char* text)
{
  do
  {
    if (matchpattern(pattern, text))
      return 1;
  }
  while ((text[0] != '\0') && matchone(p, *text++));

  return 0;
}

static int matchplus(regex_t p, regex_t* pattern, const char* text)
{
  while ((text[0] != '\0') && matchone(p, *text++))
  {
    if (matchpattern(pattern, text))
      return 1;
  }
  return 0;
}

static int matchquestion(regex_t p, regex_t* pattern, const char* text)
{
  if (p.type == UNUSED)
    return 1;
  if (matchpattern(pattern, text))
      return 1;
  if (*text && matchone(p, *text++))
    return matchpattern(pattern, text);
  return 0;
}

static int matchquantifier(regex_t p, regex_t* pattern, const char* text, int min, int max)
{
  max -= min;
  while (min > 0 && *text && matchone(p, *text++))
  {
    min--;
  }
  if (min > 0)
    return 0;
  do
  {
    if (matchpattern(pattern, text))
      return 1;
    max--;
  }
  while (max >= 0 && *text && matchone(p, *text++));

  return 0;
}

#if 0

/* Recursive matching */
static int matchpattern(regex_t* pattern, const char* text)
{
  if ((pattern[0].type == UNUSED) || (pattern[1].type == QUESTIONMARK))
  {
    return matchquestion(pattern[1], &pattern[2], text);
  }
  else if (pattern[1].type == STAR)
  {
    return matchstar(pattern[0], &pattern[2], text);
  }
  else if (pattern[1].type == PLUS)
  {
    return matchplus(pattern[0], &pattern[2], text);
  }
  else if ((pattern[0].type == END) && pattern[1].type == UNUSED)
  {
    return text[0] == '\0';
  }
  else if ((text[0] != '\0') && matchone(pattern[0], text[0]))
  {
    return matchpattern(&pattern[1], text+1);
  }
  else
  {
    return 0;
  }
}

#else

/* Iterative matching */
static int matchpattern(regex_t* pattern, const char* text)
{
  do
  {
    if ((pattern[0].type == UNUSED) || (pattern[1].type == QUESTIONMARK))
    {
      return matchquestion(pattern[0], &pattern[2], text);
    }
    else if (pattern[1].type == QUANTIFIER)
    {
      unsigned min = pattern[1].ccl[0];
      unsigned max = pattern[1].ccl[1];
      return matchquantifier(pattern[0], &pattern[2], text, min, max);
    }
    else if (pattern[1].type == STAR)
    {
      return matchstar(pattern[0], &pattern[2], text);
    }
    else if (pattern[1].type == PLUS)
    {
      return matchplus(pattern[0], &pattern[2], text);
    }
    else if ((pattern[0].type == END) && pattern[1].type == UNUSED)
    {
      return (text[0] == '\0');
    }
/*  Branching is not working properly
    else if (pattern[1].type == BRANCH)
    {
      return (matchpattern(pattern, text) || matchpattern(&pattern[2], text));
    }
*/
  }
  while ((text[0] != '\0') && matchone(*pattern++, *text++));

  return 0;
}

#endif
