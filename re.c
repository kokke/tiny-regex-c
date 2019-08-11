/*
 * Mini regex-module inspired by Rob Pike's regex code described in:
 *
 * http://www.cs.princeton.edu/courses/archive/spr09/cos333/beautiful.html
 *
 * Changes from other Regexes:
 * ---------------------------
 *  no multiline mode, \A, \z or \Z; use ^, $ and \R instead
 *  no (?i), (?s), etc; use modifiers in groups (?is:...) instead
 *  no octal, hexadecimal, unicode, control character escape sequences; use C's built-in ones instead
 *  lookarounds and groups are changed to have more features and be more consistent
 *  no POSIX classes
 *  no atomic groups (?>...); use the atomic instead (?...)!+
 */

/*
 * In this library, unless explicitly stated otherwise, all functions return the number of characters eaten, and they report errors by setting errno and returning 0.
 */

#include "re.h"
#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
/* small useful function that I'm going to pretend is in ctype.h */
static int iswordchar(int c)
{
	return isalnum(c) || c == '_';
}

#include <unistd.h>

#define UNUSED(variable) (void)(variable)

/*
 * PRIVATE FUNCTION DECLARATIONS
 */

/* compileone: compiles one regex token, returns number of chars eaten */
static size_t compileone(re_Token* compiled, const char* pattern, ClassChar cclbuf[CCLBUFLEN], size_t* ccli);
/* compileoneclc: compiles one class character, returns number of chars eaten */
static size_t compileoneclc(ClassChar* compiled, const char* pattern);
/* compilerange: compiles a range, returns number of chars eaten */
static size_t compilerange(ClassChar* compiled, const char* pattern);
/* compilequantifier: compiles a quantifier, returns number of chars eaten */
static size_t compilequantifier(re_Token* compiled, const char* pattern);
/* compilegreedy: sets whether the quantifier is greedy, returns number of chars eaten */
static size_t compilegreedy(re_Token* compiled, const char* pattern);
/* compileatomic: sets whether the quantifier is atomic, returns number of chars eaten */
static size_t compileatomic(re_Token* compiled, const char* pattern);

/* matchpattern: matches one pattern on a string, returns number of chars eaten */
static size_t matchpattern(re_Token* pattern, const char* text, size_t i, uint_fast8_t* modifiers);
/* matchcount: matches one regex token including quantifiers and sets count for number of quantifiers, returns number of characters eaten */
static size_t matchcount(re_Token pattern, uint_fast8_t* count, const char* text, size_t i, uint_fast8_t* modifiers);
/* matchone: matches one regex token ignoring quantifiers, returns number of characters eaten */
static size_t matchone(re_Token pattern, const char* text, size_t i, uint_fast8_t* modifiers);
/* matchoneclc: matches one class character, returns number of chars eaten */
static size_t matchoneclc(ClassChar pattern, const char* text, size_t i, uint_fast8_t* modifiers);
/* more matching functions */
static size_t matchwhitespace     (const char* text, size_t i, uint_fast8_t* modifiers);
static size_t matchnotwhitespace  (const char* text, size_t i, uint_fast8_t* modifiers);
static size_t matchdigit          (const char* text, size_t i, uint_fast8_t* modifiers);
static size_t matchnotdigit       (const char* text, size_t i, uint_fast8_t* modifiers);
static size_t matchwordchar       (const char* text, size_t i, uint_fast8_t* modifiers);
static size_t matchnotwordchar    (const char* text, size_t i, uint_fast8_t* modifiers);
static size_t matchnewline        (const char* text, size_t i, uint_fast8_t* modifiers);
static size_t matchwordboundary   (const char* text, size_t i, uint_fast8_t* modifiers);
static size_t matchnotwordboundary(const char* text, size_t i, uint_fast8_t* modifiers);

static size_t matchstart          (const char* text, size_t i, uint_fast8_t* modifiers);
static size_t matchend            (const char* text, size_t i, uint_fast8_t* modifiers);
static size_t matchany            (const char* text, size_t i, uint_fast8_t* modifiers);

