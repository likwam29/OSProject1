// Matthew Likwarz
// Project 1
// CS 421
// String reading example found here https://stackoverflow.com/questions/4023895/how-to-read-string-entered-by-user-in-c

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>

//Prototyped methods

#define OK       0
#define NO_INPUT 1
#define TOO_LONG 2

int quit = 0;

static int getLine (char *prmpt, char *buff, size_t sz);

int countWhiteSpace(char input[100]);

void parseCommand(char input[100]);

void outputCommand(char *input[], int length);

void singleCommand(char *input[], int length);

void sequentialCommand(char *input[], int length);

void backGroundCommand(char *input[], int length);

int checkQuit(char *input);

void removeStringTrailingNewline(char *str);



int main (void) {
    int rc;
    char buff[100];

	while(quit == 0)
	{
		rc = getLine ("Enter command> ", buff, sizeof(buff));
		if (rc == NO_INPUT) {
		    // Extra NL since my system doesn't output that on EOF.
		    printf ("\nNo input\n");
		    break;
		}

		if (rc == TOO_LONG) {
		    printf ("Input too long [%s]\n", buff);
		    break;
		}
		
		// Quit out of program if true
		if(checkQuit(buff) == 1){
			quit = 1;
			break;
		}
		
		// parse the char array and call appropriate method to run
		parseCommand(buff);
	}

    return 0;
}

// This will tell you how many whitespace characters there are.
int countWhiteSpace(char input[100]){
	int len = strlen(input);
	int i, count = 0;
	for(i=0; i<len; i++){
		if(input[i] == ' '){
			count++;
		}	
	}
	return count;
}

void parseCommand(char input[100]){
	int i = 0;
	// find how much white space there is
	int whiteSpaces = countWhiteSpace(input);
	// make char* array of whiteSpaces+1 because that is how many words there are
	char *tokenizedString[whiteSpaces + 1];
	
	// tokenize input
	char *temp = strtok(input, " ");
	while(temp != NULL){
		tokenizedString[i++] = temp;
		temp = strtok(NULL, " ");	
	}
	
	// check the tokenized input for ; & | >
	for (i = 0; i < (whiteSpaces + 1); i++){
		
		if(tokenizedString[i][0] == ';'){
			printf("SemiColon\n");
			sequentialCommand(tokenizedString, (whiteSpaces+1));
			return;
		} else if(tokenizedString[i][0] == '&'){
			printf("Ampersand\n");
			backGroundCommand(tokenizedString, (whiteSpaces+1));
			return;
		}else if(tokenizedString[i][0] == '|'){
			printf("pipe\n");
			return;
		}else if(tokenizedString[i][0] == '>'){
			printf("Carrot\n");
			outputCommand(tokenizedString, (whiteSpaces+1));
			return;
		}
	} 
        
	// Call basic exec
	singleCommand(tokenizedString, (whiteSpaces+1));
}

void singleCommand(char *input[], int length){

	int rc = fork();
    if (rc < 0) {
        // fork failed; exit
        fprintf(stderr, "fork failed\n");
        exit(1);
    }
    else if (rc == 0) {
		char *myargs[3];
		if(length == 1){
		    myargs[0] = strdup(input[0]);
		    myargs[1] = NULL;
		}else{
		    myargs[0] = strdup(input[0]);
		    myargs[1] = strdup(input[1]);
		    myargs[2] = NULL;
		}

        execvp(myargs[0], myargs);
    }
    else {
        // parent goes down this path (original process)
        int wc = wait(NULL);
    }
}

void sequentialCommand(char *input[], int length){
	int firstLength = 0;
	int secondLength = 0;
	int i;
	int afterSemiColon = 0; 
	for(i = 0; i<length; i++){
		if(input[i][0] == ';'){
			afterSemiColon = 1;
			continue;
		}
		if(afterSemiColon == 0){
			firstLength++;		
		}else{
			secondLength++;
		}
	}
	
	char *input1[firstLength];
	char *input2[secondLength];

	for(i=0; i<firstLength; i++){
		input1[i] = input[i];
	}

	for(i=0; i<secondLength; i++){
		input2[i] = input[i + firstLength + 1];
	}
	

	int rc = fork();
    if (rc < 0) {
        // fork failed; exit
        fprintf(stderr, "fork failed\n");
        exit(1);
    }
    else if (rc == 0) {
		singleCommand(input1, firstLength);
		singleCommand(input2, secondLength);
    }
    else {
        // parent goes down this path (original process)
        int wc = wait(NULL);
    }
	
}

// This will run an exec in the background
void backGroundCommand(char *input[], int length){
	int rc = fork();
    if (rc < 0) {
        // fork failed; exit
        fprintf(stderr, "fork failed\n");
        exit(1);
    }
    else if (rc == 0) {
		char *myargs[3];
        myargs[0] = strdup(input[0]);   // program: "wc" (word count)
        myargs[1] = strdup(input[1]); // argument: file to count
        myargs[2] = NULL;

		if(myargs[1][0] == ' '){
			myargs[1] = NULL;		
		}

		if(myargs[1][0] == '&'){
			myargs[1] = NULL;		
		}

        execvp(myargs[0], myargs);  // runs word count
    }
    else {
        // no need to wait
    }
}

// This will take in a command and output it to a given file
void outputCommand(char *input[], int length){
	
	removeStringTrailingNewline(input[length-1]);
	int rc = fork();
    if (rc < 0) {
        // fork failed; exit
        fprintf(stderr, "fork failed\n");
        exit(1);
    }
    else if (rc == 0) {
		// child: redirect standard output to a file
		
		close(STDOUT_FILENO);
		open(input[length-1], O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU);
		
		char *myargs[3];
        myargs[0] = strdup(input[0]);   // program: "wc" (word count)
        myargs[1] = strdup(input[1]); // argument: file to count
        myargs[2] = NULL;           // marks end of array

		if(myargs[1][0] == ' '){
			myargs[1] = NULL;		
		}

		if(myargs[1][0] == '>'){
			myargs[1] = NULL;		
		}
		
        execvp(myargs[0], myargs);  // runs word count
    }
    else {
        // parent goes down this path (original process)
        int wc = wait(NULL);
    }
}

// This will check to see if the input == quit or QUIT
// If true return 1 else 0
int checkQuit(char *input){
	if(input[0] == 'q' && input[1] == 'u' && input[2] == 'i' && input[3] == 't'){
		return 1;	
	}
	
	if(input[0] == 'Q' && input[1] == 'U' && input[2] == 'I' && input[3] == 'T'){
		return 1;	
	}
	
	return 0;
}

void removeStringTrailingNewline(char *str) {
  if (str == NULL)
    return;
  int length = strlen(str);
  if (str[length-1] == '\n')
    str[length-1]  = '\0';
}


static int getLine (char *prmpt, char *buff, size_t sz) {
    int ch, extra;

    // Get line with buffer overrun protection.
    if (prmpt != NULL) {
        printf ("%s", prmpt);
        fflush (stdout);
    }
    if (fgets (buff, sz, stdin) == NULL)
        return NO_INPUT;

    // If it was too long, there'll be no newline. In that case, we flush
    // to end of line so that excess doesn't affect the next call.
    if (buff[strlen(buff)-1] != '\n') {
        extra = 0;
        while (((ch = getchar()) != '\n') && (ch != EOF))
            extra = 1;
        return (extra == 1) ? TOO_LONG : OK;
    }

    // Otherwise remove newline and give string back to caller.
	buff[strlen(buff) -1] = '\0';
    return OK;
}
