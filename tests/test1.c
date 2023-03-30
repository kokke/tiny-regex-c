/*
 * Testing various regex-patterns
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef _UNICODE
#  include <locale.h>
#endif
#include "re.h"

struct test_case {
  char *rx;
  char *text;
  int len;
};

void re_print(re_t);

/* "\\n" => "\n" */
char *cunquote (char* s, int l)
{
    int i;
    char *r = malloc (l + 1);
    for (i=0; i<l; i++)
    {
      if (*s == '\\' && i+1 < l)
      {
        if (*(s+1) == 'n')
          r[i] = '\n', s+=2, l--;
        else if (*(s+1) == 'r')
          r[i] = '\r', s+=2, l--;
        else if (*(s+1) == 't')
          r[i] = '\t', s+=2, l--;
        else
          r[i] = *s++;
      }
      else
      {
        r[i] = *s++;
      }
    }
    r[i] = '\0';
    return r;
}

struct test_case* read_tests (const char *fname, int *ntests)
{
    char s[80];
    FILE *f = fopen (fname, "rt");
    if (!f)
      return NULL;
    int size = 120;
    struct test_case* vec = calloc (size, sizeof(struct test_case));
    int i = 0;
    *ntests = 0;
    while (fgets(s, 80, f))
    {
        // regex until first tab
        char *p = strchr (s, '\t');
        int l;
        if (!p) // no tab, just an old exreg test
          continue;
        if (s[0] == '#') // outcommented
          continue;
        l = p - s;
        vec[i].rx = cunquote (s, l);
        // string from first tab
        char *str = strchr (p, '"');
        if (!str)
          return NULL;
        char *end = strchr (str+1, '"');
        if (!end)
          return NULL;
        l = end - str - 1;
        vec[i].text = cunquote (&str[1], l);

        vec[i].len = 0;
        sscanf(&end[1], "\t%d", &vec[i].len);

        i++;
        if (i >= size)
        {
            size *= 2;
            vec = realloc (vec, size * sizeof(struct test_case));
        }
    }
    *ntests = i;
    return vec;
}

void free_test_cases (struct test_case* test_case, int ntests)
{
    for (int i=0; i < ntests; i++)
    {
        free (test_case[i].rx);
        free (test_case[i].text);
    }
    free (test_case);
}

int do_test (struct test_case* test_case, int i, int ntests, int ok)
{
    char* text;
    char* pattern;
    int should_fail;
    int length;
    int correctlen;
    int nfailed = 0;

    pattern = test_case[i].rx;
    text = test_case[i].text;
    should_fail = ok == 0;
    correctlen = test_case[i].len;

    int m = re_match(pattern, text, &length);

    if (should_fail)
    {
        if (m != (-1))
        {
            printf("\n");
            re_print(re_compile(pattern));
            fprintf(stderr, "[%d/%d]: pattern '%s' matched '%s' unexpectedly, matched %i chars. \n", i+1, ntests, pattern, text, length);
            nfailed += 1;
        }
    }
    else
    {
        if (m == (-1))
        {
            printf("\n");
            re_print(re_compile(pattern));
            fprintf(stderr, "[%d/%d]: pattern '%s' didn't match '%s' as expected. \n", (i+1), ntests, pattern, text);
            nfailed += 1;
        }
        else if (length != correctlen)
        {
            printf("\n");
            re_print(re_compile(pattern));
            fprintf(stderr, "[%d/%d]: pattern '%s' matched '%i' chars of '%s'; expected '%i'. \n", (i+1), ntests, pattern, length, text, correctlen);
            nfailed += 1;
        }
    }
    return nfailed;
}

int main()
{
    int ntests, ntests_nok;
    int nfailed = 0;
    int i;

    printf("Testing hand-picked regex patterns\n");
    
    //setlocale(LC_CTYPE, "en_US.UTF-8");
    struct test_case* tests_ok = read_tests ("tests/ok.lst", &ntests);
    for (i = 0; i < ntests; ++i)
    {
        nfailed += do_test (tests_ok, i, ntests, 1);
    }
    free_test_cases (tests_ok, ntests);

    struct test_case* tests_nok = read_tests ("tests/nok.lst", &ntests_nok);
    for (i = 0; i < ntests_nok; ++i)
    {
        nfailed += do_test (tests_nok, i, ntests_nok, 0);
    }
    free_test_cases (tests_nok, ntests_nok);
    ntests += ntests_nok;

    // regression test for unhandled BEGIN in the middle of an expression
    // we need to test text strings with all possible values for the second
    // byte because re.c was matching it against an uninitalized value, so
    // it could be anything
    int length;
    const char* pattern = "a^";
    for (i = 0; i < 255; i++) {
      char text_buf[] = { 'a', i, '\0' };
      int m = re_match(pattern, text_buf, &length);
      if (m != -1) {
        fprintf(stderr, "[%d/%d]: pattern '%s' matched '%s' unexpectedly", ntests, ntests, pattern, text_buf);
        nfailed += 1;
        break;
      }
    }
    ntests++;
    printf(" %d/%d tests succeeded.\n", ntests - nfailed, ntests);

    return nfailed; /* 0 if all tests passed */
}
