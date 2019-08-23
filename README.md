# tiny-regex-c: a small regex implementation in C
## Description
Small and portable [Regular Expression](https://en.wikipedia.org/wiki/Regular_expression) (regex) library written in C. 

Design is inspired by Rob Pike's regex-code for the book *"Beautiful Code"* [available online here](http://www.cs.princeton.edu/courses/archive/spr09/cos333/beautiful.html).

Supports a subset of the syntax and semantics of the Python standard library implementation (the `re`-module).

## Current status
All supported regex-operators seem to work properly according to the test-set.

I think you should test the patterns you are going to use. You can easily modify the test-harness to generate tests for your intended patterns to check for compliance.

**I will gladly accept patches correcting bugs.**

## Design goals
The main design goal of this library is to be small, correct, self contained and use few resources while retaining acceptable performance and feature completeness. Clarity of the code is also highly valued.

## Notable features and omissions
- No use of dynamic memory allocation (i.e. no calls to `malloc` or `free`).
- No support for multiline mode, \A, \z or \Z; use ^, $ and \R instead.
- No octal, hexadecimal, unicode or control character escape sequences; use C's built-in ones instead.
- No POSIX classes (e.g. [:alnum:]).
- No atomic groups (?>...); use the atomic quantifier instead.
- No support for named capture: `(^P<name>group)` etc.
- No inline modifier changes (?i) etc; use groups instead.
- Groups are reworked to act the same way but provide more features. Now, a group consists of the following items:
	- An open bracket.
	- Nothing if the group is capturing, and ? if it isn't.
	- A list of modifiers in any order
	- An equals sign if the group is a lookaround (doesn't 'eat' any characters); an exclamation mark if the group is an inverted lookaround; a colon if the group has any characters between the open bracket and the colon, and nothing otherwise.
	- A regex.
	- A close bracket.

	You cannot do a capturing lookaround (=regex), (!regex).
- For testing, [exrex](https://github.com/asciimoo/exrex) is used to randomly generate test-cases from regex patterns, which are fed into the regex code for verification. Try `make test` to generate a few thousand tests cases yourself.
- Small code and binary size: <1000 SLOC, ~6kb binary for x86. Statically #define'd memory usage / allocation.
- Compiled for x86 using GCC 8.3.0 and optimizing for size, the binary takes up ~6kb code space and allocates ~0.2kb RAM:
  ```
  > gcc -Os -c re.c
  > size re.o
   text    data     bss     dec     hex filename
   5797     208      72    6077    17bd re.o
      
  ```

## API
This is the public / exported API:
```C
/* re_compile: compile regex string pattern to a Regex */
size_t re_compile(Regex* compiled, const char* pattern);

/* re_match: returns index of first match of pattern in text */
/* stores the length of the match in length if it is not NULL */
size_t re_match(Regex pattern, const char* text, size_t* length);

/* re_matchg: returns number of matches of pattern in text */
size_t re_matchg(Regex pattern, const char* text);

/* re_print: prints a regex to stdout */
void re_print(Regex pattern);
```

## Supported regex-operators
The following features / regex-operators are supported by this library.

 - `.`         Dot, matches any character
 - `^`         Start anchor, matches beginning of string
 - `$`         End anchor, matches end of string
 - `*`         Asterisk, match zero or more (greedy)
 - `+`         Plus, match one or more (greedy)
 - `?`         Question, match zero or one (greedy)
 - `?`         Question, make quantifier non-greedy
 - `+`         Plus, make quantifier atomic
 - `[abc]`     Character class, match if one of {'a', 'b', 'c'}
 - `[^abc]`   Inverted class, match if NOT one of {'a', 'b', 'c'}
 - `[a-zA-Z]` Character ranges, the character set of the ranges { a-z | A-Z }
 - `\s`       Whitespace, \t \f \r \n \v and spaces
 - `\S`       Non-whitespace
 - `\d`       Digits, [0-9]
 - `\D`       Non-digits
 - `\w`       Alphanumeric, [a-zA-Z0-9_]
 - `\W`       Non-alphanumeric
 - `\R`       Any newline (\r\n or \n)
 - `\b`       A word boundary (where one side is \w and the other is \W)
 - `\B`       Non-word boundary
 - `i`        case Insensitive modifier
 - `s`        Single line modifier (where a dot matches newlines too)
 - `()`       Groups (capturing is not currently implemented)
 - `(?:)`     Non-capturing groups
 - `(?is:)`   Non-capturing groups with modifiers
 - `(?=)`     Lookaheads
 - `(?!)`     Inverted lookaheads

## Usage
Compile a regex from ASCII-string (char-array) to a custom pattern structure using `re_compile()`.

Search a text-string for a regex and get an index into the string, using `re_match()`.

The returned index points to the first place in the string, where the regex pattern matches.

If the regular expression doesn't match, then 0 is returned and errno is set.

## Examples
Example of usage:
```C
/* Standard null-terminated C-string to search: */
const char* string_to_search = "ahem.. 'hello world !' ..";

Regex pattern;

/* Compile a simple regular expression using character classes, meta-char and quantifiers: */
re_compile(&pattern, "[Hh]ello [Ww]orld\\s*[!]?");

/* Check if the regex matches the text: */
errno = 0;
size_t length;
size_t match_idx = re_match(pattern, string_to_search, &length);
if (!errno)
	printf("Match at index %zu with length %zu.\n", match_idx, length);
```

For more usage examples I encourage you to look at the code in the `tests`-folder, as well as `example.c` for a simple `grep` implementation.

## TODO
- Implement captures.
- Implement backwards (<) modifier for lookbehinds.
- Implement branches (| operator).
- Make groupstack a local variable for thread-safety.
- Add file sizes for other architectures in README.md.
- Add `tests/speed.c` for performance and time measurements.

## License
All material in this repository is in the public domain.

