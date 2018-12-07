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

#ifdef __cplusplus
extern "C"{
#endif

#define MAX_REGEXP_OBJECTS      30    /* Max number of regex symbols in expression. */

typedef struct regex_t
{
  unsigned char  type;   /* CHAR, STAR, etc.                      */
  union
  {
    unsigned char  ch;   /*      the character itself             */
    unsigned char* ccl;  /*  OR  a pointer to characters in class */
  };
} regex_t;

/* Typedef'd array to get abstract datatype. */
typedef regex_t re_t[MAX_REGEXP_OBJECTS];


/* Compile regex string pattern to a regex_t-array and copy to re_compiled.
 * Return the number of regex objects copied upon success or -1 upon error. */
int re_compile(re_t re_compiled, const char* pattern);


/* Find matches of the compiled pattern inside text. */
int  re_matchp(re_t pattern, const char* text);


/* Find matches of the txt pattern inside text (will compile automatically first). */
int  re_match(const char* pattern, const char* text);


#ifdef __cplusplus
}
#endif