/* printone: prints one regex token */
static void printone(re_Token pattern);
/* printoneclc: prints one class character */
static void printoneclc(ClassChar pattern);

/*
 * GLOBAL CONFIGURATION
 */

/* the array of all metabsls (sequences that begin with a backslash) */
const struct
{
	char pattern;
	size_t (*validator)(const char*, size_t, uint_fast8_t*);
}
metabsls[] =
{
	{'s', matchwhitespace},
	{'S', matchnotwhitespace},
	{'d', matchdigit},
	{'D', matchnotdigit},
	{'w', matchwordchar},
	{'W', matchnotwordchar},
	{'R', matchnewline},
	{'b', matchwordboundary},
	{'B', matchnotwordboundary}
};

/* the array of all metachars */
const struct
{
	char pattern;
	size_t (*validator)(const char*, size_t, uint_fast8_t*);
}
metachars[] =
{
	{'^', matchstart},
	{'$', matchend},
	{'.', matchany}
};

/* the array of all quantifiers */
const struct
{
	char pattern;
	uint_fast8_t min;
	uint_fast8_t max;
}
quantifiers[] =
{
	{'?', 0, 1},
	{'*', 0, UINT_FAST8_MAX},
	{'+', 1, UINT_FAST8_MAX}
};

#define MOD_I 0b00000001
#define MOD_S 0b00000010
#define MOD_B 0b00000100

const struct
{
	char pattern;
	uint_fast8_t modifier;
}
modifierchars[] =
{
	{'i', MOD_I}, /* case Insensitive */
	{'s', MOD_S}, /* Single line (DOTALL) */
	{'<', MOD_B}  /* backwards */
};

/*
 * GLOBAL VARIABLES
 */

re_Token* groupstack[MAXGROUPS] = {NULL};
size_t groupstacki = 0;

uint_fast8_t modifierstack[MAXGROUPS] = {0};
size_t modifierstacki = 0;

/*
 * COMPILATION FUNCTIONS
 */

void re_compile(Regex* compiled, const char* pattern)
{
	groupstacki = 0;
	compiled->ccli = 0;
	size_t pi = 0; /* index into pattern  */
	size_t ri = 0; /* index into compiled */

	while (pattern[pi] != '\0' && ri < MAXTOKENS) {
		errno = 0;
		pi += compileone(&compiled->tokens[ri], &pattern[pi], compiled->cclbuf, &compiled->ccli);
		if (errno)
			return;
		pi += compilequantifier(&compiled->tokens[ri], &pattern[pi]);
		if (errno)
			return;
		pi += compilegreedy(&compiled->tokens[ri], &pattern[pi]);
		if (errno)
			return;
		pi += compileatomic(&compiled->tokens[ri], &pattern[pi]);
		if (errno)
			return;

		++ri;
	}
	if (ri == MAXTOKENS) {
		/* regex is too large */
		errno = ENOBUFS;
		return;
	}
	/* indicate the end of the regex */
	compiled->tokens[ri].type = TOKEN_END;

	if (groupstacki)
		errno = EINVAL;
}

