// Matthew Likwarz
// Project 1
// CS 421

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>

//Prototyped methods

int quit = 0;

static int getInput(char *prompt, char *buffer, size_t size);

int countWhiteSpace(char input[100]);

void parseCommand(char input[100]);

void outputCommand(char *input[], int length, int numCommands);

void singleCommand(char *input[], int length);

void pipeCommand(char *input[], int length);

void sequentialCommand(char *input[], int length);

void backGroundCommand(char *input[], int length);

int checkQuit(char *input);

void removeStringTrailingNewline(char *str);

// This is the main method that runs the program
int main (void) {
    int validInput;
    char buff[100];

	while(quit == 0)
	{
		memset(buff, 0, sizeof buff);
		validInput = getInput("Enter command> ", buff, sizeof(buff));
		if (validInput == 1) {
		    printf ("\nNo input\n");
		    break;
		}

		if (validInput == 2) {
		    printf ("Input too long only char[100]\n");
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
		
		// first check if special command
		if(tokenizedString[i][0] == ';'){
			sequentialCommand(tokenizedString, (whiteSpaces+1));
			return;
		} else if(tokenizedString[i][0] == '&'){
			backGroundCommand(tokenizedString, (whiteSpaces+1));
			return;
		}else if(tokenizedString[i][0] == '|'){
			pipeCommand(tokenizedString, (whiteSpaces+1));
			return;
		}else if(tokenizedString[i][0] == '>'){
			outputCommand(tokenizedString, (whiteSpaces+1), i);
			return;
		}
	} 
        
	// Call basic exec
	singleCommand(tokenizedString, (whiteSpaces+1));
}

// This will run a single command that doesn't have ; > | & in it
void singleCommand(char *input[], int length){

	int rc = fork();
    if (rc < 0) {
        fprintf(stderr, "fork failed\n");
        exit(1);
    }
    else if (rc == 0) {
		// child
		int i = 0;
        char *myargs[length+1];

		for(i = 0; i<length; i++){
			myargs[i] = strdup(input[i]);
		}
        myargs[length] = NULL;
        execvp(myargs[0], myargs);
    }
    else {
        // parent
        wait(NULL);
    }
}

// This will run a piped input linux command
void pipeCommand(char *input[], int length){
	int firstLength = 0;
	int secondLength = 0;
	int i;
	int afterSemiColon = 0; 
	for(i = 0; i<length; i++){
		if(input[i][0] == '|'){
			afterSemiColon = 1;
			continue;
		}
		if(afterSemiColon == 0){
			firstLength++;		
		}else{
			secondLength++;
		}
	}
	
	char *input1[firstLength + 1];
	char *input2[secondLength + 1];

	for(i=0; i<firstLength; i++){
		input1[i] = input[i];
	}
	input1[firstLength] = NULL;

	for(i=0; i<secondLength; i++){
		input2[i] = input[i + firstLength + 1];
	}
	input2[secondLength] = NULL;
	
	int rc = fork();
    if (rc < 0) {
        fprintf(stderr, "fork failed\n");
        exit(1);
    }
    else if (rc == 0) {
		int fd[2];
		//make the pipe
		pipe(fd);
		int pid = fork();
		if (pid < 0) {
		    fprintf(stderr, "fork failed\n");
		    exit(1);
		}
		else if (pid == 0) {
			// close this end of the pipe because the child doesn't need it
			close(fd[1]);
			dup2(fd[0], 0);
			execvp(input2[0], input2);
			close(fd[0]);
		}
		else {
			// close this end of the pipe because the parent doesn't need it
			close(fd[0]);
			dup2(fd[1], 1);
			execvp(input1[0], input1);
			close(fd[1]);
		}
    }
    else {
       wait(NULL);
    }
}

// This will run two commands sequentially
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
        fprintf(stderr, "fork failed\n");
        exit(1);
    }
    else if (rc == 0) {
		// child
		singleCommand(input1, firstLength);
		singleCommand(input2, secondLength);
    }
    else {
        // parent
        wait(NULL);
    }
	
}

// This will run an exec in the background
void backGroundCommand(char *input[], int length){
	int rc = fork();
    if (rc < 0) {
        fprintf(stderr, "fork failed\n");
        exit(1);
    }
    else if (rc == 0) {
		int i = 0;
        char *myargs[length];

		for(i = 0; i<length; i++){
			myargs[i] = strdup(input[i]);
		}
        myargs[length -1] = NULL;
        execvp(myargs[0], myargs);
    }
    else {
        // no need to wait on child
    }
}

// This will take in a command and output it to a given file
void outputCommand(char *input[], int length, int numCommands){
	int rc = fork();
    if (rc < 0) {
        fprintf(stderr, "fork failed\n");
        exit(1);
    }
    else if (rc == 0) {
		// child
		
		close(STDOUT_FILENO);
		open(input[length-1], O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU);
		
		int i = 0;
        char *myargs[numCommands+1];

		for(i = 0; i<numCommands; i++){
			myargs[i] = strdup(input[i]);
		}
        myargs[numCommands] = NULL;
        execvp(myargs[0], myargs);
    }
    else {
        // parent
        wait(NULL);
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

// This method will print out the prompt and send back what the user input
// with some basic error handling
static int getInput(char *prompt, char *buffer, size_t size) {
    int ch, extra;

    // Get line with buffer overrun protection.
    if (prompt != NULL) {
        printf ("%s", prompt);
        fflush (stdout);
    }
    if (fgets (buffer, size, stdin) == NULL)
        return 1;

    // If it was too long, there'll be no newline. In that case, we flush
    // to end of line so that excess doesn't affect the next call.
    if (buffer[strlen(buffer)-1] != '\n') {
        extra = 0;
        while (((ch = getchar()) != '\n') && (ch != EOF))
            extra = 1;
        return (extra == 1) ? 2 : 0;
    }

    // Otherwise remove newline and give string back to caller.
	buffer[strlen(buffer) -1] = '\0';
    return 0;
}
