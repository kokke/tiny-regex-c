/*

This file tests two bug patterns reported by @DavidKorczynski in https://github.com/kokke/tiny-regex-c/issues/44

*/

#include <assert.h>
#include <stdlib.h> /* for NULL */
#include <string.h>
#include "re.h"


int main()
{
  /* Test 1: inverted set without a closing ']' */
  assert(re_compile("\\\x01[^\\\xff][^") == NULL);

  /* Test 2: set with an incomplete escape sequence and without a closing ']' */
  assert(re_compile("\\\x01[^\\\xff][\\") == NULL);


  /* Test 3: a failed char-class compile must not leave a CHAR_CLASS token
     behind for the next call (issue #104). */
  {
    char overlong[64];
    int len = -1;

    memset(overlong, 'a', sizeof(overlong));
    overlong[0] = '[';
    overlong[40] = ']';
    overlong[41] = '\0';

    assert(re_compile(overlong) == NULL);

    /* A lone trailing backslash emits no token, so matching it must
       simply report the empty match, not follow a stale class pointer. */
    assert(re_match("\\", "b", &len) == 0);
    assert(len == 0);
  }

  return 0;
}

