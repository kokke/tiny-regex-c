/*
 * This programs tests various fixed regex-patterns.
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
	char* text;
} Test;

Test testvector[] =
{
	{ false , "a"                        , ""                       },
	{ true  , "a*"                       , ""                       },
	{ false , "[^s][^b]"                 , "a"                      },
	{ false , "[^\\d]+\\s"               , "e"                      },
	{ true  , "\\d"                      , "5"                      },
	{ false , "\\d+"                     , "y"                      },
	{ true  , "\\w+"                     , "hej"                    },
	{ true  , "\\s"                      , "\t \n"                  },
	{ false , "\\S"                      , "\t \n"                  },
	{ true  , "[\\s]"                    , "\t \n"                  },
	{ false , "[\\S]"                    , "\t \n"                  },
	{ false , "\\D"                      , "5"                      },
	{ false , "\\W+"                     , "hej"                    },
	{ true  , "[0-9]+"                   , "12345"                  },
	{ true  , "\\D"                      , "hej"                    },
	{ false , "\\d"                      , "hej"                    },
	{ true  , "[^\\w]"                   , "\\"                     },
	{ true  , "[\\W]"                    , "\\"                     },
	{ false , "[\\w]"                    , "\\"                     },
	{ true  , "[^\\d]"                   , "d"                      },
	{ false , "[\\d]"                    , "d"                      },
	{ false , "[^\\D]"                   , "d"                      },
	{ true  , "[\\D]"                    , "d"                      },
	{ true  , "a+a"                      , "aaa"                    },
	{ true  , "^.*\\\\.*$"               , "c:\\Tools"              },
	{ true  , "^[\\+-]*[\\d]+$"          , "+27"                    },
	{ true  , "[abc]"                    , "1c2"                    },
	{ false , "[abc]"                    , "1C2"                    },
	{ true  , "[1-5]+"                   , "0123456789"             },
	{ true  , "[.2]"                     , "1C2"                    },
	{ true  , "a*$"                      , "Xaa"                    },
	{ true  , "[a-h]+"                   , "abcdefghxxx"            },
	{ false , "[a-h]+"                   , "ABCDEFGH"               },
	{ true  , "[A-H]+"                   , "ABCDEFGH"               },
	{ false , "[A-H]+"                   , "abcdefgh"               },
	{ true  , "[^\\s]+"                  , "abc def"                },
	{ true  , "[^fc]+"                   , "abc def"                },
	{ true  , "[^d\\sf]+"                , "abc def"                },
	{ true  , "\n"                       , "abc\ndef"               },
	{ true  , "b.\\s*\n"                 , "aa\r\nbb\r\ncc\r\n\r\n" },
	{ true  , ".*c"                      , "abcabc"                 },
	{ true  , ".+c"                      , "abcabc"                 },
	{ true  , "[b-z].*"                  , "ab"                     },
	{ true  , "b[k-z]*"                  , "ab"                     },
	{ false , "[0-9]"                    , "  - "                   },
	{ true  , "[^0-9]"                   , "  - "                   },
	{ true  , "0|"                       , "0|"                     },
	{ false , "\\d\\d:\\d\\d:\\d\\d"     , "0s:00:00"               },
	{ false , "\\d\\d:\\d\\d:\\d\\d"     , "000:00"                 },
	{ false , "\\d\\d:\\d\\d:\\d\\d"     , "00:0000"                },
	{ false , "\\d\\d:\\d\\d:\\d\\d"     , "100:0:00"               },
	{ false , "\\d\\d:\\d\\d:\\d\\d"     , "00:100:00"              },
	{ false , "\\d\\d:\\d\\d:\\d\\d"     , "0:00:100"               },
	{ true  , "\\d\\d?:\\d\\d?:\\d\\d?"  , "0:0:0"                  },
	{ true  , "\\d\\d?:\\d\\d?:\\d\\d?"  , "0:00:0"                 },
	{ true  , "\\d\\d?:\\d\\d?:\\d\\d?"  , "0:0:00"                 },
	{ true  , "\\d\\d?:\\d\\d?:\\d\\d?"  , "00:0:0"                 },
	{ true  , "\\d\\d?:\\d\\d?:\\d\\d?"  , "00:00:0"                },
	{ true  , "\\d\\d?:\\d\\d?:\\d\\d?"  , "00:0:00"                },
	{ true  , "\\d\\d?:\\d\\d?:\\d\\d?"  , "0:00:00"                },
	{ true  , "\\d\\d?:\\d\\d?:\\d\\d?"  , "00:00:00"               },
	{ true  , "[Hh]ello [Ww]orld\\s*[!]?", "Hello world !"          },
	{ true  , "[Hh]ello [Ww]orld\\s*[!]?", "hello world !"          },
	{ true  , "[Hh]ello [Ww]orld\\s*[!]?", "Hello World !"          },
	{ true  , "[Hh]ello [Ww]orld\\s*[!]?", "Hello world!   "        },
	{ true  , "[Hh]ello [Ww]orld\\s*[!]?", "Hello world    !"       },
	{ true  , "[Hh]ello [Ww]orld\\s*[!]?", "hello World      !"     },
	{ false , "\\d\\d?:\\d\\d?:\\d\\d?"  , "a:0"                    },
	{ true  , "[^\\w][^-1-4]"            , ")T"                     },
	{ true  , "[^\\w][^-1-4]"            , ")^"                     },
	{ true  , "[^\\w][^-1-4]"            , "*)"                     },
	{ true  , "[^\\w][^-1-4]"            , "!."                     },
	{ true  , "[^\\w][^-1-4]"            , " x"                     },
	{ true  , "[^\\w][^-1-4]"            , "$b"                     },
	{ true  , ".?bar"                    , "real_bar"               },
	{ false , ".?bar"                    , "real_foo"               },
	{ false , "X?Y"                      , "Z"                      },
	{ true  , "\\d+\\w?12"               , "959312"                 },
	{ true  , "\\d+5"                    , "12345"                  },
	{ false , "\\d++5"                   , "12345"                  },
	{ false , "abcd"                     , "aBcD"                   },
	{ true  , "(?i:abcd)"                , "aBcD"                   },
	{ false , "..."                      , "\n \n"                  },
	{ true  , "(?s:...)"                 , "\n \n"                  },
	{ false , "(?s:(?-s:.))"             , "\n"                     },
	{ true  , "(?is:A.)"                 , "a\n"                    },
	{ false , "(?is:(?-is:.g.))"         , "\nG\n"                  },
	{ true  , "(?is:(?-is:.g.))"         , "\ng\n"                  },
	{ false , "abc\\bdef"                , "abcdef"                 },
	{ true  , "abc\\Bdef"                , "abcdef"                 },
	{ true  , "\\Bing\\b"                , "joining."               },
	{ false , "\\Bing\\b"                , " ing "                  },
	{ false,  "\\Bing\\b"                , "ing"                    },
	{ false , "\\Bing\\b"                , "bingg"                  },
	{ true  , "abc\\Rdef"                , "abc\r\ndef"             },
	{ true  , "abc\\Rdef"                , "abc\ndef"               },
	{ false , "abc\n\\Rdef"              , "abc\ndef"               },
	{ true  , "abc\r\\Rdef"              , "abc\r\ndef"             },
	{ true  , "^(a+)a$"                  , "aaa"                    },
	{ true  , "^a(a*)a$"                 , "aa"                     },
	{ true  , "^(a)+a$"                  , "aaa"                    },
	{ true  , "^(Hello){3}(World){1,2}$" , "HelloHelloHelloWorld"   },
	{ true  , "^(is:[ab])+?bc$"          , "aAaAaaAAaaAAAAbAaaAbc"  },
	{ true  , "(?=.*ghi)abc"             , "abcdefghi"              },
	{ true  , "(?s=.*END)BEGIN"          , "BEGIN..content..\nEND"  },
	{ false , "(?s=.*END)BEGIN"          , "BEGIN..content..\n"     },
	{ false , "(?s!.*END)BEGIN"          , "BEGIN..content..\nEND"  },
	{ true  , "(?s!.*END)BEGIN"          , "BEGIN..content..\n"     },
	{ false , "(b*){1}+b"                , "bbbbb"                  },
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
		re_match(pattern, testvector[i].text, NULL);

		if (testvector[i].shouldsucceed && errno) {
			/* failed where it should have succeeded */
			re_print(pattern);
			printf("\n");
			fprintf(stderr, "[%zu/%zu]: pattern '%s' didn't match '%s' as expected.\n", i+1, ntests, testvector[i].pattern, testvector[i].text);
			++nfailed;
		} else if (!testvector[i].shouldsucceed && !errno) {
			/* succeeded where it should have failed */
			fprintf(stderr, "[%zu/%zu]: pattern '%s' matched '%s' unexpectedly.\n", i+1, ntests, testvector[i].pattern, testvector[i].text);
			++nfailed;
		} else {
			/*
			fprintf(stderr, "[%zu/%zu]: pattern '%s' worked as expected.\n", i+1, ntests, testvector[i].pattern);
			*/
		}
	}

	printf("%zu/%zu tests succeeded.\n", ntests - nfailed, ntests);

	return 0;
}
