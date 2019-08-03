/*
 * Testing various regex-patterns
 */

#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include "re.h"

typedef struct Test
{
	bool shouldsucceed;
	char* pattern;
	uint_fast8_t modifiers;
	char* text;
} Test;

Test testvector[] =
{
	{ true  , "\\d"                      ,           0, "5"                      },
	{ false , "\\d+"                     ,           0, "y"                      },
	{ true  , "\\w+"                     ,           0, "hej"                    },
	{ true  , "\\s"                      ,           0, "\t \n"                  },
	{ false , "\\S"                      ,           0, "\t \n"                  },
	{ true  , "[\\s]"                    ,           0, "\t \n"                  },
	{ false , "[\\S]"                    ,           0, "\t \n"                  },
	{ false , "\\D"                      ,           0, "5"                      },
	{ false , "\\W+"                     ,           0, "hej"                    },
	{ true  , "[0-9]+"                   ,           0, "12345"                  },
	{ true  , "\\D"                      ,           0, "hej"                    },
	{ false , "\\d"                      ,           0, "hej"                    },
	{ true  , "[^\\w]"                   ,           0, "\\"                     },
	{ true  , "[\\W]"                    ,           0, "\\"                     },
	{ false , "[\\w]"                    ,           0, "\\"                     },
	{ true  , "[^\\d]"                   ,           0, "d"                      },
	{ false , "[\\d]"                    ,           0, "d"                      },
	{ false , "[^\\D]"                   ,           0, "d"                      },
	{ true  , "[\\D]"                    ,           0, "d"                      },
	{ true  , "^.*\\\\.*$"               ,           0, "c:\\Tools"              },
	{ true  , "^[\\+-]*[\\d]+$"          ,           0, "+27"                    },
	{ true  , "[abc]"                    ,           0, "1c2"                    },
	{ false , "[abc]"                    ,           0, "1C2"                    },
	{ true  , "[1-5]+"                   ,           0, "0123456789"             },
	{ true  , "[.2]"                     ,           0, "1C2"                    },
	{ true  , "a*$"                      ,           0, "Xaa"                    },
	{ true  , "[a-h]+"                   ,           0, "abcdefghxxx"            },
	{ false , "[a-h]+"                   ,           0, "ABCDEFGH"               },
	{ true  , "[A-H]+"                   ,           0, "ABCDEFGH"               },
	{ false , "[A-H]+"                   ,           0, "abcdefgh"               },
	{ true  , "[^\\s]+"                  ,           0, "abc def"                },
	{ true  , "[^fc]+"                   ,           0, "abc def"                },
	{ true  , "[^d\\sf]+"                ,           0, "abc def"                },
	{ true  , "\n"                       ,           0, "abc\ndef"               },
	{ true  , "b.\\s*\n"                 ,           0, "aa\r\nbb\r\ncc\r\n\r\n" },
	{ true  , ".*c"                      ,           0, "abcabc"                 },
	{ true  , ".+c"                      ,           0, "abcabc"                 },
	{ true  , "[b-z].*"                  ,           0, "ab"                     },
	{ true  , "b[k-z]*"                  ,           0, "ab"                     },
	{ false , "[0-9]"                    ,           0, "  - "                   },
	{ true  , "[^0-9]"                   ,           0, "  - "                   },
	{ true  , "0|"                       ,           0, "0|"                     },
	{ false , "\\d\\d:\\d\\d:\\d\\d"     ,           0, "0s:00:00"               },
	{ false , "\\d\\d:\\d\\d:\\d\\d"     ,           0, "000:00"                 },
	{ false , "\\d\\d:\\d\\d:\\d\\d"     ,           0, "00:0000"                },
	{ false , "\\d\\d:\\d\\d:\\d\\d"     ,           0, "100:0:00"               },
	{ false , "\\d\\d:\\d\\d:\\d\\d"     ,           0, "00:100:00"              },
	{ false , "\\d\\d:\\d\\d:\\d\\d"     ,           0, "0:00:100"               },
	{ true  , "\\d\\d?:\\d\\d?:\\d\\d?"  ,           0, "0:0:0"                  },
	{ true  , "\\d\\d?:\\d\\d?:\\d\\d?"  ,           0, "0:00:0"                 },
	{ true  , "\\d\\d?:\\d\\d?:\\d\\d?"  ,           0, "0:0:00"                 },
	{ true  , "\\d\\d?:\\d\\d?:\\d\\d?"  ,           0, "00:0:0"                 },
	{ true  , "\\d\\d?:\\d\\d?:\\d\\d?"  ,           0, "00:00:0"                },
	{ true  , "\\d\\d?:\\d\\d?:\\d\\d?"  ,           0, "00:0:00"                },
	{ true  , "\\d\\d?:\\d\\d?:\\d\\d?"  ,           0, "0:00:00"                },
	{ true  , "\\d\\d?:\\d\\d?:\\d\\d?"  ,           0, "00:00:00"               },
	{ true  , "[Hh]ello [Ww]orld\\s*[!]?",           0, "Hello world !"          },
	{ true  , "[Hh]ello [Ww]orld\\s*[!]?",           0, "hello world !"          },
	{ true  , "[Hh]ello [Ww]orld\\s*[!]?",           0, "Hello World !"          },
	{ true  , "[Hh]ello [Ww]orld\\s*[!]?",           0, "Hello world!   "        },
	{ true  , "[Hh]ello [Ww]orld\\s*[!]?",           0, "Hello world    !"       },
	{ true  , "[Hh]ello [Ww]orld\\s*[!]?",           0, "hello World      !"     },
	{ false , "\\d\\d?:\\d\\d?:\\d\\d?"  ,           0, "a:0"                    },
	{ true  , "[^\\w][^-1-4]"            ,           0, ")T"                     },
	{ true  , "[^\\w][^-1-4]"            ,           0, ")^"                     },
	{ true  , "[^\\w][^-1-4]"            ,           0, "*)"                     },
	{ true  , "[^\\w][^-1-4]"            ,           0, "!."                     },
	{ true  , "[^\\w][^-1-4]"            ,           0, " x"                     },
	{ true  , "[^\\w][^-1-4]"            ,           0, "$b"                     },
	{ true  , ".?bar"                    ,           0, "real_bar"               },
	{ false , ".?bar"                    ,           0, "real_foo"               },
	{ false , "X?Y"                      ,           0, "Z"                      },
	{ true  , "\\d+\\w?12"               ,           0, "959312"                 },
	{ true  , "\\d+5"                    ,           0, "12345"                  },
	{ false , "\\d++5"                   ,           0, "12345"                  },
	{ false , "abcd"                     ,           0, "aBcD"                   },
	{ true  , "abcd"                     ,       MOD_I, "aBcD"                   },
	{ false , "..."                      ,           0, "\n \n"                  },
	{ true  , "..."                      ,       MOD_S, "\n \n"                  }
};


