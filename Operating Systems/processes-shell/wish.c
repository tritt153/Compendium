#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX_WORDS 10
#define MAX_WORD_LENGTH 50
#define ERROR_MSG "An error has occurred\n"

void executeCommand(char args[MAX_WORDS][MAX_WORD_LENGTH], char path[MAX_WORDS][MAX_WORD_LENGTH], int isRedirect);
void splitString(char args[MAX_WORDS][MAX_WORD_LENGTH], char *buffer);
int isEmpty(char *buffer);
void getArguments(char args[MAX_WORDS][MAX_WORD_LENGTH], char* arguments[]);
int getNumEntries(char arr[MAX_WORDS][MAX_WORD_LENGTH]);
int isValidRedirection(char args[MAX_WORDS][MAX_WORD_LENGTH], char *input);
void splitLeftRight(char* input, char* left, char* right, int index);
int getNumWords(char* input);
void printError();

int main(int argc, char *argv[]) {
    char args[MAX_WORDS][MAX_WORD_LENGTH];
    memset(args, '\0', sizeof(args));
    char path[MAX_WORDS][MAX_WORD_LENGTH];
    memset(path, '\0', sizeof(args));
    strcpy(path[0], "/bin"); //default path
    
    size_t size = 32; 
    size_t input;
    char *buffer;
    buffer = (char *)malloc(size * sizeof(char));
    FILE *ptr;
	
	if(argc == 1) { //interactive 
		while(1) {
			memset(args, '\0', sizeof(args));
			printf("wish>");
			input = getline(&buffer, &size, stdin);
			if(isValidRedirection(args, buffer) == 0) {
					memset(args, '\0', sizeof(args));
					splitString(args, buffer);
					executeCommand(args, path, 0);
				} else if(isValidRedirection(args, buffer) == 1) {
					memset(args, '\0', sizeof(args));
					splitString(args, buffer);
					executeCommand(args, path, 1);
				} else {
					printError();
				}					
		}	
	} else if(argc == 2) { //batch
		ptr = fopen(argv[1], "r");
		int check = 0;
		if(ptr != NULL) {
			while(input = getline(&buffer, &size, ptr) != -1) {
				check = isValidRedirection(args, buffer);
				if(check == 0 && !(isEmpty(buffer))) {
					memset(args, '\0', sizeof(args));
					splitString(args, buffer);
					executeCommand(args, path, 0);
				} else if(check == 1 && !(isEmpty(buffer))) {
					memset(args, '\0', sizeof(args));
					splitString(args, buffer);
					executeCommand(args, path, 1);
				} else if(check == -1) {
					printError();
				}
			}
			fclose(ptr);
		} else {
			printError();
			exit(1);
		}
	} else {
		printError();
		exit(1);
	}
	free(buffer);
}

void executeCommand(char args[MAX_WORDS][MAX_WORD_LENGTH], char path[MAX_WORDS][MAX_WORD_LENGTH], int isRedirect) {
	
	int lastEntry = getNumEntries(args) - 1; //index of last element
	char* redirect;
	if(isRedirect) {
		redirect = (char *)malloc(MAX_WORD_LENGTH * sizeof(char));
		redirect = strcpy(redirect, args[lastEntry]);
		memset(args[lastEntry], '\0', sizeof(args[lastEntry])); //get rid before execv
	}
	
	if(strcmp(args[0], "cd") == 0) {
		if(args[1][0] == '\0' || (args[1][0] != '\0' && args[2][0] != '\0')) {
			printError();
		} else {
			int check = chdir(args[1]);
			if(check == -1) { 
				printError();
			}
		}		
	} else if(strcmp(args[0], "exit") == 0) { //exit
		if(args[1][0] != '\0') {
			printError();
		} else {
			exit(0);
		}
	} else if(strcmp(args[0], "path") == 0) { //path
		int i = 0;
		while(args[i][0] != '\0') {
			strcpy(path[i], args[i + 1]);
			i++;
		}
	} else { //other files 
		char * arguments[200]; //needed for execv
		getArguments(args, arguments);
		
		int numDirectories = getNumEntries(path);
		if(numDirectories == 0) {
			printError();
			return;
		}
		//check each element of path
		char* absPath;
		for(int i = 0; i < numDirectories; i++) {
			absPath = (char *)malloc(MAX_WORD_LENGTH * sizeof(char));
			absPath = strcpy(absPath, path[i]);
			absPath = strcat(absPath, "/");
			absPath = strcat(absPath, args[0]);
			if(access(absPath, X_OK) != -1)
				break;
		}
		
		if(access(absPath, X_OK) == -1) {
			printError();
		} else {
			int rc = fork();
			if(rc < 0) {
				printError();
				exit(1);
			} else if (rc == 0)  { //in the child process
				if(isRedirect == 1) { //only redirect if the user wants
					close(STDOUT_FILENO);
   	 				open(redirect, O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU);
   	 				free(redirect);
				}
				execv(absPath, arguments);
			} else {
				wait(NULL);
			}
		}

	}
}

