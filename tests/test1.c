/*
 * Testing various regex-patterns
 */
#define _POSIX_C_SOURCE 199309L
#if __STDC_VERSION__ >= 199901L
#define _XOPEN_SOURCE 600
#else
#define _XOPEN_SOURCE 500
#endif /* __STDC_VERSION__ */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define BUILD_WITH_ERRORMSG
#define RE_BUILDWITH_DEBUG

#include "re.h"


#define OK    ((char*) 1)
#define NOK   ((char*) 0)


char* test_vector[][3] =
{
  { OK,  "\\d",              "5"           },
  { OK,  "\\w+",             "hej"         },
  { OK,  "\\s",              "\t \n"       },
  { NOK, "\\S",              "\t \n"       },
  { OK,  "[\\s]",            "\t \n"       },
  { NOK, "[\\S]",            "\t \n"       },
  { NOK, "\\D",              "5"           },
  { NOK, "\\W+",             "hej"         },
  { OK,  "[0-9]+",           "12345"       },
  { OK,  "\\D",              "hej"         },
  { NOK, "\\d",              "hej"         },
  { OK,  "[^\\w]",           "\\"          },
  { OK,  "[\\W]",            "\\"          },
  { NOK, "[\\w]",            "\\"          },
  { OK,  "[^\\d]",           "d"           },
  { NOK, "[\\d]",            "d"           },
  { NOK, "[^\\D]",           "d"           },
  { OK,  "[\\D]",            "d"           },
  { OK,  "^.*\\\\.*$",       "c:\\Tools"   },
  { OK,  "^[\\+-]*[\\d]+$",  "+27"         },
  { OK,  "[abc]",            "1c2"         },
  { NOK, "[abc]",            "1C2"         },
  { OK,  "[1-5]+",           "0123456789"  },
  { OK,  "[.2]",             "1C2"         },
  { OK,  "a*$",              "Xaa"         },
  { OK,  "a*$",              "Xaa"         },
  { OK,  "[a-h]+",           "abcdefghxxx" },
  { NOK, "[a-h]+",           "ABCDEFGH"    },
  { OK,  "[A-H]+",           "ABCDEFGH"    },
  { NOK, "[A-H]+",           "abcdefgh"    },
  { OK,  "[^\\s]+",          "abc def"     },
  { OK,  "[^fc]+",           "abc def"     },
  { OK,  "[^d\\sf]+",        "abc def"     },
  { OK,  "\n",               "abc\ndef"    },
  { OK,  "b.\\s*\n",         "aa\r\nbb\r\ncc\r\n\r\n" },
  { OK,  ".*c",              "abcabc"      },
  { OK,  ".+c",              "abcabc"      },
  { OK,  "[b-z].*",          "ab"          },
  { OK,  "b[k-z]*",          "ab"          },
  { NOK, "[0-9]",            "  - "        },
  { OK,  "[^0-9]",           "  - "        },
  { OK,  "[Hh]ello [Ww]orld\\s*[!]?", "Hello world !" },
  { OK,  "[Hh]ello [Ww]orld\\s*[!]?", "hello world !" },
  { OK,  "[Hh]ello [Ww]orld\\s*[!]?", "Hello World !" },
  { OK,  "[Hh]ello [Ww]orld\\s*[!]?", "Hello world!   " },
  { OK,  "[Hh]ello [Ww]orld\\s*[!]?", "Hello world  !" },
  { OK,  "[Hh]ello [Ww]orld\\s*[!]?", "hello World    !" },
/*
  { OK,  "[^\\w][^-1-4]",     ")T"          },
  { OK,  "[^\\w][^-1-4]",     ")^"          },
  { OK,  "[^\\w][^-1-4]",     "*)"          },
  { OK,  "[^\\w][^-1-4]",     "!."          },
  { OK,  "[^\\w][^-1-4]",     " x"          },
  { OK,  "[^\\w][^-1-4]",     "$b"          },
*/
};


//void re_print(re_t, unsigne);

int main()
{
    char* text;
    char* pattern;
    int should_fail;
    size_t ntests = sizeof(test_vector) / sizeof(*test_vector);
    size_t nfailed = 0;
    size_t i;

    for (i = 0; i < ntests; ++i)
    {
        pattern = test_vector[i][1];
        text = test_vector[i][2];
        should_fail = (test_vector[i][0] == NOK);
        
        int m = 0;
        
        struct timespec start, stop;
        struct timespec startp, stopp;
        clock_gettime(CLOCK_MONOTONIC, &start);
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &startp);
        
        m = re_match(pattern, text);
        
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &stopp);
        clock_gettime(CLOCK_MONOTONIC, &stop);

        double result = (stop.tv_sec - start.tv_sec) * 
                    1e6 + (stop.tv_nsec - start.tv_nsec) / 1e3;    // in microseconds
        double resultp = (stopp.tv_sec - startp.tv_sec) * 
                    1e6 + (stopp.tv_nsec - startp.tv_nsec) / 1e3;    // in microseconds
        
        printf("test %lu took %lf uS / %lf uS MONOTONIC/PROCESS\r\n", i, result, resultp);
        
        if (should_fail)
        {
            if (m != (-1))
            {
                printf("\n");
                unsigned int ln = 0;
              

                re_t res = re_compile(pattern, &ln);
                re_print(res, ln);
                
                fprintf(stderr, "[%lu/%lu]: pattern '%s' matched '%s' unexpectedly. \n", (i+1), ntests, pattern, text);
                nfailed += 1;
            }
        }
        else
        {
            if (m == (-1))
            {
                printf("\n");
                unsigned int ln = 0;
                
                re_t res = re_compile(pattern, &ln);
                re_print(res, ln);
                
                fprintf(stderr, "[%lu/%lu]: pattern '%s' didn't match '%s' as expected. \n", (i+1), ntests, pattern, text);
                nfailed += 1;
            }
        }
    }

    // printf("\n");
    printf("%lu/%lu tests succeeded.\n", ntests - nfailed, ntests);
    printf("\n");
    printf("\n");
    printf("\n");

    return 0;
}

