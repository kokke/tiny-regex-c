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

//use recursive or loop compare on a large stack
//undefine RE_MATCH_RECURSIVE for embedded or kernel
#ifndef RE_MATCH_RECURSIVE
	#define RE_MATCH_RECURSIVE 
#endif //RE_MATCH_RECURSIVE

#ifndef RE_REGEX_INSTANCE_REALLOCATE
    #define RE_REGEX_INSTANCE_REALLOCATE
#endif

#ifndef RE_TRUNC_ARRAY_IF_POSSIBLE
    #define RE_TRUNC_ARRAY_IF_POSSIBLE
#endif

#ifndef RE_BUILDWITH_DEBUG
    #define RE_BUILDWITH_DEBUG
#endif

#ifndef BUILD_WITH_ERRORMSG
    #define LOGERR(...)
#else
    #define LOGERR(...) fprintf(stderr, __VA_ARGS__)
#endif

#ifndef RE_CHAR_CLASS_LENGTH
    //per struct regex_t instance
    #define RE_CHAR_CLASS_LENGTH 20
#endif

#ifndef MIN_REGEXP_OBJECTS
    //min number of regex symbols in expression
    #define MIN_REGEXP_OBJECTS 30
#endif

/*
 * Below can be uncommented to modify the behaviour or optimizations
 */
 
/*uncomment to use on a small stack machines*/
//#undef RE_MATCH_RECURSIVE

/*uncomment to disallow grow of the regex_t array upon MIN_REGEXP_OBJECTS limit reached*/
//#undef RE_REGEX_INSTANCE_REALLOCATE

/*uncomment to disaalow the optimization of the regex_t array by truncating unused objects*/
//#undef RE_TRUNC_ARRAY_IF_POSSIBLE

/*uncommnet to remove debug functions (print and trace functions)*/
//#undef RE_BUILDWITH_DEBUG

/*uncomment to send error messages to stderr*/
//#define BUILD_WITH_ERRORMSG


#ifdef __cplusplus
extern "C"{
#endif



/* Typedef'd pointer to get abstract datatype. */
typedef struct regex_t* re_t;


/* Compile regex string pattern to a regex_t-array. 
 * o_reg_cnt can be NULL*/
re_t re_compile(const char* pattern, unsigned int * o_reg_cnt);


/* Find matches of the compiled pattern inside text. */
int  re_matchp(re_t pattern, const char* text);


/* Find matches of the txt pattern inside text (will compile automatically first). */
int  re_match(const char* pattern, const char* text);

#ifdef RE_BUILDWITH_DEBUG
void re_print(re_t pattern, unsigned int count);
void re_trace(re_t pattern, unsigned int count);
#else
#define re_print(_a_, _b_)
#define re_trace(_a_, _b_)
#endif

#ifdef __cplusplus
}
#endif
