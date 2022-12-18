#include <stdio.h>
#include <stdlib.h>
#include "re.h"

int main() {
 
   const char *text = "table football";
   const char *pattern = "l$";   
   int index,len;

   index = re_match(pattern, text, &len);

   if (index==13 && len==1) {
      return 0; 
   } else {
      printf("ERROR! index=%d len=%d \n",index,len);
      return -1;
   }

}	
