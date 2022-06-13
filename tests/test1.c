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

/*
#define OK    ((char*) 1)
#define NOK   ((char*) 0)

char* test_vector[][4] =
{
  { OK,  "\\d",                       "5",                (char*) 1      },
  { OK,  "\\w+",                      "hej",              (char*) 3      },
  { OK,  "\\s",                       "\t \n",            (char*) 1      },
  { NOK, "\\S",                       "\t \n",            (char*) 0      },
  { OK,  "[\\s]",                     "\t \n",            (char*) 1      },
  { NOK, "[\\S]",                     "\t \n",            (char*) 0      },
  { NOK, "\\D",                       "5",                (char*) 0      },
  { NOK, "\\W+",                      "hej",              (char*) 0      },
  { OK,  "[0-9]+",                    "12345",            (char*) 5      },
  { OK,  "\\D",                       "hej",              (char*) 1      },
  { NOK, "\\d",                       "hej",              (char*) 0      },
  { OK,  "[^\\w]",                    "\\",               (char*) 1      },
  { OK,  "[\\W]",                     "\\",               (char*) 1      },
  { NOK, "[\\w]",                     "\\",               (char*) 0      },
  { OK,  "[^\\d]",                    "d",                (char*) 1      },
  { NOK, "[\\d]",                     "d",                (char*) 0      },
  { NOK, "[^\\D]",                    "d",                (char*) 0      },
  { OK,  "[\\D]",                     "d",                (char*) 1      },
  { OK,  "^.*\\\\.*$",                "c:\\Tools",        (char*) 8      },
  { OK,  "^[\\+-]*[\\d]+$",           "+27",              (char*) 3      },
  { OK,  "[abc]",                     "1c2",              (char*) 1      },
  { NOK, "[abc]",                     "1C2",              (char*) 0      },
  { OK,  "[1-5]+",                    "0123456789",       (char*) 5      },
  { OK,  "[1-5-]+",                   "123-",             (char*) 4      },
  { OK,  "[1-5-]+[-1-2]-[-]", 	      "13132231--353444-511--",    (char *) 22  },
  { OK,  "[.2]",                      "1C2",              (char*) 1      },
  { OK,  "a*$",                       "Xaa",              (char*) 2      },
  { OK,  "a*$",                       "Xaa",              (char*) 2      },
  { OK,  "[a-h]+",                    "abcdefghxxx",      (char*) 8      },
  { NOK, "[a-h]+",                    "ABCDEFGH",         (char*) 0      },
  { OK,  "[A-H]+",                    "ABCDEFGH",         (char*) 8      },
  { NOK, "[A-H]+",                    "abcdefgh",         (char*) 0      },
  { OK,  "[^\\s]+",                   "abc def",          (char*) 3      },
  { OK,  "[^fc]+",                    "abc def",          (char*) 2      },
  { OK,  "[^d\\sf]+",                 "abc def",          (char*) 3      },
  { OK,  "\n",                        "abc\ndef",         (char*) 1      },
  { OK,  "b.\\s*\n",                  "aa\r\nbb\r\ncc\r\n\r\n",(char*) 4 },
  { OK,  ".*c",                       "abcabc",           (char*) 6      },
  { OK,  ".+c",                       "abcabc",           (char*) 6      },
  { OK,  "[b-z].*",                   "ab",               (char*) 1      },
  { OK,  "b[k-z]*",                   "ab",               (char*) 1      },
  { NOK, "[0-9]",                     "  - ",             (char*) 0      },
  { OK,  "[^0-9]",                    "  - ",             (char*) 1      },
  { OK,  "0|",                        "0",                (char*) 1      }, // 42
  { OK,  "0|",                        "",                 (char*) 0      },
  { OK,  "0|",                        "0|",               (char*) 1      },
  { OK,  "^0|",                       "x0",               (char*) 0      },
  { NOK, "\\d\\d:\\d\\d:\\d\\d",      "0s:00:00",         (char*) 0      },
  { NOK, "\\d\\d:\\d\\d:\\d\\d",      "000:00",           (char*) 0      },
  { NOK, "\\d\\d:\\d\\d:\\d\\d",      "00:0000",          (char*) 0      },
  { NOK, "\\d\\d:\\d\\d:\\d\\d",      "100:0:00",         (char*) 0      },
  { NOK, "\\d\\d:\\d\\d:\\d\\d",      "00:100:00",        (char*) 0      },
  { NOK, "\\d\\d:\\d\\d:\\d\\d",      "0:00:100",         (char*) 0      },
  { OK,  "\\d\\d?:\\d\\d?:\\d\\d?",   "0:0:0",            (char*) 5      },
  { OK,  "\\d\\d?:\\d\\d?:\\d\\d?",   "0:00:0",           (char*) 6      },
  { OK,  "\\d\\d?:\\d\\d?:\\d\\d?",   "0:0:00",           (char*) 5      },
  { OK,  "\\d\\d?:\\d\\d?:\\d\\d?",   "00:0:0",           (char*) 6      },
  { OK,  "\\d\\d?:\\d\\d?:\\d\\d?",   "00:00:0",          (char*) 7      },
  { OK,  "\\d\\d?:\\d\\d?:\\d\\d?",   "00:0:00",          (char*) 6      },
  { OK,  "\\d\\d?:\\d\\d?:\\d\\d?",   "0:00:00",          (char*) 6      },
  { OK,  "\\d\\d?:\\d\\d?:\\d\\d?",   "00:00:00",         (char*) 7      },
  { OK,  "[Hh]ello [Ww]orld\\s*[!]?", "Hello world !",    (char*) 12     },
  { OK,  "[Hh]ello [Ww]orld\\s*[!]?", "hello world !",    (char*) 12     },
  { OK,  "[Hh]ello [Ww]orld\\s*[!]?", "Hello World !",    (char*) 12     },
  { OK,  "[Hh]ello [Ww]orld\\s*[!]?", "Hello world!   ",  (char*) 11     },
  { OK,  "[Hh]ello [Ww]orld\\s*[!]?", "Hello world  !",   (char*) 13     },
  { OK,  "[Hh]ello [Ww]orld\\s*[!]?", "hello World    !", (char*) 15     },
  { NOK, "\\d\\d?:\\d\\d?:\\d\\d?",   "a:0",              (char*) 0      },
  { OK,  "[^\\w][^-1-4]",     ")T",          (char*) 2      },
  { OK,  "[^\\w][^-1-4]",     ")^",          (char*) 2      },
  { OK,  "[^\\w][^-1-4]",     "*)",          (char*) 2      },
  { OK,  "[^\\w][^-1-4]",     "!.",          (char*) 2      },
  { OK,  "[^\\w][^-1-4]",     " x",          (char*) 2      },
  { OK,  "[^\\w][^-1-4]",     "$b",          (char*) 2      },
  { OK,  ".?bar",                      "real_bar",        (char*) 4      },
  { NOK, ".?bar",                      "real_foo",        (char*) 0      },
  { NOK, "X?Y",                        "Z",               (char*) 0      },
  { OK, "[a-z]+\nbreak",              "blahblah\nbreak",  (char*) 14     },
  { OK, "[a-z\\s]+\nbreak",           "bla bla \nbreak",  (char*) 14     },
  { NOK, "a\\",                       "a\\",              (char*) 0      },
  { NOK, "\\",                        "\\",               (char*) 0      },
  { OK,  "\\\\",                      "\\",               (char*) 1      },
  { OK,  "0|1",                       "0",                (char*) 1      },
  { OK,  "[A-Z]|[0-9]",               "0",                (char*) 1      },
  { OK,  "\\w|\\s",                   "_ ",               (char*) 1      },
  // no multibyte support yet
  //{ OK,  "\\w+",                      "Çüéâ",             (char*) 4      },
};
*/

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

    printf("%d/%d tests succeeded.\n", ntests - nfailed, ntests);
    printf("\n");
    printf("\n");

    return nfailed; /* 0 if all tests passed */
}
