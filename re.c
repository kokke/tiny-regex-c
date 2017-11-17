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




#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

/* Overrides for the re.h: */

#define MIN_REGEXP_OBJECTS          30    /* Max number of regex symbols in expression. */
#define RE_CHAR_CLASS_LENGTH        40    /* Max length of character-class buffer in.   */

#define BUILD_WITH_ERRORMSG

#include "re.h"



enum { UNUSED, DOT, BEGIN, END, QUESTIONMARK, STAR, PLUS, CHAR, CHAR_CLASS, INV_CHAR_CLASS, DIGIT, NOT_DIGIT, ALPHA, NOT_ALPHA, WHITESPACE, NOT_WHITESPACE, BRANCH };

typedef struct regex_objs_t
{
    unsigned char  type;   // CHAR, STAR, etc.
    union
    {
        unsigned char  ch;   //      the character itself
        unsigned int ccl;    // OR  a offset to characters in class
    };
} regex_objs_t;

typedef struct regex_t
{
    unsigned int objoffset;     //indicates the start of the regex objs
    unsigned char data[];       //data
} regex_t;


/* Private function declarations: */
static int matchpattern(regex_t* reg, regex_objs_t * pattern, const char* text);
static int matchcharclass(char c, const char* str);
static int matchstar(regex_t * reg, regex_objs_t p, regex_objs_t* pattern, const char* text);
static int matchplus(regex_t * reg, regex_objs_t p, regex_objs_t* pattern, const char* text);
static int matchone(regex_t* pattern, regex_objs_t p, char c);
static int matchdigit(char c);
static int matchalpha(char c);
static int matchwhitespace(char c);
static int matchmetachar(char c, const char* str);
static int matchrange(char c, const char* str);
static int ismetachar(char c);



/* Public functions: */
int re_match(const char* pattern, const char* text)
{
    int ret = -1;
    re_t reg = re_compile(pattern, NULL);
    if (reg == NULL)
    {
        LOGERR("failed at %s:%u\r\n", __FUNCTION__, __LINE__);
    }
    else
    {
        ret = re_matchp(reg, text);
        free((void*) reg);
    }

    return ret;
}

int re_matchp(re_t regx, const char* text)
{
    //the below should never happen
    assert( regx != NULL );
    assert( text != NULL );
    
    struct regex_objs_t * objs = SECTION_OBJECTS(regx);
    int idx = -1;

    if (objs[0].type == BEGIN)
    {
        //starts from begin ^
        return ( (matchpattern(regx, &objs[1], text)) ? 0 : -1 );
    }
    else
    {
        do
        {
            idx += 1;
            if (matchpattern(regx, objs, text))
            {
                return idx;
            }
        } while (*text++ != '\0');

        return -1;
    }

    //should not reach there
}

int __re_compile_count(const char* pattern, unsigned int * objs, unsigned int * strln)
{
    assert(objs != NULL);
    assert(strln != NULL);
    
    unsigned int i = 0;
    unsigned char c = 0;
    
    while (pattern[i] != '\0')
    {   
		c = pattern[i];
        
		(*objs)++;
        
		if (c == '[')
        {
            // Remember where pattern starts.
            unsigned int pat_begin = 0;
            unsigned int pat_len = 0;
            
            // Look-ahead to determine if negated
            if (pattern[i+1] == '^')
            {
                i += 1; // Increment i to avoid including '^' in the char-buffer
            }  
            
            //[test]
            //storing start offset of the pattern
            pat_begin = i + 1;

            /* Copy characters inside [..] to buffer */
            while (pattern[++i] != ']')
            {
                //check if next is not null
                if (pattern[i] == '\0')
                {
                    LOGERR("the next element in pattern [] is NULL before reaching ] symbol\r\n"
                           "--> i=%u value=%s at %s:%u\r\n", 
                           i, 
                           &pattern[pat_begin], 
                           __FUNCTION__, 
                           __LINE__);
                           
                    return 1;
                }
            }
            
            //calculating the patt length
            pat_len = (i - pat_begin);
            *strln += pat_len + 1; // + NULL
        }
		
		i += 1;
	}
	
	//'UNUSED' is a sentinel used to indicate end-of-pattern
	(*objs)++;
	
	return 0;
}

