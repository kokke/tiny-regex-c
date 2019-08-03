# tiny-regex-c: a small regex implementation in C
### Description
Small and portable [Regular Expression](https://en.wikipedia.org/wiki/Regular_expression) (regex) library written in C. 

Design is inspired by Rob Pike's regex-code for the book *"Beautiful Code"* [available online here](http://www.cs.princeton.edu/courses/archive/spr09/cos333/beautiful.html).

Supports a subset of the syntax and semantics of the Python standard library implementation (the `re`-module).

### Current status
All supported regex-operators seem to work properly according to the test-set.

I think you should test the patterns you are going to use. You can easily modify the test-harness to generate tests for your intended patterns to check for compliance.

**I will gladly accept patches correcting bugs.**

### Design goals
The main design goal of this library is to be small, correct, self contained and use few resources while retaining acceptable performance and feature completeness. Clarity of the code is also highly valued.

### Notable features and omissions
- Small code and binary size: <1000 SLOC, ~5kb binary for x86. Statically #define'd memory usage / allocation.
- No use of dynamic memory allocation (i.e. no calls to `malloc` / `free`).
- No support for multiline mode, \A, \z or \Z; use ^, $ and \R instead.
- No octal, hexadecimal, unicode or control character escape sequences; use C's built-in ones instead.
- No POSIX classes (e.g. [:alnum:]).
- No atomic groups (?>...); use the atomic quantifier instead.
- No support for capturing groups or named capture: `(^P<name>group)` etc.
- Groups are reworked to act the same way but provide more features. Now, a group consists of the following items:
	- An open bracket.
	- Nothing if the group is capturing, and ? if it isn't.
	- A list of modifiers in any order, with each one being preceeded by a dash (minus sign) to disable it.
	- An equals sign if the group is a lookaround (doesn't 'eat' any characters); an exclamation mark if the group is inverted, and nothing or a colon otherwise.
	- A regex.
	- A close bracket.

	Note that a capturing inverted group (e.g. (!regex) ) will always capture nothing.
- Thorough testing : [exrex](https://github.com/asciimoo/exrex) is used to randomly generate test-cases from regex patterns, which are fed into the regex code for verification. Try `make test` to generate a few thousand tests cases yourself.
- Compiled for x86 using GCC 4.7.4 and optimizing for size, the binary takes up ~2-3kb code space and allocates ~0.5kb RAM :
  ```
  > gcc -Os -c re.c
  > size re.o
      text     data     bss     dec     hex  filename
      4701      208       0    4909     132d re.o
      
  ```

### API
This is the public / exported API:
```C
/* re_compile: compile regex string pattern to a Regex */
void re_compile(Regex* compiled, const char* pattern);

/* re_match: returns index of first match of pattern in text */
/* stores the length of the match in length if it is not NULL */
size_t re_rmatch(Regex pattern, const char* text, size_t* length);
size_t re_smatch(const char* pattern, const char* text, size_t* length);

/* re_matchg: returns number of matches of pattern in text */
size_t re_rmatchg(Regex pattern, const char* text);
size_t re_smatchg(const char* pattern, const char* text);

/* re_print: prints a regex to stdout */
void re_print(Regex pattern);
```

### Supported regex-operators
The following features / regex-operators are supported by this library.

  -  `.`         Dot, matches any character
  -  `^`         Start anchor, matches beginning of string
  -  `$`         End anchor, matches end of string
  -  `*`         Asterisk, match zero or more (greedy)
  -  `+`         Plus, match one or more (greedy)
  -  `?`         Question, match zero or one (greedy)
  -  `?`         Question, make quantifier non-greedy
  -  `+`         Plus, make quantifier atomic
  -  `[abc]`     Character class, match if one of {'a', 'b', 'c'}
  -  `[^abc]`   Inverted class, match if NOT one of {'a', 'b', 'c'}
  -  `[a-zA-Z]` Character ranges, the character set of the ranges { a-z | A-Z }
  -  `\s`       Whitespace, \t \f \r \n \v and spaces
  -  `\S`       Non-whitespace
  -  `\d`       Digits, [0-9]
  -  `\D`       Non-digits
  -  `\w`       Alphanumeric, [a-zA-Z0-9_]
  -  `\W`       Non-alphanumeric
  -  `\R`       Any newline (\r\n or \n)
  -  `\b`       A word boundary (where one side is \w and the other is \W)
  -  `\B`       Non-word boundary

### Usage
Compile a regex from ASCII-string (char-array) to a custom pattern structure using `re_compile()`.

Search a text-string for a regex and get an index into the string, using `re_smatch()` or `re_rmatch()`.

The returned index points to the first place in the string, where the regex pattern matches.

If the regular expression doesn't match, then 0 is returned and errno is set.

### Examples
Example of usage:
```C
/* Standard null-terminated C-string to search: */
const char* string_to_search = "ahem.. 'hello world !' ..";

Regex pattern;

/* Compile a simple regular expression using character classes, meta-char and greedy + non-greedy quantifiers: */
re_compile(&pattern, "[Hh]ello [Ww]orld\\s*[!]?");

/* Check if the regex matches the text: */
errno = 0;
size_t match_idx = re_rmatch(pattern, string_to_search);
if (!errno)
	printf("match at idx %d.\n", match_idx);
```

For more usage examples I encourage you to look at the code in the `tests`-folder.

### TODO
- tests that include atomics, word boundaries and \R
- remove unnecessary comments
- Add `example.c` that demonstrates usage.
- implement | operator
- implement i (case Insensitive) and s (Single line/DOTALL) modifiers
- implement groups and captures
- Add `tests/test_perf.c` for performance and time measurements.

### License
All material in this repository is in the public domain.

