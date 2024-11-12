#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "re.h"

static void default_function(int index, int* match_length)
{
	printf("match at index %i, with length of %i\n", index, *match_length);
}

void realize_a_test(char* text, char* pattern)
{
	printf("text is \"%s\" and pattern \"%s\"\n", text, pattern);
	int occurrence;
	re_t my_regex = re_compile(pattern);
	
	/* MATCH TIMES */
	if ( (occurrence = match_times(my_regex,text,default_function)) )
		printf("there is %i occurence\n", occurrence);
	else
		puts("0 match");

	/* MATCH FULL */
	if (match_full(my_regex,text))
		puts("pattern match perfectly\n");
	else
		puts("pattern doesn't match perfectly\n");
}

int main(void)
{
	realize_a_test("pseudo404",".");
	realize_a_test("pseudo404",".+");
	realize_a_test("554864","5");
	realize_a_test("554864","[0-9][0-9]");
	realize_a_test("lettre","[0-9]"); // no occurence and obviously no match
	return 0;
}