re_t re_compile(const char* pattern, unsigned int * o_reg_cnt)
{
    assert(pattern != NULL);
    
    //calculating the length
    unsigned int objs = 0;
    unsigned int strln = 0;
    
    if (__re_compile_count(pattern, &objs, &strln) != 0)
    {
        //leave
        return NULL;
    }
    
    //calculating total size
    size_t totallen = 
        ( objs * sizeof(struct regex_objs_t) ) + strln + sizeof(struct regex_t);

    re_t re_compiled = 
            (re_t) calloc(1, totallen);
    
    if (re_compiled == NULL)
    {
        LOGERR("calloc returned NULL at %s:%u\r\n", __FUNCTION__, __LINE__);
        return NULL;
    }
    
    re_compiled->objoffset = strln;
    char * patrns = (char *) &re_compiled->data[0];
    //objects starts at end of the strln
    struct regex_objs_t * re_obj = 
        (struct regex_objs_t *) &re_compiled->data[strln];

    char c;			            // current char in pattern
    unsigned int i = 0;         // index into pattern
    unsigned int j = 0;         // index into re_compiled
    unsigned int patoff = 0;    //pattern offset


    while (pattern[i] != '\0')
    {   
		c = pattern[i];
		
		switch (c)
		{
			/* Meta-characters: */
			case '^':
				re_obj[j].type = BEGIN;
			break;
			
			case '$':
				re_obj[j].type = END;
			break;
			
			case '.': 
				re_obj[j].type = DOT;
			break;
			
			case '*': 
				re_obj[j].type = STAR; 
			break;
			
			case '+': 
				re_obj[j].type = PLUS;
			break;
			
			case '?': 
				re_obj[j].type = QUESTIONMARK;
			break;
			
			case '|': 
				re_obj[j].type = BRANCH;
			break;
			
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
						case 'd': 
							re_obj[j].type = DIGIT; 
						break;
						
						case 'D': 
							re_obj[j].type = NOT_DIGIT;
						break;
						
						case 'w': 
							re_obj[j].type = ALPHA;
						break;
						
						case 'W': 
							re_obj[j].type = NOT_ALPHA; 
						break;
						
						case 's': 
							re_obj[j].type = WHITESPACE; 
						break;
						
						case 'S': 
							re_obj[j].type = NOT_WHITESPACE;
						break;

						/* Escaped character, e.g. '.' or '$' */ 
						default:  
							re_obj[j].type = CHAR;
							re_obj[j].ch = pattern[i];
						break;
					}
				}
				/* '\\' as last char in pattern -> invalid regular expression. */
				/*
				else
				{ 
				  re_obj[j].type = CHAR;
				  re_obj[j].regdata.ch = pattern[i];
				}
				*/
			} 
			break;

			/* Character class: */
			case '[':
			{
				// Remember where pattern starts.
				unsigned int pat_begin = 0;
				unsigned int pat_len = 0;
				
				// Look-ahead to determine if negated
				if (pattern[i+1] == '^')
				{
					re_obj[j].type = INV_CHAR_CLASS;
					i += 1; // Increment i to avoid including '^' in the char-buffer
				}  
				else
				{
					re_obj[j].type = CHAR_CLASS;
				}
				//[test]
				//storing start offset of the pattern
				pat_begin = i + 1;

				/* Copy characters inside [..] to buffer */
				while (pattern[++i] != ']')
				{
					//check if next is not null
					if (pattern[i] == '\0')
					{
						LOGERR("the next element in pattern [] is NULL before reaching ] symbol\r\n"
                               "--> i=%u value=%s at %s:%u\r\n", 
                               i, 
                               &pattern[pat_begin], 
                               __FUNCTION__, 
                               __LINE__);
                               
						goto re_temp_error;
					}
				}
                
                //calculating the patt length
				pat_len = (i - pat_begin);
				
                re_obj[j].ccl = patoff;
				//copy data to the structure, unsafe operation
				memmove((void*)&patrns[patoff], (const void*) &pattern[pat_begin], pat_len);
                
                patoff += pat_len + 1; //+null
            }
			break;

			/* Other characters: */
			default:
			{
				re_obj[j].type = CHAR;
				re_obj[j].ch = c;
			} 
			break;
		}
		
		i += 1;
		j += 1;
	}
	
	//'UNUSED' is a sentinel used to indicate end-of-pattern
	re_obj[j].type = UNUSED;
	
	//move next, j now indicates the amount of the recs from 0..j-1
	j++;

	//return the result
	if (o_reg_cnt != NULL)
	{
		*o_reg_cnt = objs;
	}
	
	return re_compiled;
	
