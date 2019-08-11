/*
 * This program prints out a regular expression.
 */

#include <stdio.h>
#include "re.h"


int main(int argc, char* argv[])
{
	if (argc != 2) {
		printf("\nUsage: %s <PATTERN> \n", argv[0]);
		return -1;
	}
	Regex pattern;
	re_compile(&pattern, argv[1]);
	re_print(pattern);
	return 0;
}

