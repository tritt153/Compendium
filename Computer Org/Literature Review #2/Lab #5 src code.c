#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int PC = 0;

void printIFormat(int opcode, int rs, int rd, int immediate, int isBranch);

void printRFormat(int opcode, int rd, int rs, int rt, int shamt, int funct);

void split(char* line, char** parts);

void translate(char **parts);

int getRegisterNumber(char*);

char* toBinary(int, int);

int main(void) {
  FILE* input = fopen("mips.txt", "r");
  
  char line[50];
  char* parts[4];

  while(fgets(line, 50, input)) {
    printf("0x%08x: ", PC);
    printf("%s", line);

    split(line, parts);
    translate(parts);
    PC +=4;
  }
}
void split(char* line, char** parts) {
  char* temp = strtok(line, " ");
  int i = 0;
  while(temp != NULL) {
    parts[i] = temp;
    temp = strtok(NULL, " ");
    i++;
  }
}
void translate(char **parts) {
  int opcode, rd, rs, rt, shamt, funct, immediate;
  int isBranch = 0;

  //I-format
  if(strcmp(parts[0], "addi") == 0) {
    opcode = 8;
    rd = getRegisterNumber(&parts[1][1]) + 
         atoi(&parts[1][2]);
    rs = getRegisterNumber(&parts[2][1]) +
         atoi(&parts[2][2]);
    immediate = atoi(parts[3]);

    printIFormat(opcode, rs, rd, immediate, isBranch);
  }
  else if(strcmp(parts[0], "andi") == 0) {
    opcode = 12;
    rs = getRegisterNumber(&parts[2][1]) + 
         atoi(&parts[2][2]);
    rd = getRegisterNumber(&parts[1][1]) + 
         atoi(&parts[1][2]);
    immediate = atoi(parts[3]);

    printIFormat(opcode, rs, rd, immediate, isBranch);
  }
  else if(strcmp(parts[0], "bne") == 0) {
    isBranch = 1;
    opcode = 5;
    rs = getRegisterNumber(&parts[1][1]) + 
         atoi(&parts[1][2]);
    rd = getRegisterNumber(&parts[2][1]) + 
         atoi(&parts[2][2]);
    immediate = atoi(parts[3]);

    printIFormat(opcode, rs, rd, immediate, isBranch);
  }

  //R-Format
  else if(strcmp(parts[0], "add") == 0) {
    opcode = 0;
    rs = getRegisterNumber(&parts[1][1]) + 
         atoi(&parts[1][2]);
    rt = getRegisterNumber(&parts[2][1]) + 
         atoi(&parts[2][2]);
    rd = getRegisterNumber(&parts[3][1]) + 
         atoi(&parts[3][2]);
    shamt = 0;
    funct = 32;
  
    printRFormat(opcode, rs, rt, rd, shamt, funct);
  }
  else if(strcmp(parts[0], "sub") == 0) {
    opcode = 0;
    rs = getRegisterNumber(&parts[1][1]) + 
         atoi(&parts[1][2]);
    rt = getRegisterNumber(&parts[2][1]) + 
         atoi(&parts[2][2]);
    rd = getRegisterNumber(&parts[3][1]) + 
         atoi(&parts[3][2]);
    shamt = 0;
    funct = 34;
  
    printRFormat(opcode, rs, rt, rd, shamt, funct);
  }
  else if(strcmp(parts[0], "sll") == 0) {
    opcode = 0;
    rs = getRegisterNumber(&parts[1][1]) +  
         atoi(&parts[1][2]);
    rt = 0;
    rd = getRegisterNumber(&parts[2][1]) + 
         atoi(&parts[2][2]);
    shamt = atoi(parts[3]);
    funct = 0;
  
    printRFormat(opcode, rs, rt, rd, shamt, funct);
  }
}
void printIFormat(int opcode, int rs, int rd, int immediate, int isBranch) { 
  char* opcodeBinary = malloc(6 * sizeof(char));
  opcodeBinary = toBinary(opcode, 6);
  printf("\t(I) %s ", opcodeBinary);

  char* rsBinary = malloc(5 * sizeof(char));
  rsBinary = toBinary(rs, 5);
  printf("%s ", rsBinary);

  char* rdBinary = malloc(5 * sizeof(char));
  rdBinary = toBinary(rd, 5);
  printf("%s ", rdBinary);

  char* immediateBinary = malloc(16 * sizeof(char));
  immediateBinary = toBinary(immediate, 16);

  if(isBranch == 1) {
    int newPC = PC + 4 + immediate;
    printf("%s ", immediateBinary);
    printf("(Branch Address: 0x%08x)\n", newPC);
  } else {
     printf("%s\n", immediateBinary);
  }
}
void printRFormat(int opcode, int rd, int rs, int rt, int shamt, int funct) {
  char* opcodeBinary = malloc(6 * sizeof(char));
  opcodeBinary = toBinary(opcode, 6);
  printf("\t(R) %s ", opcodeBinary);

  char* rsBinary = malloc(5 * sizeof(char));
  rsBinary = toBinary(rs, 5);
  printf("%s ", rsBinary);

  char* rtBinary = malloc(5 * sizeof(char));
  rtBinary = toBinary(rt, 5);
  printf("%s ", rtBinary);

  char* rdBinary = malloc(5 * sizeof(char));
  rdBinary = toBinary(rd, 5);
  printf("%s ", rdBinary);

  char* shamtBinary = malloc(5 * sizeof(char));
  shamtBinary = toBinary(shamt, 5);
  printf("%s ", shamtBinary);
 
 char* functBinary = malloc(6 * sizeof(char));
  functBinary = toBinary(funct, 6);
  printf("%s\n", functBinary);
}
char* toBinary(int input, int size) {
  char* target = malloc(size * sizeof(char));
  for(int i = size-1; i >= 0; i--) {
      target[i] = (input % 2) + '0';
      input /= 2;
    }
  return target;
}
int getRegisterNumber(char* input) {
  int res; 
  switch(input[0]) {
    case 't':
      res = 8;
      break;
    case 'a':
      res = 4;
      break;
    case 'v':
      res = 2;
      break;
    case 's':
      res = 16;
      break;
  }
  return res;
}