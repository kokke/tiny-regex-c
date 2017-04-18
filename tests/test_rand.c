
#include <stdio.h>
#include "re.h"


int main(int argc, char** argv)
{
  if (argc == 3)
  {
    int m = re_match(argv[1], argv[2]);
    if (m != -1) 
      return 0;
  }
  else
  {
    printf("\nUsage: %s <PATTERN> <TEXT> \n", argv[0]);
  }   
  return -2; 
}

