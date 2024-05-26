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
 *   '[^abc]'   Inverted class, match if NOT one of {'a', 'b', 'c'}
 *   '[a-zA-Z]' Character ranges, the character set of the ranges { a-z | A-Z }
 *   '\s'       Whitespace, \t \f \r \n \v and spaces
 *   '\S'       Non-whitespace
 *   '\w'       Alphanumeric, [a-zA-Z0-9_]
 *   '\W'       Non-alphanumeric
 *   '\d'       Digits, [0-9]
 *   '\D'       Non-digits
 *   '|'        Branch, matches either the preceding or following pattern
 *
 *
 */



#include "re.h"
#include <stdio.h>
#include <ctype.h>

/* Definitions: */

#define MAX_REGEXP_LEN      70    /* Max number of bytes for a regex. */


enum regex_type_e { UNUSED, DOT, BEGIN, END, QUESTIONMARK, STAR, PLUS, CHAR, CHAR_CLASS, INV_CHAR_CLASS, DIGIT, NOT_DIGIT, ALPHA, NOT_ALPHA, WHITESPACE, NOT_WHITESPACE, /* BRANCH */ };

typedef struct regex_t
{
  unsigned char type;    /* CHAR, STAR, etc.                      */
  unsigned char data_len;
  unsigned char data[0];
} regex_t;

static re_t getnext(regex_t* pattern)
{
  return (re_t)(((unsigned char*)pattern) + 2 + pattern->data_len);
}



/* Private function declarations: */
static int matchpattern(regex_t* pattern, const char* text, int* matchlength);
static int matchcharclass(char c, unsigned char len, const char* str);
static int matchstar(regex_t *p, regex_t* pattern, const char* text, int* matchlength);
static int matchplus(regex_t *p, regex_t* pattern, const char* text, int* matchlength);
static int matchone(regex_t* p, char c);
static int matchdigit(char c);
static int matchalpha(char c);
static int matchwhitespace(char c);
static int matchmetachar(char c, const char* str);
static int matchrange(char c, const char* str);
static int matchdot(char c);
static int ismetachar(char c);



/* Public functions: */
int re_match(const char* pattern, const char* text, int* matchlength)
{
  return re_matchp(re_compile(pattern), text, matchlength);
}

