/*

This file tests two bug patterns reported by @DavidKorczynski in https://github.com/kokke/tiny-regex-c/issues/44

*/

#include <stdlib.h>
#include <assert.h>
#include "re.h"

const char* invalid_patterns[] = {
  "\\\x01[^\\\xff][^",
  "\\\x01[^\\\xff][\\",
};


int main(void)
{
  const size_t npatterns = sizeof(invalid_patterns)/sizeof(*invalid_patterns);
  size_t i;

  /* loop through all invalid patterns and assume compilation rejects them (returns NULL) */
  for (i = 0U; i < npatterns; ++i)
  {
    assert(re_compile(invalid_patterns[i]) == NULL);

  }
  return 0;
}
