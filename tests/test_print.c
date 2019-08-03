/*
		This program prints out a verbose explanation of a given regular expression.
*/

#include <stdio.h>
#include "re.h"


int main(int argc, char* argv[])
{
	if (argc != 2) {
		printf("\nUsage: %s <PATTERN> \n", argv[0]);
		return -2;
	}
	Regex pattern;
	re_compile(&pattern, argv[1]);
	re_print(pattern);
	return 0;
}

