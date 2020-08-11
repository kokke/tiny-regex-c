/*

This file tests two bug patterns reported by @DavidKorczynski in https://github.com/kokke/tiny-regex-c/issues/44

*/


#include <assert.h>
#include "re.h"


int main()
{
  char pattern1[] = "\\\x01[^\\\xff][^";
  void* p1 = re_compile(pattern1);
  assert(p1 == 0);

  char pattern2[] = "\\\x01[^\\\xff][\\";
  void* p2 = re_compile(pattern2);
  assert(p2 == 0);

  return 0;
}

