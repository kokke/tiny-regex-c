/*

This file tests two bug patterns reported by @DavidKorczynski in https://github.com/kokke/tiny-regex-c/issues/44

*/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h> /* for NULL */
#include "re.h"


int main()
{
  printf("Testing handling of invalid regex patterns:\n");

  /* Test 1: inverted set without a closing ']' */
  assert(re_compile("\\\x01[^\\\xff][^") == NULL);

  /* Test 2: set with an incomplete escape sequence and without a closing ']' */
  assert(re_compile("\\\x01[^\\\xff][\\") == NULL);

  printf(" 2/2 tests succeeded.\n");
  return 0;
}