static size_t compileone(re_Token* compiled, const char* pattern, ClassChar cclbuf[CCLBUFLEN], size_t* ccli)
{
	size_t i;
	switch (pattern[0]) {
		case '\\':
			if (!pattern[1]) {
				/* invalid regex, has \ as last character */
				errno = EINVAL;
				return 0;
			}
			/* is a metabsls */
			for (size_t i = 0; i < sizeof(metabsls)/sizeof(metabsls[0]); ++i) {
				if (pattern[1] == metabsls[i].pattern) {
					/* metabackslash */
					compiled->type = TOKEN_METABSL;
					compiled->meta = i;
					return 2;
				}
			}
			/* literal escaped char */
			compiled->type = TOKEN_CHAR;
			compiled->ch = pattern[1];
			return 2;
		case '[':
			/* character class */
			compiled->type = TOKEN_CHARCLASS;
			compiled->ccl = &cclbuf[*ccli];
			i = 1;
			if (pattern[i] == '^') {
				++i;
				compiled->type = TOKEN_INVCHARCLASS;
			}
			while (pattern[i] && pattern[i] != ']') {
				errno = 0;
				if (*ccli >= CCLBUFLEN) {
					/* buffer is too small */
					errno = ENOBUFS; /* technically, this errno code refers to buffer space in a file stream, but I think it is still appropriate */
					return 0;
				}
				i += compileoneclc(&cclbuf[*ccli], pattern+i);
				if (errno)
					return 0;
				i += compilerange(&cclbuf[*ccli], pattern+i);
				if (errno)
					return 0;
				++*ccli;
			}
			if (!pattern[i]) {
				/* invalid regex, doesn't close the [ */
				errno = EINVAL;
				return 0;
			}
			if (*ccli >= CCLBUFLEN) {
				/* buffer is too small for null terminator */
				errno = ENOBUFS;
				return 0;
			}
			cclbuf[(*ccli)++].type = CCL_END;
			return i+1;
		case '(':
			/* group */
			compiled->type = TOKEN_GROUP;
			i = 1;

			compiled->capturing = true;
			if (pattern[i] == '?') {
				compiled->capturing = false;
				++i;
			}

			compiled->modifiers = 0;
			if (groupstacki)
				compiled->modifiers = groupstack[groupstacki-1]->modifiers;
			for (;;) {
				for (size_t j = 0; j < sizeof(modifierchars)/sizeof(modifierchars[0]); ++j) {
					if (pattern[i] == modifierchars[j].pattern) {
						compiled->modifiers |= modifierchars[j].modifier;
						++i;
						goto modifierfound;
					}
				}
				if (pattern[i] == '-') {
					++i;
					for (size_t j = 0; j < sizeof(modifierchars)/sizeof(modifierchars[0]); ++j) {
						if (pattern[i] == modifierchars[j].pattern) {
							compiled->modifiers &= ~modifierchars[j].modifier;
							++i;
							goto modifierfound;
						}
					}
					--i;
				}
				break;
modifierfound:
				continue;
			}

			compiled->grouptype = GROUP_NORMAL;
			if (pattern[i] == '=') {
				compiled->grouptype = GROUP_LOOKAROUND;
				++i;
			} else if (pattern[i] == '!') {
				compiled->grouptype = GROUP_INVERTED;
				++i;
			} else if (pattern[i] == ':') {
				++i;
			}

			if (groupstacki >= MAXGROUPS) {
				errno = ENOBUFS;
				return 0;
			}
			groupstack[groupstacki] = compiled;
			++groupstacki;
			return i;
		case ')':
			/* group end */
			compiled->type = TOKEN_GROUPEND;
			--groupstacki;
			return 1;
		case '\0':
			/* shouldn't happen */
			errno = EINVAL;
			return 0;
		default:
			/* is a metachar */
			for (size_t i = 0; i < sizeof(metachars)/sizeof(metachars[0]); ++i) {
				if (pattern[0] == metachars[i].pattern) {
					compiled->type = TOKEN_METACHAR;
					compiled->meta = i;
					return 1;
				}
			}
			
			/* literal char */
			compiled->type = TOKEN_CHAR;
			compiled->ch = pattern[0];
			return 1;
	}
	/* UNREACHABLE */
}

static size_t compileoneclc(ClassChar* compiled, const char* pattern)
{
	switch (pattern[0]) {
		case '\\':
			if (!pattern[1]) {
				/* invalid regex, doesn't end the \ or close the [ */
				errno = EINVAL;
				return 0;
			}
			/* is a metabsls */
			for (size_t i = 0; i < sizeof(metabsls)/sizeof(metabsls[0]); ++i) {
				if (pattern[1] == metabsls[i].pattern) {
					compiled->type = CCL_METABSL;
					compiled->meta = i;
					return 2;
				}
			}
			/* literal escaped char */
			compiled->type = CCL_CHARRANGE;
			compiled->first = pattern[1];
			return 2;
		case '\0': /* FALLTHROUGH */
		case ']':
			/* shouldn't happen */
			errno = EINVAL;
			return 0;
		default:
			/* literal char */
			compiled->type = CCL_CHARRANGE;
			compiled->first = pattern[0];
			return 1;
	}
	/* UNREACHABLE */
}