int re_matchp(re_t pattern, const char* text, int* matchlength)
{
  *matchlength = 0;
  if (pattern != 0)
  {
    if (pattern->type == BEGIN)
    {
      return ((matchpattern(getnext(pattern), text, matchlength)) ? 0 : -1);
    }
    else
    {
      int idx = -1;

      do
      {
        idx += 1;

        if (matchpattern(pattern, text, matchlength))
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

static int min(int a, int b)
{
  return (a <= b) ? a : b;
}

re_t re_compile(const char* pattern)
{
  /* The size of this static array substantiates the static RAM usage of this module.
     MAX_REGEXP_LEN is the max number number of bytes in the expression. */
  static unsigned char re_data[MAX_REGEXP_LEN];

  char c;     /* current char in pattern   */
  int i = 0;  /* index into pattern        */
  int j = 0;  /* index into re_data    */

  while (pattern[i] != '\0' && (j+3 < MAX_REGEXP_LEN))
  {
    c = pattern[i];
    regex_t *re_compiled = (regex_t*)(re_data+j);
    re_compiled->data_len = 0;

    switch (c)
    {
      /* Meta-characters: */
      case '^': {    re_compiled->type = BEGIN;           } break;
      case '$': {    re_compiled->type = END;             } break;
      case '.': {    re_compiled->type = DOT;             } break;
      case '*': {    re_compiled->type = STAR;            } break;
      case '+': {    re_compiled->type = PLUS;            } break;
      case '?': {    re_compiled->type = QUESTIONMARK;    } break;
/*    case '|': {    re_compiled->type = BRANCH;          } break; <-- not working properly */

      /* Escaped character-classes (\s \w ...): */
      case '\\':
      {
        if (pattern[i+1] != '\0')
        {
          /* Skip the escape-char '\\' */
          i += 1;
          /* ... and check the next */
          switch (pattern[i])
          {
            /* Meta-character: */
            case 'd': {    re_compiled->type = DIGIT;            } break;
            case 'D': {    re_compiled->type = NOT_DIGIT;        } break;
            case 'w': {    re_compiled->type = ALPHA;            } break;
            case 'W': {    re_compiled->type = NOT_ALPHA;        } break;
            case 's': {    re_compiled->type = WHITESPACE;       } break;
            case 'S': {    re_compiled->type = NOT_WHITESPACE;   } break;

              /* Escaped character, e.g. '.', '$' or '\\' */
            default:
            {
              re_compiled->type = CHAR;
              re_compiled->data_len = 1;
              re_compiled->data[0] = pattern[i];
            } break;
          }
        }
        /* '\\' as last char without previous \\ -> invalid regular expression. */
        else
        {
          return 0;
        }
      } break;

      /* Character class: */
      case '[':
      {
        int char_limit = min(0xff, MAX_REGEXP_LEN - j - 4); // 4 for this object and UNUSED at the minimum

        /* Look-ahead to determine if negated */
        if (pattern[i+1] == '^')
        {
          re_compiled->type = INV_CHAR_CLASS;
          i += 1; /* Increment i to avoid including '^' in the char-buffer */
          if (pattern[i+1] == 0) /* incomplete pattern, missing non-zero char after '^' */
          {
            return 0;
          }
        }
        else
        {
          re_compiled->type = CHAR_CLASS;
        }

        /* Copy characters inside [..] to buffer */
        while (    (pattern[++i] != ']')
                && (pattern[i]   != '\0')) /* Missing ] */
        {
          if (pattern[i] == '\\')
          {
            if (re_compiled->data_len >= char_limit)
            {
              //fputs("exceeded internal buffer!\n", stderr);
              return 0;
            }
            if (pattern[i+1] == 0) /* incomplete pattern, missing non-zero char after '\\' */
            {
              return 0;
            }
            re_compiled->data[re_compiled->data_len++] = pattern[i++];
          }
          // TODO: I think this "else if" is a bug, should just be "if"
          else if (re_compiled->data_len >= char_limit)
          {
              //fputs("exceeded internal buffer!\n", stderr);
              return 0;
          }
          re_compiled->data[re_compiled->data_len++] = pattern[i];
        }
        if (re_compiled->data_len >= char_limit)
        {
            /* Catches cases such as [00000000000000000000000000000000000000][ */
            //fputs("exceeded internal buffer!\n", stderr);
            return 0;
        }
      } break;

      case '\0': // EOL
        return 0;

      /* Other characters: */
      default:
      {
        re_compiled->type = CHAR;
        re_compiled->data_len = 1;
        re_compiled->data[0] = (unsigned char)c;
      } break;
    }
    i += 1;
    j += 2 + re_compiled->data_len;
  }
  if (j + 1 >= MAX_REGEXP_LEN) {
      //fputs("exceeded internal buffer!\n", stderr);
       return 0;
  }
  /* 'UNUSED' is a sentinel used to indicate end-of-pattern */
  re_data[j] = UNUSED;
  re_data[j+1] = 0;

  return (re_t) re_data;
}

void re_print(regex_t* pattern)
{
  const char *const types[] = { "UNUSED", "DOT", "BEGIN", "END", "QUESTIONMARK", "STAR", "PLUS", "CHAR", "CHAR_CLASS", "INV_CHAR_CLASS", "DIGIT", "NOT_DIGIT", "ALPHA", "NOT_ALPHA", "WHITESPACE", "NOT_WHITESPACE" /*, "BRANCH" */ };

  int j;
  char c;

  if (!pattern)
    return;
  for (;; pattern = getnext(pattern))
  {
    if (pattern->type == UNUSED)
    {
      break;
    }

    if (pattern->type <= NOT_WHITESPACE)
      printf("type: %s", types[pattern->type]);
    else
      printf("invalid type: %d", pattern->type);

    if (pattern->type == CHAR_CLASS || pattern->type == INV_CHAR_CLASS)
    {
      printf(" [");
      if (pattern->type == INV_CHAR_CLASS)
        printf("^");
      for (j = 0; j < pattern->data_len; ++j)
      {
        c = pattern->data[j];
        if ((c == '\0') || (c == ']'))
        {
          break;
        }
        printf("%c", c);
      }
      printf("]");
    }
    else if (pattern->type == CHAR)
    {
      printf(" '%c'", pattern->data[0]);
    }
    printf("\n");
  }
}



/* Private functions: */
static int matchdigit(char c)
{
  return isdigit((unsigned char)c);
}
static int matchalpha(char c)
{
  return isalpha((unsigned char)c);
}
static int matchwhitespace(char c)
{
  return isspace((unsigned char)c);
}
static int matchalphanum(char c)
{
  return ((c == '_') || matchalpha(c) || matchdigit(c));
}
static int matchrange(char c, const char* str)
{
  return (    (c != '-')
           && (str[0] != '\0')
           && (str[0] != '-')
           && (str[1] == '-')
           && (str[2] != '\0')
           && (    (c >= str[0])
                && (c <= str[2])));
}
static int matchdot(char c)
{
#if defined(RE_DOT_MATCHES_NEWLINE) && (RE_DOT_MATCHES_NEWLINE == 1)
  (void)c;
  return 1;
#else
  return c != '\n' && c != '\r';
#endif
}
static int ismetachar(char c)
{
  return ((c == 's') || (c == 'S') || (c == 'w') || (c == 'W') || (c == 'd') || (c == 'D'));
}

static int matchmetachar(char c, const char* str)
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

static int matchcharclass(char c, unsigned char len, const char* str)
{
  if (str[0] == '-' && c == '-') {
      return 1;
  }

  int i = 0;
  do
  {
    if (matchrange(c, str))
    {
      return 1;
    }
    else if (str[0] == '\\')
    {
      /* Escape-char: increment str-ptr and match on next char */
      str += 1;
      if (matchmetachar(c, str))
      {
        return 1;
      }
      else if ((c == str[0]) && !ismetachar(c))
      {
        return 1;
      }
    }
    else if (c == str[0])
    {
      if (c == '-')
      {
        if ((str[-1] == '\0') || (i == len - 1))
            return 1;
        // else continue
      }
      else
      {
        return 1;
      }
    }
  }
  while (++i < len && *str++ != '\0');

  return 0;
}

static int matchone(regex_t* p, char c)
{
  switch (p->type)
  {
    case DOT:            return matchdot(c);
    case CHAR_CLASS:     return  matchcharclass(c, p->data_len, (const char*)p->data);
    case INV_CHAR_CLASS: return !matchcharclass(c, p->data_len, (const char*)p->data);
    case DIGIT:          return  matchdigit(c);
    case NOT_DIGIT:      return !matchdigit(c);
    case ALPHA:          return  matchalphanum(c);
    case NOT_ALPHA:      return !matchalphanum(c);
    case WHITESPACE:     return  matchwhitespace(c);
    case NOT_WHITESPACE: return !matchwhitespace(c);
    case BEGIN:          return 0;
    default:             return  (p->data[0] == c);
  }
}

static inline int matchstar(regex_t* p, regex_t* pattern, const char* text, int* matchlength)
{
  return matchplus(p, pattern, text, matchlength) || matchpattern(pattern, text, matchlength);
}

static int matchplus(regex_t* p, regex_t* pattern, const char* text, int* matchlength)
{
  const char* prepoint = text;
  while ((text[0] != '\0') && matchone(p, *text))
  {
    text++;
  }
  for (; text > prepoint; text--)
  {
    if (matchpattern(pattern, text, matchlength)) {
      *matchlength += text - prepoint;
      return 1;
    }
  }
  return 0;
}

static int matchquestion(regex_t *p, regex_t* pattern, const char* text, int* matchlength)
{
  if (matchpattern(pattern, text, matchlength))
      return 1;
  if (*text && matchone(p, *text++))
  {
    if (matchpattern(pattern, text, matchlength))
    {
      (*matchlength)++;
      return 1;
    }
  }
  return 0;
}

/* Iterative matching */
static int matchpattern(regex_t* pattern, const char* text, int* matchlength)
{
  int pre = *matchlength;
  while (1)
  {
    if (pattern->type == UNUSED)
    {
      return 1;
    }
    regex_t* next_pattern = getnext(pattern);
    if (next_pattern->type == QUESTIONMARK)
    {
      return matchquestion(pattern, getnext(next_pattern), text, matchlength);
    }
    else if (next_pattern->type == STAR)
    {
      return matchstar(pattern, getnext(next_pattern), text, matchlength);
    }
    else if (next_pattern->type == PLUS)
    {
      return matchplus(pattern, getnext(next_pattern), text, matchlength);
    }
    else if ((pattern->type == END) && next_pattern->type == UNUSED)
    {
      return (text[0] == '\0');
    }
/*  Branching is not working properly
    else if (pattern->type == BRANCH)
    {
      return (matchpattern(pattern, text) || matchpattern(getnext(next_pattern), text));
    }
*/
  (*matchlength)++;
    if (text[0] == '\0')
      break;
    if (!matchone(pattern, *text++))
      break;
    pattern = next_pattern;
  }

  *matchlength = pre;
  return 0;
}
