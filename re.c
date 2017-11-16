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


typedef struct regex_t
{
    unsigned char  type;   /* CHAR, STAR, etc.                      */
    union
    {
        unsigned char  ch;   /*      the character itself             */
        unsigned char ccl[RE_CHAR_CLASS_LENGTH];  /*  OR  a pointer to characters in class */
    };
} regex_t;


/* Private function declarations: */
static int matchpattern(regex_t* pattern, const char* text);
static int matchcharclass(char c, const char* str);
static int matchstar(regex_t p, regex_t* pattern, const char* text);
static int matchplus(regex_t p, regex_t* pattern, const char* text);
static int matchone(regex_t p, char c);
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

int re_matchp(re_t pattern, const char* text)
{
    //the below should never happen
    assert( pattern != NULL );
    assert( text != NULL );

    int idx = -1;

    if (pattern[0].type == BEGIN)
    {
        //starts from begin ^
        return ( (matchpattern(&pattern[1], text)) ? 0 : -1 );
    }
    else
    {
        do
        {
            idx += 1;
            if (matchpattern(pattern, text))
            {
                return idx;
            }
        } while (*text++ != '\0');

        return -1;
    }

    //should not reach there
}

re_t re_compile(const char* pattern, unsigned int * o_reg_cnt)
{
    assert(pattern != NULL);

    re_t re_compiled = 
            (re_t) calloc(MIN_REGEXP_OBJECTS, sizeof(struct regex_t));

    if (re_compiled == NULL)
    {
        LOGERR("calloc returned NULL at %s:%u\r\n", __FUNCTION__, __LINE__);
        return NULL;
    }
    
    char c;			// current char in pattern
    unsigned int i = 0;  // index into pattern
    unsigned int j = 0;  // index into re_compiled
    unsigned int re_compiled_count = MIN_REGEXP_OBJECTS;


    while (pattern[i] != '\0')
    {   
        /*
         * last one is reserved for the final one
         * check if re_compiled still have space otherwide do:
         * - 1 needed because the j counter is
         * incremented at the end and the last object is indicating
         * the end of the array i.e UNUSED.
         */
        if ( (re_compiled_count - 1) <= j)
        {
            
#ifdef RE_REGEX_INSTANCE_REALLOCATE
            //reallocate memory
            re_compiled_count += MIN_REGEXP_OBJECTS;
            
            re_t re_temp = 
                (re_t) realloc(re_compiled, (re_compiled_count * sizeof(struct regex_t)));
            
            if (re_temp == NULL)
            {
                LOGERR("realloc returned NULL! requested %lu bytes at %s:%u\r\n", 
                      (re_compiled_count * sizeof(struct regex_t)), 
                      __FUNCTION__, 
                      __LINE__);
                      
                goto re_temp_error;
            }
            
            re_compiled = re_temp;
#else
            LOGERR("got run out of free static instances of the REGEXP_OBJECTS! has %u, at %s:%u\r\n",
                    MIN_REGEXP_OBJECTS, 
                    __FUNCTION__, 
                    __LINE__);
                    
            goto re_temp_error;
#endif
		
		}
		
		c = pattern[i];
		
		switch (c)
		{
			/* Meta-characters: */
			case '^':
				re_compiled[j].type = BEGIN;
			break;
			
			case '$':
				re_compiled[j].type = END;
			break;
			
			case '.': 
				re_compiled[j].type = DOT;
			break;
			
			case '*': 
				re_compiled[j].type = STAR; 
			break;
			
			case '+': 
				re_compiled[j].type = PLUS;
			break;
			
			case '?': 
				re_compiled[j].type = QUESTIONMARK;
			break;
			
			case '|': 
				re_compiled[j].type = BRANCH;
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
							re_compiled[j].type = DIGIT; 
						break;
						
						case 'D': 
							re_compiled[j].type = NOT_DIGIT;
						break;
						
						case 'w': 
							re_compiled[j].type = ALPHA;
						break;
						
						case 'W': 
							re_compiled[j].type = NOT_ALPHA; 
						break;
						
						case 's': 
							re_compiled[j].type = WHITESPACE; 
						break;
						
						case 'S': 
							re_compiled[j].type = NOT_WHITESPACE;
						break;

						/* Escaped character, e.g. '.' or '$' */ 
						default:  
							re_compiled[j].type = CHAR;
							re_compiled[j].ch = pattern[i];
						break;
					}
				}
				/* '\\' as last char in pattern -> invalid regular expression. */
				/*
				else
				{ 
				  re_compiled[j].type = CHAR;
				  re_compiled[j].regdata.ch = pattern[i];
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
					re_compiled[j].type = INV_CHAR_CLASS;
					i += 1; // Increment i to avoid including '^' in the char-buffer
				}  
				else
				{
					re_compiled[j].type = CHAR_CLASS;
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
				
				//check if it can be fitted in the struct re_regex -> ccl
				if (  pat_len >= RE_CHAR_CLASS_LENGTH )
				{
					//pattern[i] = '\0';
					
					LOGERR("the [%.*s] pattern is too long!\r\n"
						   "-->can fit only %u of uint8_t has %u at %s:%u\r\n",
						   pat_len,
						   &pattern[pat_begin],
						   RE_CHAR_CLASS_LENGTH,
						   pat_len,
						   __FUNCTION__,
						   __LINE__);
					
                    /*
                     * Notoce:
                     * in order to deal with this situation, the following improvements
                     * can be made:
                     * either: in struct regex_t instead of the fixed char array use 
                     *          pointer to store pointer to the heap (call malloc)
                     *          This descreases the flexibility because in order to
                     *          , for instance, transfer the compiled regex from 
                     *          userland to kernel, it will be required to copy 
                     *          struct regex_t array and then trace this array to copy
                     *          each malloced patern storages.
                     * or: modify the struct regex_t so at the first place it contains 
                     *      a pointer to the 'long' pattern string which is separated by
                     *      NULL terminators and a pointer to the struct regex_obj which is
                     *      an original of the current version of the struct regex_t.
                     *      Or another approach is to make the new struct regex_t solid
                     *      by storing everyting to the temporary storage and then knowing
                     *      all required lengths and sizes of the data allocate a single
                     *      piece of memory and copy everything there storing the (not the
                     *      pointer) integer offsets in the pattern 'long' array.
                     */
					goto re_temp_error;
				}
				
				//copy data to the structure, unsafe operation
				memmove((void*)re_compiled[j].ccl, (const void*) &pattern[pat_begin], pat_len);
			}
			break;

			/* Other characters: */
			default:
			{
				re_compiled[j].type = CHAR;
				re_compiled[j].ch = c;
			} 
			break;
		}
		
		i += 1;
		j += 1;
	}
	
	//'UNUSED' is a sentinel used to indicate end-of-pattern
	re_compiled[j].type = UNUSED;
	
	//move next, j now indicates the amount of the recs from 0..j-1
	j++;
	