static size_t compilerange(ClassChar* compiled, const char* pattern)
{
	if (pattern[0] != '-') {
		/* not a range */
		if (compiled->type == CCL_CHARRANGE)
			compiled->last = compiled->first;
		return 0;
	}
	if (compiled->type != CCL_CHARRANGE) {
		/* the previous char was not range-able (e.g. was a metabsl [\w-b] ) */
		errno = EINVAL;
		return 0;
	}
	switch (pattern[1]) {
		case '\\':
			if (!pattern[2]) {
				/* invalid regex, ends on a backslash */
				errno = EINVAL;
				return 0;
			}
			for (size_t i = 0; i < sizeof(metabsls)/sizeof(metabsls[0]); ++i) {
				if (pattern[2] == metabsls[i].pattern) {
					/* a range from a character to a metabsl; error (e.g. [b-\w] )*/
					errno = EINVAL;
					return 0;
				}
			}
			/* range from a character to an escaped literal char (e.g. [H-\Z]) */
			compiled->last = pattern[2];
			return 4;
		case ']':
			/* ccl ends on dash; (e.g. [asdf-]); treat dash as literal */
			compiled->last = compiled->first;
			return 0;
		case '\0':
			/* ccl ends unclosed on dash; error */
			errno = EINVAL;
			return 0;
		default:
			/* regular range from char to char */
			compiled->last = pattern[1];
			return 2;
	}
	/* UNREACHABLE */
}

static size_t compilequantifier(re_Token* compiled, const char* pattern)
{
	compiled->quantifiermin = 1;
	compiled->quantifiermax = 1;

	/* is a quantifier char */
	for (size_t i = 0; i < sizeof(quantifiers)/sizeof(quantifiers[0]); ++i) {
		if (pattern[0] == quantifiers[i].pattern) {
			compiled->quantifiermin = quantifiers[i].min;
			compiled->quantifiermax = quantifiers[i].max;
			return 1;
		}
	}
	if (pattern[0] != '{')
		/* there is no quantifier */
		return 0;

	/* from now in, it checks inside the {} */

	size_t i;

	/* loop to check min quantifier */
	compiled->quantifiermin = 0;
	for (i = 1; pattern[i]; ++i) {
		if (isdigit(pattern[i])) {
			compiled->quantifiermin *= 10;
			compiled->quantifiermin += pattern[i] - '0';
		} else if (pattern[i] == ',') {
			/* start entering the max, but first if there is no max set it to UINT_FAST8_MAX (infinity) */
			++i;
			if (pattern[i] == '}') {
				compiled->quantifiermax = UINT_FAST8_MAX;
				return i+1;
			}
			break;
		} else if (pattern[i] == '}') {
			/* it only has one value with no comma */
			compiled->quantifiermax = compiled->quantifiermin;
			return i+1;
		} else {
			/* invalid character in {}, treat entire thing as literal */
			compiled->quantifiermin = 1;
			compiled->quantifiermax = 1;
			return 0;
		}
	}
	/* loop to check max quantifier */
	compiled->quantifiermax = 0;
	for (; pattern[i]; ++i) {
		if (isdigit(pattern[i])) {
			compiled->quantifiermax *= 10;
			compiled->quantifiermax += pattern[i] - '0';
		} else if (pattern[i] == '}') {
			/* finish entering max */
			return i+1;
		} else {
			/* invalid character in {}, treat entire thing as literal */
			compiled->quantifiermin = 1;
			compiled->quantifiermax = 1;
			return 0;
		}
	}
	if (!pattern[i])
		/* pattern ends on an open {, treat entire thing as literal */
		return 0;
	return i+1;
}

static size_t compilegreedy(re_Token* compiled, const char* pattern)
{
	switch (pattern[0]) {
		case '?':
			compiled->greedy = false;
			return 1;
		default:
			compiled->greedy = true;
			return 0;
	}
}

