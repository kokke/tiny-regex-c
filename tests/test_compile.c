#include <assert.h>
#include <stdlib.h> /* for NULL */
#include "re.h"


static void test_class_buffer_limit(size_t class_length, int should_compile)
{
  char pattern[42];
  size_t i;
  re_t compiled;

  pattern[0] = '[';
  for (i = 0; i < class_length; ++i)
  {
    pattern[i + 1] = 'x';
  }
  pattern[class_length + 1] = ']';
  pattern[class_length + 2] = '\0';

  compiled = re_compile(pattern);
  assert((compiled != NULL) == should_compile);
}

int main(void)
{
  int matchlength = 0;
  static const unsigned char issue_pattern[] = {91, 1, 45, 25, 92, 0};
  static const unsigned char issue_text[] = {92, 92, 92, 92, 92, 0};
  static const unsigned char trailing_escape[] = {92, 0};
  static const unsigned char trailing_escape_text[] = {48, 0, 0};

  /* Issue #44: incomplete inverted classes and incomplete class escapes. */
  assert(re_compile("\\\x01[^\\\xff][^") == NULL);
  assert(re_compile("\\\x01[^\\\xff][\\") == NULL);

  /* Issue #101: an invalid class must not poison the next compilation. */
  assert(re_match((const char*)issue_pattern,
                  (const char*)issue_text, &matchlength) == -1);
  assert(matchlength == 0);
  assert(re_match((const char*)trailing_escape,
                  (const char*)trailing_escape_text, &matchlength) == -1);
  assert(matchlength == 0);
  assert(re_compile((const char*)trailing_escape) == NULL);

  /* Valid compilations still work after malformed patterns. */
  assert(re_match("[a-z]", "q", &matchlength) == 0);
  assert(matchlength == 1);

  /* Literal, escaped, and trailing escaped class characters remain valid. */
  assert(re_match("[-]", "-", &matchlength) == 0);
  assert(re_match("[\\-]", "-", &matchlength) == 0);
  assert(re_match("[a-]", "-", &matchlength) == 0);
  assert(re_match("[a\\-]", "-", &matchlength) == 0);
  assert(re_match("[a\\]]", "]", &matchlength) == 0);

  /* A 38-byte class plus its terminator fits; one more byte does not. */
  test_class_buffer_limit(38, 1);
  test_class_buffer_limit(39, 0);
  assert(re_match("[z]", "z", &matchlength) == 0);
  assert(matchlength == 1);

  return 0;
}
