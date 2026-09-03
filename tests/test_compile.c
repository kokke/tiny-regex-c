/*

This file tests two bug patterns reported by @DavidKorczynski in https://github.com/kokke/tiny-regex-c/issues/44

*/

#include <assert.h>
#include <stdlib.h> /* for NULL */
#include "re.h"


int main()
{
  /* Test 1: inverted set without a closing ']' */
  assert(re_compile("\\\x01[^\\\xff][^") == NULL);

  /* Test 2: set with an incomplete escape sequence and without a closing ']' */
  assert(re_compile("\\\x01[^\\\xff][\\") == NULL);

  /* Test 3: a pattern ending in a lone, unescaped backslash is itself
     invalid and should be rejected outright, rather than silently
     leaving the corresponding re_compiled[] slot untouched. re_compiled
     and ccl_buf are function-static, so if this slot kept whatever a
     previous, unrelated compile call had left there, a later match
     could read a dangling/NULL character-class pointer and crash. See
     https://github.com/kokke/tiny-regex-c/issues/101 */
  {
    int matchlength = 0;

    /* Leave a CHAR_CLASS type with no ccl pointer assigned behind in
       re_compiled[0]: this pattern fails to compile partway through
       parsing "[...\" because the class is missing both a closing ']'
       and a character after the escape, but only after the class's
       type tag has already been written. */
    assert(re_compile("[\x01-\x19\\") == NULL);

    /* A pattern that is just a trailing backslash used to fall through
       re_compile() without touching re_compiled[0] at all, so it would
       inherit that leftover CHAR_CLASS tag from the compile above and
       crash trying to match against it. It must be rejected instead. */
    assert(re_compile("\\") == NULL);
    assert(re_match("\\", "\\\\\\\\\\", &matchlength) == -1);

    /* A subsequent, unrelated, well-formed pattern must still compile
       and match correctly, confirming the earlier failed compiles left
       no poisoned state behind for it to trip over. */
    assert(re_match("[0-9]+", "abc123", &matchlength) == 3);
    assert(matchlength == 3);
  }

  return 0;
}

