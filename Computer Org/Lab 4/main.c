#include <stdio.h>
#include <stdlib.h>

int main(void) {
  int input;
  printf("Enter Positiive Int: ");
  scanf("%d", &input);

  if(input < 0) { 
    printf("Please enter a positive number");
    exit(0);
  }

  int result = 0;
  while(input != 0) { 
    result += input % 2;
    input /= 2;
  }  

  printf("\n%d", result);
}