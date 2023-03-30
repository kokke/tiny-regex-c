/*

This file tests two bug patterns reported by @DavidKorczynski in
https://github.com/kokke/tiny-regex-c/issues/44

And some structural issues with nested groups.

*/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h> /* for NULL */
#include "re.h"

void re_print(re_t pattern);
typedef struct regex_t
{
  unsigned type;     /* CHAR, STAR, etc.                      */
  union
  {
    char  ch;            /*      the character itself             */
    char* ccl;           /*  OR  a pointer to characters in class */
    unsigned char  group_num;   /*  OR the number of group patterns. */
    unsigned char  group_start; /*  OR for GROUPEND, the start index of the group. */
    struct {
      unsigned short n;  /* match n times */
      unsigned short m;  /* match n to m times */
    };
  } u;
} regex_t;

int main()
{
  size_t i;
  int failed = 0;
  int ntests = 0;
  printf("Testing handling of invalid regex patterns:\n");
  const char *const tests[] = {
    /* Test 1: inverted set without a closing ']' */
    "\\\x01[^\\\xff][^",
    /* Test 2: set with an incomplete escape sequence and without a closing ']' */
    "\\\x01[^\\\xff][\\",
    /* Invalid escape. '\\' as last char without previous \\ */
    "\\",
    /* incomplete char classes */
    "[^", "[abc\\",
    /* overlong char classes */
    "[0123456789012345678901234567890123456789]",
    "[01234567890123456789\\0123456789012345678]",
    "[00000000000000000000000000000000000000][",
    /* quantifiers without context: nothing to repeat at position 0 */
    "+", "?", "*",
    /* Tests 7-12: invalid quantifiers. */
    /* note that python and perl allows these, and matches them exact. */
    // "{2}", "x{}", "x{1,2,}", "x{,2,}", "x{-2}",
  };

  for (i = 0; i < sizeof(tests)/sizeof(*tests); i++)
  {
    const char *s = tests[i];
    ntests++;
    regex_t *p = re_compile(s);
    if (p != NULL)
    {
      printf(" [%d] re_compile(\"%s\") must not compile.\n", ntests, s);
      re_print(p);
      failed++;
    }
  }
  printf(" %d/%d tests succeeded.\n", ntests-failed, ntests);

  printf("Testing compilation of nested groups:\n");
  re_t p = re_compile("((ab)|b)+");
  int printed = 0;

  ntests++;
  if (p[0].u.group_num != 6)
  {
    printf(" [%d] wrong [0].group_num %hu for ((ab)|b)+\n", ntests, p[0].u.group_num);
    if (!printed)
      re_print(p);
    printed = 1;
    failed++;
  }
  ntests++;
  if (p[1].u.group_num != 2)
  {
    printf(" [%u] wrong [1].group_num %hu.\n", ntests, p[1].u.group_num);
    if (!printed)
      re_print(p);
    printed = 1;
    failed++;
  }
  ntests++;
  if (p[4].u.group_start != 1)
  {
    printf(" [%u] wrong [4].group_start %hu.\n", ntests, p[4].u.group_start);
    if (!printed)
      re_print(p);
    printed = 1;
    failed++;
  }
  ntests++;
  if (p[7].u.group_start != 0)
  {
    printf(" [%u] wrong [7].group_start %hu.\n", ntests, p[7].u.group_start);
    if (!printed)
      re_print(p);
    printed = 1;
    failed++;
  }

  printf(" %d/%d tests succeeded.\n", ntests-failed, ntests);
  return failed ? 1 : 0;
}