re_temp_error:
    if (re_compiled != NULL)
    {
        free(re_compiled);
    }
    
	return NULL;
}

#ifdef RE_BUILDWITH_DEBUG

void re_print(re_t pattern, unsigned int count)
{
  assert(pattern != NULL);
  
  //convert pattern
  struct regex_objs_t * objs = SECTION_OBJECTS(pattern);
  
  for (unsigned int i = 0; objs[i].type != UNUSED; ++i)
	{
		if ((count != 0) && (i >= count))
		{
			printf("!!!<>>\r\n");
			break;
		}
		if (objs[i].type == BEGIN)
		{
			printf("^");
		}
		else if (objs[i].type == END)
		{
			printf("$");
		}
		
		if (objs[i].type == CHAR_CLASS || objs[i].type == INV_CHAR_CLASS)
		{
			printf("[");
			/*int j;
			char c;
			for (j = 0; j < MAX_CHAR_CLASS_LEN; ++j)
			{
				c = pattern[i].ccl[j];
				if ((c == '\0') || (c == ']'))
				{
					break;
				}
				printf("%c", c);
			}*/
			printf("%s", OFFSET_TO_PATTERN(pattern, objs[i].ccl));
			printf("]");
		}
		else if (objs[i].type == CHAR)
		{
			printf("'%c'", objs[i].ch);
		}
	}
	
	return;
}

void re_trace(re_t pattern, unsigned int count)
{
    assert(pattern != NULL);
    
	const char* types[] = { "UNUSED", "DOT", "BEGIN", "END", "QUESTIONMARK", "STAR", "PLUS", "CHAR", "CHAR_CLASS", "INV_CHAR_CLASS", "DIGIT", "NOT_DIGIT", "ALPHA", "NOT_ALPHA", "WHITESPACE", "NOT_WHITESPACE", "BRANCH" };
	
    //convert pattern
    struct regex_objs_t * objs = SECTION_OBJECTS(pattern);
    
	for (unsigned int i = 0; objs[i].type != UNUSED; ++i)
	{
		if ((count != 0) && (i >= count))
		{
			printf("reached end (count: %u) before the UNUSED node was reached\r\n", count);
			break;
		}

		printf("type: %s", types[objs[i].type]);
		if (objs[i].type == CHAR_CLASS || objs[i].type == INV_CHAR_CLASS)
		{
			printf(" [");
			/*int j;
			char c;
			for (j = 0; j < MAX_CHAR_CLASS_LEN; ++j)
			{
				c = pattern[i].ccl[j];
				if ((c == '\0') || (c == ']'))
				{
					break;
				}
				printf("%c", c);
			}*/
			printf("%s", OFFSET_TO_PATTERN(pattern, objs[i].ccl));
			printf("]");
		}
		else if (objs[i].type == CHAR)
		{
			printf(" '%c'", objs[i].ch);
		}
		printf("\n");
	}
	
	return;
}

