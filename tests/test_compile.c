/*

This file tests two bug patterns reported by @DavidKorczynski in https://github.com/kokke/tiny-regex-c/issues/44

*/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h> /* for NULL */
#include "re.h"
void re_print(re_t pattern);

int main()
{
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
    /* quantifiers without context: nothing to repeat at position 0 */
    "+", "?", "*",
    /* Tests 7-12: invalid quantifiers. */
    "x{0}", "x{2,1}",
    /* note that python and perl allows these, and matches them exact. */
    // "{2}", "x{}", "x{1,2,}", "x{,2,}", "x{-2}",
  };

  for (size_t i = 0; i < sizeof(tests)/sizeof(*tests); i++)
  {
    const char *s = tests[i];
    ntests++;
    re_t p = re_compile(s);
    if (p != NULL)
    {
      printf(" [%u] re_compile(\"%s\") must not compile.\n", (unsigned)i+1, s);
      re_print(p);
      failed++;
    }
  }

  printf(" %d/%d tests succeeded.\n", ntests-failed+1, ntests+1);
  return 0;
}