#ifdef RE_TRUNC_ARRAY_IF_POSSIBLE

	//check if the memory can be truncated
	if (re_compiled_count > j)
	{
		re_compiled_count = j; 
		re_t re_temp = 
				(re_t) realloc(re_compiled, (re_compiled_count * sizeof(struct regex_t)) );
		
		if (re_temp == NULL)
		{
			LOGERR("realloc returned NULL! requested %lu bytes at %s:%u\r\n", 
				   (re_compiled_count * sizeof(struct regex_t)), 
                   __FUNCTION__, 
                   __LINE__);
            //ignore
		}
		else
		{
			re_compiled = re_temp;
		}
	}
	
#endif

	//return the result
	if (o_reg_cnt != NULL)
	{
		*o_reg_cnt = re_compiled_count;
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
  
  for (unsigned int i = 0; pattern[i].type != UNUSED; ++i)
	{
		if ((count != 0) && (i >= count))
		{
			printf("!!!<>>\r\n");
			break;
		}
		if (pattern[i].type == BEGIN)
		{
			printf("^");
		}
		else if (pattern[i].type == END)
		{
			printf("$");
		}
		
		if (pattern[i].type == CHAR_CLASS || pattern[i].type == INV_CHAR_CLASS)
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
			printf("%s", pattern[i].ccl);
			printf("]");
		}
		else if (pattern[i].type == CHAR)
		{
			printf("'%c'", pattern[i].ch);
		}
	}
	
	return;
}

void re_trace(re_t pattern, unsigned int count)
{
    assert(pattern != NULL);
    
	const char* types[] = { "UNUSED", "DOT", "BEGIN", "END", "QUESTIONMARK", "STAR", "PLUS", "CHAR", "CHAR_CLASS", "INV_CHAR_CLASS", "DIGIT", "NOT_DIGIT", "ALPHA", "NOT_ALPHA", "WHITESPACE", "NOT_WHITESPACE", "BRANCH" };
	
	for (unsigned int i = 0; pattern[i].type != UNUSED; ++i)
	{
		if ((count != 0) && (i >= count))
		{
			printf("reached end (count: %u) before the UNUSED node was reached\r\n", count);
			break;
		}

		printf("type: %s", types[pattern[i].type]);
		if (pattern[i].type == CHAR_CLASS || pattern[i].type == INV_CHAR_CLASS)
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
			printf("%s", pattern[i].ccl);
			printf("]");
		}
		else if (pattern[i].type == CHAR)
		{
			printf(" '%c'", pattern[i].ch);
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

static int matchone(regex_t p, char c)
{
  switch (p.type)
  {
    case DOT:            return 1;
    case CHAR_CLASS:     return  matchcharclass(c, (const char*)p.ccl);
    case INV_CHAR_CLASS: return !matchcharclass(c, (const char*)p.ccl);
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


#ifdef RE_MATCH_RECURSIVE

/* Recursive matching */
static int matchpattern(regex_t* pattern, const char* text)
{
  if ((pattern[0].type == UNUSED) || (pattern[1].type == QUESTIONMARK))
  {
    return 1;
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
      return 1;
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
    else if (pattern[1].type == BRANCH)
    {
      return (matchpattern(pattern, text) || matchpattern(&pattern[2], text));
    }
  }
  while ((text[0] != '\0') && matchone(*pattern++, *text++));

  return 0;
}

#endif




