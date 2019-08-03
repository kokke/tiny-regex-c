/*
	Negative version of test_rand.c -- returns true if no match

	This program tries to match a given regular expression with text given as input to stdin.
	If the text is NOT a match for the pattern, the program returns 0.
	If the text does match the pattern, the program returns -2.

	This program is used in random testing to test a lot of random text and regex together.
	See ./scripts/regex_test_neg.py and the Makefile for this project for the gritty details.
*/

#include <errno.h>
#include <stdio.h>
#include "re.h"

int main(int argc, char* argv[])
{
	if (argc != 3) {
		printf("\nUsage: %s <PATTERN> <TEXT>\n", argv[0]);
		return -2;
	}
	errno = 0;
	re_smatch(argv[1], argv[2], NULL, 0);
	if (!errno)
		return -2;
	return 0;
}
