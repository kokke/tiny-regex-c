/*
 * tiny-regex-c
 * This example implements a very simple form of grep(1) using tiny-regex-c.
 * The code should be fairly self-explanatory.
 */

#include <errno.h>
#include <stdio.h>

#include "re.h"

int main(int argc, char* argv[])
{
	if (argc != 2) {
		fprintf(stderr, "Usage: %s <PATTERN>\n", argv[0]);
		return 2;
	}
	/* the regex struct */
	Regex pattern;
	/* set errno to 0 to check for errors reliably, as the functions aren't guaranteed set errno to 0 on success */
	errno = 0;
	/* compile the regex struct from the command line argument */
	re_compile(&pattern, argv[1]);
	/* check for errors in the compilation process */
	if (errno != 0) {
		perror("Error while compiling regex");
		return 2;
	}

	/* loop for each line of input until there is no more */
	char buffer[200];
	while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
		/* set errno to 0 to check for errors reliably */
		errno = 0;
		/* match input with pattern, not saving the length and with no modifiers */
		re_rmatch(pattern, buffer, NULL, 0);
		if (!errno)
			/* rmatch succeeded, so print out the line */
			printf("%s", buffer);
	}
	return 0;
}