#endif

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
static int matchrange(char c, const char* str)
{
  return ((c != '-') && (str[0] != '-') && (str[1] == '-') && (str[2] != '\0') && ((c >= str[0]) && (c <= str[2])));
}
static int ismetachar(char c)
{
  return ((c == 's') || (c == 'S') == (c == 'w') || (c == 'W') || (c == 'd') || (c == 'D'));
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

static int matchcharclass(char c, const char* str)
{
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
        return ((str[-1] == '\0') || (str[1] == '\0'));
      }
      else
      {
        return 1;
      }
    }
  }
  while (*str++ != '\0');

  return 0;
}

static int matchone(regex_t* pattern, regex_objs_t p, char c)
{
  switch (p.type)
  {
    case DOT:            return 1;
    case CHAR_CLASS:     return  matchcharclass(c, (const char*)OFFSET_TO_PATTERN(pattern, p.ccl));
    case INV_CHAR_CLASS: return !matchcharclass(c, (const char*)OFFSET_TO_PATTERN(pattern, p.ccl));
    case DIGIT:          return  matchdigit(c);
    case NOT_DIGIT:      return !matchdigit(c);
    case ALPHA:          return  matchalphanum(c);
    case NOT_ALPHA:      return !matchalphanum(c);
    case WHITESPACE:     return  matchwhitespace(c);
    case NOT_WHITESPACE: return !matchwhitespace(c);
    default:             return  (p.ch == c);
  }
}

static int matchstar(regex_t * reg, regex_objs_t p, regex_objs_t* pattern, const char* text)
{
  do
  {
    if (matchpattern(reg, pattern, text))
      return 1;
  }
  while ((text[0] != '\0') && matchone(reg, p, *text++));

  return 0;
}

static int matchplus(regex_t * reg, regex_objs_t p, regex_objs_t* pattern, const char* text)
{
  while ((text[0] != '\0') && matchone(reg, p, *text++))
  {
    if (matchpattern(reg, pattern, text))
      return 1;
  }
  return 0;
}


#ifdef RE_MATCH_RECURSIVE

/* Recursive matching */
static int matchpattern(regex_t* reg, regex_objs_t * pattern, const char* text)
{
  if ((pattern[0].type == UNUSED) || (pattern[1].type == QUESTIONMARK))
  {
    return 1;
  }
  else if (pattern[1].type == STAR)
  {
    return matchstar(reg, pattern[0], &pattern[2], text);
  }
  else if (pattern[1].type == PLUS)
  {
    return matchplus(reg, pattern[0], &pattern[2], text);
  }
  else if ((pattern[0].type == END) && pattern[1].type == UNUSED)
  {
    return text[0] == '\0';
  }
  else if ((text[0] != '\0') && matchone(reg, pattern[0], text[0]))
  {
    return matchpattern(reg, &pattern[1], text+1);
  }
  else
  {
    return 0;
  }
}

#else

/* Iterative matching */
static int matchpattern(regex_t* reg, regex_objs_t * pattern, const char* text)
{    
    do
    {
        if ((pattern[0].type == UNUSED) || (pattern[1].type == QUESTIONMARK))
        {
          return 1;
        }
        else if (pattern[1].type == STAR)
        {
          return matchstar(reg, pattern[0], &pattern[2], text);
        }
        else if (pattern[1].type == PLUS)
        {
          return matchplus(reg, pattern[0], &pattern[2], text);
        }
        else if ((pattern[0].type == END) && pattern[1].type == UNUSED)
        {
          return (text[0] == '\0');
        }
        else if (pattern[1].type == BRANCH)
        {
          return (matchpattern(reg, pattern, text) || matchpattern(reg, &pattern[2], text));
        }
    }
    while ((text[0] != '\0') && matchone(reg, *pattern++, *text++));

    return 0;
}

#endif