int main()
{
	const size_t ntests = sizeof(testvector) / sizeof(Test);
	size_t nfailed = 0;

	for (size_t i = 0; i < ntests; ++i) {
		errno = 0;
		Regex pattern;
		re_compile(&pattern, testvector[i].pattern);
		if (errno) {
			fprintf(stderr, "[%zu/%zu]: pattern '%s' failed to compile.\n", i+1, ntests, testvector[i].pattern);
			++nfailed;
			continue;
		}
		re_rmatch(pattern, testvector[i].text, NULL, testvector[i].modifiers);

		if (testvector[i].shouldsucceed && errno) {
			/* failed where it should have succeeded */
			re_print(pattern);
			fprintf(stderr, "[%zu/%zu]: pattern '%s' with modifiers '%"PRIuFAST8"' didn't match '%s' as expected.\n", i+1, ntests, testvector[i].pattern, testvector[i].modifiers, testvector[i].text);
			++nfailed;
		} else if (!testvector[i].shouldsucceed && !errno) {
			/* succeeded where it should have failed */
			fprintf(stderr, "[%zu/%zu]: pattern '%s' with modifiers '%"PRIuFAST8"' matched '%s' unexpectedly.\n", i+1, ntests, testvector[i].pattern, testvector[i].modifiers, testvector[i].text);
			++nfailed;
		} else {
			/* fprintf(stderr, "[%zu/%zu]: pattern '%s' worked as expected.\n", i+1, ntests, testvector[i].pattern); */
		}
	}

	printf("%zu/%zu tests succeeded.\n", ntests - nfailed, ntests);
	printf("\n");
	printf("\n");
	printf("\n");

	return 0;
}