static size_t compileatomic(re_Token* compiled, const char* pattern)
{
	switch (pattern[0]) {
		case '+':
			compiled->atomic = true;
			return 1;
		default:
			compiled->atomic = false;
			return 0;
	}
}

/*
 * MATCHING FUNCTIONS
 */

size_t re_match(Regex pattern, const char* text, size_t* length)
{
	for (size_t i = 0; i == 0 || text[i-1]; ++i) {
		errno = 0;
		modifierstacki = 0;
		uint_fast8_t modifiers = 0;
		const size_t lengthBuf = matchpattern(pattern.tokens, text, i, &modifiers);
		if (modifierstacki) {
			errno = EINVAL;
			return 0;
		}
		if (!errno) {
			/* first successful match */
			if (length)
				*length = lengthBuf;
			return i;
		}
	}
	/* no matches */
	errno = EINVAL;
	return 0;
}

size_t re_matchg(Regex pattern, const char* text)
{
	size_t i = 0;
	size_t c = 0;
	while (text[i]) {
		size_t length = 0;
		errno = 0;
		i += re_match(pattern, text+i, &length);
		if (errno)
			return c;
		++c;
		i += length;
	}
	return c;
}

static size_t matchpattern(re_Token* pattern, const char* text, size_t i, uint_fast8_t* modifiers)
{
	const size_t starti = i;

	/* consume as many tokens as you can iteratively, to avoid as much recursion as possible */
	for (;;) {
		if (pattern[0].type == TOKEN_END)
			return 0;
		if (pattern[0].quantifiermin != pattern[0].quantifiermax && !pattern[0].atomic)
			break;
		uint_fast8_t count = pattern[0].greedy ? pattern[0].quantifiermax : pattern[0].quantifiermin;
		i += matchcount(pattern[0], &count, text, i, modifiers);
		if (count < pattern[0].quantifiermin)
			return 0;
		++pattern;
	}

	const size_t oldi = i;

	/* all the tokens that can be iteratively consumed have been, now recurse */
	uint_fast8_t count = pattern[0].greedy ? pattern[0].quantifiermax : pattern[0].quantifiermin;
	for (;;) {
		i = oldi;
		i += matchcount(pattern[0], &count, text, i, modifiers);
		if (count < pattern[0].quantifiermin)
			return 0;

		errno = 0;
		i += matchpattern(pattern+1, text, i, modifiers);

		if (!errno)
			break;

		if (pattern[0].greedy) {
			if (count <= pattern[0].quantifiermin)
				return 0;
			--count;
		} else {
			if (count >= pattern[0].quantifiermax)
				return 0;
			++count;
		}
	}

	errno = 0;
	return i-starti;
}

static size_t matchcount(re_Token pattern, uint_fast8_t* count, const char* text, size_t i, uint_fast8_t* modifiers)
{
	const size_t oldi = i;

	for (uint_fast8_t c = 0; c < *count; ++c) {
		errno = 0;
		i += matchone(pattern, text, i, modifiers);
		if (errno) {
			*count = c;
			return i-oldi;
		}
	}
	return i-oldi;
}

