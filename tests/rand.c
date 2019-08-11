/*
	This program tests regular expressions against random strings.
*/

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#define __USE_POSIX2
#include <stdio.h>
#include <time.h>

#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>

#include "re.h"

const char* tests[] =
{
	"\\d+\\w?\\D\\d",
	"\\s+[a-zA-Z0-9?]*",
	"\\w*\\d?\\w\\?",
	"[^\\d]+\\\\?\\s",
	"[^\\w][^-1-4]",
	"[^\\w]",
	"[^1-4]",
	"[^-1-4]",
	"[^\\d]+\\s?[\\w]*",
	"a+b*[ac]*.+.*.[\\.].",
	"a?b[ac*]*.?[\\]+[?]?",
	"[1-5-]+[-1-2]-[-]",
	"[-1-3]-[-]+",
	"[1-5]+[-1-2]-[\\-]",
	"[-1-2]*1",
	"\\s?[a-fKL098]+-?",
	"[\\-]*\\-",
	"[\\\\]+",
	"[0-9a-fA-F]+",
	"[1379][2468][abcdef]",
	"[012345-9]?[0123-789]",
	"[012345-9]",
	"[0-56789]",
	"[abc-zABC-Z]",
	"[a\\d]?1234",
	".*123faerdig",
	".?\\w+jsj$",
	"[?to][+to][?ta][*ta]",
	"\\d+",
	"[a-z]+",
	"\\s+[a-zA-Z0-9?]*",
	"\\w",
	"\\d",
	"[\\d]",
	"[^\\d]",
	"[^-1-4]",
	"\\d+",
	"[a-z]+",
	"\\s+[a-zA-Z0-9?]*",
	"^\\w",
	"^\\d",
	"[\\d]",
	"^[^\\d]",
	"[^\\w]+",
	"^[\\w]+",
	"^[^0-9]",
	"[a-z].[A-Z]",
	"[-1-3]-[-]+",
	"[1-5]+[-1-2]-[\\-]",
	"[-0-9]+",
	"[\\-]+",
	"[\\\\]+",
	"[0-9a-fA-F]+",
	"[1379][2468][abcdef]",
	"[012345-9]",
	"[0-56789]",
	".*123faerdig"
};

const char* genpass(const char* pattern, pcre2_code* regex);
const char* genfail(pcre2_code* regex);

int main()
{
	srand(time(NULL));

	const size_t ntests = sizeof(tests) / sizeof(const char*);
	const size_t npass = 100;
	const size_t nfail = 100;

	for (size_t i = 0; i < ntests; ++i) {
		printf("[%zu/%zu]: testing regex '%s' with %zu to pass and %zu to fail.\n", i+1, ntests, tests[i], npass, nfail);

		Regex regex;
		errno = 0;
		re_compile(&regex, tests[i]);
		if (errno) {
			fprintf(stderr, "[%zu/%zu]: pattern '%s' failed to compile.\n", i+1, ntests, tests[i]);
			continue;
		}

		int dummyerrorcode;
		PCRE2_SIZE dummyerroroffset;
		pcre2_code* pcreregex = pcre2_compile((PCRE2_SPTR8)tests[i], PCRE2_ZERO_TERMINATED, 0, &dummyerrorcode, &dummyerroroffset, NULL);
		if (!pcreregex) {
			fprintf(stderr, "[%zu/%zu]: pattern '%s' failed to compile with PCRE.\n", i+1, ntests, tests[i]);
			continue;
		}

		for (size_t j = 0; j < npass; ++j) {
			const char* str = genpass(tests[i], pcreregex);
			if (!str) {
				fprintf(stderr, "[%zu/%zu]: pattern '%s' failed to generate passing string.\n", i+1, ntests, tests[i]);
				continue;
			}
			errno = 0;
			re_match(regex, str, NULL);
			if (errno) {
				re_print(regex);
				fprintf(stderr, "[%zu/%zu]: pattern '%s' didn't match '%s' as expected.\n", i+1, ntests, tests[i], str);
				continue;
			}
		}
		for (size_t j = 0; j < nfail; ++j) {
			const char* str = genfail(pcreregex);
			errno = 0;
			re_match(regex, str, NULL);
			if (!errno) {
				re_print(regex);
				fprintf(stderr, "[%zu/%zu]: pattern '%s' matched '%s' unexpectedly.\n", i+1, ntests, tests[i], str);
				continue;
			}
		}

		pcre2_code_free(pcreregex);
	}
	return 0;
}

#define MAXCMDLEN 100
#define MAXLEN 50

const char* genpass(const char* pattern, pcre2_code* regex)
{
	static char pass[MAXLEN];
	char cmd[MAXCMDLEN];
	snprintf(cmd, MAXCMDLEN, "python scripts/exrex.py -r '%s'", pattern);
	pcre2_match_data* dummymd = pcre2_match_data_create(1, NULL);
	if (!dummymd)
		return NULL;
	do {
		FILE* exrex = popen(cmd, "r");
		if (!exrex) {
			pcre2_match_data_free(dummymd);
			return NULL;
		}
		
		if (!fgets(pass, MAXLEN, exrex) && ferror(exrex)) {
			pcre2_match_data_free(dummymd);
			pclose(exrex);
			return NULL;
		}
		/* remove the trailing newline */
		pass[strlen(pass)-1] = '\0';

		pclose(exrex);
	} while (pcre2_match(regex, (PCRE2_SPTR8)pass, PCRE2_ZERO_TERMINATED, 0, 0, dummymd, NULL) == PCRE2_ERROR_NOMATCH);

	pcre2_match_data_free(dummymd);

	return pass;
}

const char* genfail(pcre2_code* regex)
{
	static char fail[MAXLEN];
	pcre2_match_data* dummymd = pcre2_match_data_create(1, NULL);
	if (!dummymd)
		return NULL;
	do {
		const size_t len = rand()%(MAXLEN-1) + 1;
		for (size_t i = 0; i < len; ++i) {
			fail[i] = (rand()%('\x7f'-' ')) + ' ';
		}
		fail[len] = '\0';
		errno = 0;
	} while (pcre2_match(regex, (PCRE2_SPTR8)fail, PCRE2_ZERO_TERMINATED, 0, 0, dummymd, NULL) != PCRE2_ERROR_NOMATCH);

	pcre2_match_data_free(dummymd);
	return fail;
}
