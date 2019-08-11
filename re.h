/*
 * Mini regex-module inspired by Rob Pike's regex code described in:
 *
 * http://www.cs.princeton.edu/courses/archive/spr09/cos333/beautiful.html
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* max number of tokens in regex */
#define MAXTOKENS 30
/* max length of character-class buffer */
#define CCLBUFLEN 20
/* max number of nested groups */
#define MAXGROUPS 5

/* enum for all the types a char in a char class can be */
typedef enum ClassCharType
{
	CCL_END, /* terminator of CHARCLASS sequence */
	CCL_METABSL, /* metabackslash; note that metachars don't work in char classes */
	CCL_CHARRANGE /* literal character range: single characters count as a range */
} ClassCharType;

/* a char that can go inside a CHARCLASS (represented by [])*/
typedef struct ClassChar
{
	ClassCharType type;
	union
	{
		size_t meta; /* METABSL/INVMETABSL: index in metabsls */
		struct /* CHARRANGE */
		{
			char first; /* first char in range */
			char last; /* last char in range */
		};
	};
} ClassChar;

/* the different types that each regex token can be */
typedef enum TokenType
{
	TOKEN_END, /* the end of the regex */
	TOKEN_GROUP, /* a group */
	TOKEN_GROUPEND, /* the end of a group */
	TOKEN_METABSL, /* metabackslash: meta character sequences that begin with backslash */
	TOKEN_METACHAR, /* a metachar, such as $ */
	TOKEN_CHARCLASS, /* a character class that is surrounded by [...] */
	TOKEN_INVCHARCLASS, /* a character class that is inverted by placing a ^ at the start [^...] */
	TOKEN_CHAR /* a literal character */
} TokenType;

/* the different types that each group can be */
typedef enum GroupType
{
	GROUP_NORMAL, /* a normal group that eats characters */
	GROUP_LOOKAROUND, /* a group that doesn't eat characters */
	GROUP_INVERTED /* an inverted group */
} GroupType;

/* struct for each regex token */
typedef struct re_Token
{
	TokenType type;
	union
	{
		size_t meta; /* METABSL/INVMETABSL/METACHAR: index in metabsls/metachars */
		ClassChar* ccl; /* CHARCLASS/INVCHARCLASS: a pointer to characters in class (pointer to somewhere in cclbuf) */
		char ch; /* CHAR: the character itself */
		struct /* GROUP */
		{
			bool capturing;
			uint_fast8_t modifiers;
			GroupType grouptype;
		};
	};
	uint_fast8_t quantifiermin;
	uint_fast8_t quantifiermax;
	bool greedy; /* whether the token is greedy (takes up as many characters as possible) or lazy (takes up as few characters as possible) */
	bool atomic; /* whether the token is atomic (cannot change if the rest of the regex fails) or not; sometimes known as possessive */
} re_Token;

/* main struct for a regex */
typedef struct Regex
{
	re_Token tokens[MAXTOKENS]; /* array of tokens in regex */
	ClassChar cclbuf[CCLBUFLEN]; /* buffer in which character class strings are stored */
	size_t ccli; /* index into buffer */
} Regex;

/* re_compile: compile regex string pattern to a Regex) */
void re_compile(Regex* compiled, const char* pattern);

/* re_match: returns index of first match of pattern in text */
/* stores the length of the match in length if it is not NULL */
size_t re_match(Regex pattern, const char* text, size_t* length);

/* re_matchg: returns number of matches of pattern in text */
size_t re_matchg(Regex pattern, const char* text);

/* re_print: prints a regex to stdout */
void re_print(Regex pattern);

#ifdef __cplusplus
}
#endif