static size_t matchone(re_Token pattern, const char* text, size_t i, uint_fast8_t* modifiers)
{
	size_t chars;
	size_t ccli;
	switch (pattern.type) {
		case TOKEN_GROUP:
			/* TODO capturing */
			if (modifierstacki >= MAXGROUPS) {
				errno = ENOBUFS;
				return 0;
			}
			modifierstack[modifierstacki] =  *modifiers;
			++modifierstacki;
			*modifiers = pattern.modifiers;
			return 0;
		case TOKEN_GROUPEND:
			*modifiers = modifierstack[--modifierstacki];
			return 0;
		case TOKEN_METABSL:
			errno = 0;
			chars = metabsls[pattern.meta].validator(text, i, modifiers);
			if (errno)
				return 0;
			return chars;
		case TOKEN_METACHAR:
			errno = 0;
			chars = metachars[pattern.meta].validator(text, i, modifiers);
			if (errno)
				return 0;
			return chars;
		case TOKEN_CHARCLASS:
			ccli = 0;
			while (pattern.ccl[ccli].type != CCL_END) {
				errno = 0;
				i += matchoneclc(pattern.ccl[ccli], text, i, modifiers);
				if (!errno)
					return 1;
				++ccli;
			}
			/* all the chars in the class failed; matching failed */
			errno = EINVAL;
			return 0;
		case TOKEN_INVCHARCLASS:
			if (!text[i]) {
				errno = EINVAL;
				return 0;
			}
			ccli = 0;
			while (pattern.ccl[ccli].type != CCL_END) {
				errno = 0;
				i += matchoneclc(pattern.ccl[ccli], text, i, modifiers);
				if (!errno) {
					/* matchoneclc succeeded; fail the charclass */
					errno = EINVAL;
					return 0;
				}
				++ccli;
			}
			/* all the chars in the class failed; matching succeeded */
			errno = 0;
			return 1;
		case TOKEN_CHAR:
			if (
				( (*modifiers & MOD_I) && tolower(pattern.ch) != tolower(text[i])) ||
				(!(*modifiers & MOD_I) &&         pattern.ch  !=         text[i] )
			) {
				errno = EINVAL;
				return 0;
			}
			return 1;
		default:
			/* unknown re_Token type: should never happen */
			errno = EINVAL;
			return 0;
	}
	/* UNREACHABLE */
}

static size_t matchoneclc(ClassChar pattern, const char* text, size_t i, uint_fast8_t* modifiers)
{
	/* this function always returns 1 */
	switch (pattern.type) {
		case CCL_METABSL:
			errno = 0;
			metabsls[pattern.meta].validator(text, i, modifiers);
			if (errno)
				return 0;
			return 1;
		case CCL_CHARRANGE:
			if (
				( (*modifiers & MOD_I) && (tolower(text[i]) < tolower(pattern.first) || tolower(text[i]) > tolower(pattern.last))) ||
				(!(*modifiers & MOD_I) && (        text[i]  <         pattern.first  ||         text[i]  >         pattern.last ))
			) {
				errno = EINVAL;
				return 0;
			}
			return 1;
		default:
			/* should never happen */
			errno = EINVAL;
			return 0;
	}
	/* UNREACHABLE */
}

size_t matchwhitespace(const char* text, size_t i, uint_fast8_t* modifiers)
{
	UNUSED(modifiers);
	if (!isspace(text[i])) {
		errno = EINVAL;
		return 0;
	}
	return 1;
}
size_t matchnotwhitespace(const char* text, size_t i, uint_fast8_t* modifiers)
{
	UNUSED(modifiers);
	if (!text[i] || isspace(text[i])) {
		errno = EINVAL;
		return 0;
	}
	return 1;
}
size_t matchdigit(const char* text, size_t i, uint_fast8_t* modifiers)
{
	UNUSED(modifiers);
	if (!isdigit(text[i])) {
		errno = EINVAL;
		return 0;
	}
	return 1;
}
size_t matchnotdigit(const char* text, size_t i, uint_fast8_t* modifiers)
{
	UNUSED(modifiers);
	if (!text[i] || isdigit(text[i])) {
		errno = EINVAL;
		return 0;
	}
	return 1;
}
size_t matchwordchar(const char* text, size_t i, uint_fast8_t* modifiers)
{
	UNUSED(modifiers);
	if (!iswordchar(text[i])) {
		errno = EINVAL;
		return 0;
	}
	return 1;
}
size_t matchnotwordchar(const char* text, size_t i, uint_fast8_t* modifiers)
{
	UNUSED(modifiers);
	if (!text[i] || iswordchar(text[i])) {
		errno = EINVAL;
		return 0;
	}
	return 1;
}
size_t matchnewline(const char* text, size_t i, uint_fast8_t* modifiers)
{
	UNUSED(modifiers);
	if (text[i] == '\r' && text[i+1] == '\n')
		return 2;
	else if (text[i] == '\n')
		return 1;
	errno = EINVAL;
	return 0;
}
size_t matchwordboundary(const char* text, size_t i, uint_fast8_t* modifiers)
{
	UNUSED(modifiers);
	if (
		(i > 0 && iswordchar(text[i-1]) != !iswordchar(text[i])) ||
		(i == 0 && !iswordchar(text[0]))
	) {
		errno = EINVAL;
		return 0;
	}
	return 0;
}
size_t matchnotwordboundary(const char* text, size_t i, uint_fast8_t* modifiers)
{
	UNUSED(modifiers);
	if (
		(i > 0 && iswordchar(text[i-1]) == !iswordchar(text[i])) ||
		(i == 0 && iswordchar(text[0]))
	) {
		errno = EINVAL;
		return 0;
	}
	return 0;
}