void splitString(char args[MAX_WORDS][MAX_WORD_LENGTH], char *buffer) {
	char *noNewLine; //to get rid of new line
	noNewLine = strtok(buffer, "\n");
	char *temp;
	temp = strtok(noNewLine, " >");
	int count = 0;
	
	while((temp != NULL)) {
		strcpy(args[count], temp);
		count++;
		temp = strtok(NULL, " >");
	}
}

int isEmpty(char *buffer) {
	for(int i = 0; i < strlen(buffer) - 2; i++) {
		if(buffer[i] != ' ' && buffer[i] != '\t') {
			buffer += i; //gets rid of leading white space
			return 0;
		}
	}
	return 1;
}

void getArguments(char args[MAX_WORDS][MAX_WORD_LENGTH], char* arguments[]) {
	int i = 0;
	while(args[i][0] != '\0') {
		arguments[i] = args[i];
		i++;
	}
	arguments[i] = NULL;
}

int getNumEntries(char arr[MAX_WORDS][MAX_WORD_LENGTH]) {
	int i = 0;
	while(arr[i][0] != '\0') {
		i++;
	}
	return i;
}

int isValidRedirection(char args[MAX_WORDS][MAX_WORD_LENGTH], char *input) {
	int count = 0;
	int index = 0; //index of first >
	int isValid = 0;
	for(int i = 0; i < strlen(input) - 1; i++) {
		if(input[i] == '>') {
			if(count == 0) {
				index = i;
			}		
			count++;
		}
	}
	char *left;
	left = (char *)malloc(MAX_WORD_LENGTH * sizeof(char));
	char *right;
	right = (char *)malloc(MAX_WORD_LENGTH * sizeof(char));
	//cheeky method to help get around the inconsistency of the spaces between
	//the > and the words sometimes it is there and sometimes not
	splitLeftRight(input, left, right, index);
	if(count == 0) { //not an error, but not a valid redirect
		isValid = 0;
	} else if(count = 1 && input[strlen(input) - 2] != '>' && input[0] != '>'
	&& getNumWords(right) <= 1) { //a valid attempt to redirect
		isValid = 1;		
	} else { //error
		isValid = -1;
	}	
	free(left);
	free(right);
	return isValid;	
}

void splitLeftRight(char* input, char* left, char* right, int index) {
	for(int i = 0; i < index; i++) {
		left[i] = input[i];
	}
	int temp = 0;
	for(int i = index + 1; i < strlen(input); i++) {
		right[temp] = input[i];
		temp++;
	}
}

int getNumWords(char* input) {
	int i;
	int count = 0;
	for(i = 0; i < strlen(input); i++) {
		if(input[i] == ' ') {
			count++;
		}
	}
	return count;
}

void printError() {
	write(STDERR_FILENO, ERROR_MSG, strlen(ERROR_MSG));
}