size_t matchstart(const char* text, size_t i, uint_fast8_t* modifiers)
{
	UNUSED(text);
	UNUSED(modifiers);
	if (i) {
		errno = EINVAL;
		return 0;
	}
	return 0;
}
size_t matchend(const char* text, size_t i, uint_fast8_t* modifiers)
{
	UNUSED(modifiers);
	if (text[i] != '\0') {
		errno = EINVAL;
		return 0;
	}
	return 0;
}
size_t matchany(const char* text, size_t i, uint_fast8_t* modifiers)
{
	if (text[i] == '\0' || (!(*modifiers & MOD_S) && text[i] == '\n')) {
		errno = EINVAL;
		return 0;
	}
	return 1;
}

/*
 * PRINTING FUNCTIONS
 */

void re_print(Regex pattern)
{
	for (size_t i = 0; pattern.tokens[i].type != TOKEN_END; ++i) {
		printone(pattern.tokens[i]);
	}
	printf("\n");
}

static void printone(re_Token pattern)
{
	switch (pattern.type) {
		case TOKEN_METABSL:
			printf("\\%c", metabsls[pattern.meta].pattern);
			break;
		case TOKEN_METACHAR:
			printf("%c", metachars[pattern.meta].pattern);
			break;
		case TOKEN_CHARCLASS: /* fallthrough */
		case TOKEN_INVCHARCLASS:
			printf("[");
			if (pattern.type == TOKEN_INVCHARCLASS)
				printf("^");
			for (size_t i = 0; pattern.ccl[i].type != CCL_END; ++i)
				printoneclc(pattern.ccl[i]);
			printf("]");
			break;
		case TOKEN_CHAR:
			printf("%c", pattern.ch);
			break;
		default:
			/* unknown token type: shouldn't happen */
			errno = EINVAL;
			return;
	}
	for (size_t i = 0; i < sizeof(quantifiers)/sizeof(quantifiers[0]); ++i) {
		if (pattern.quantifiermin == quantifiers[i].min && pattern.quantifiermax == quantifiers[i].max) {
			printf("%c", quantifiers[i].pattern);
			goto nocharquantifier;
		}
	}
	if (pattern.quantifiermin != 1 || pattern.quantifiermax != 1) {
		printf("{");
		if (pattern.quantifiermin != 0)
			printf("%"PRIuFAST8, pattern.quantifiermin);
		if (pattern.quantifiermax == UINT_FAST8_MAX)
			printf(",");
		else if (pattern.quantifiermax != pattern.quantifiermin)
			printf(",%"PRIuFAST8, pattern.quantifiermax);
		printf("}");
	}
nocharquantifier:
	if (!pattern.greedy)
		printf("?");
	if (pattern.atomic)
		printf("+");
}

static void printoneclc(ClassChar pattern)
{
	switch (pattern.type) {
		case CCL_METABSL:
			printf("\\%c", metabsls[pattern.meta].pattern);
			break;
		case CCL_CHARRANGE:
			printf("%c", pattern.first);
			if (pattern.last != pattern.first) {
				printf("-%c", pattern.last);
			}
			break;
		default:
			/* shouldn't happen */
			errno = EINVAL;
			return;
	}
}
