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

int countWhiteSpace(char *input);

void parseCommand(char *input);

void normalCommand(char *input);

void singleCommand(char *input, int length);

int checkQuit(char *input);



int main (void) {
    int rc;
    char buff[100];

	while(quit == 0)
	{
		rc = getLine ("Enter string> ", buff, sizeof(buff));
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
		
		// parse the char array
		parseCommand(buff);
		
		//singleCommand(buff, 1);
	}

    
    return 0;
}

// This will tell you how many whitespace characters there are.
int countWhiteSpace(char *input){
	int len = strlen(input);
	int i, count = 0;
	for(i=0; i<len; i++){
		if(input[i] == ' '){
			count++;
		}	
	}
	return count;
}

void parseCommand(char *input){
	// find how much white space there is
	int whiteSpaces = countWhiteSpace(input);
	// make char* array of whiteSpaces+1 because that is how many words there are

	// tokenize input
	
	// check the tokenized input for ; & | >

	// call appropriate method depending on part above
	
}

void singleCommand(char *input, int length){
	int rc = fork();
    if (rc < 0) {
        // fork failed; exit
        fprintf(stderr, "fork failed\n");
        exit(1);
    }
    else if (rc == 0) {
		
        char *myargs[3];
        myargs[0] = strdup("ls");   // program: "wc" (word count)
        myargs[1] = strdup("-al"); // argument: file to count
        myargs[2] = NULL;           // marks end of array
        execvp(myargs[0], myargs);  // runs word count
    }
    else {
        // parent goes down this path (original process)
        int wc = wait(NULL);
    }
}

void normalCommand(char *input){
	printf("call normalCommand\n");
	int rc = fork();
    if (rc < 0) {
        // fork failed; exit
        fprintf(stderr, "fork failed\n");
        exit(1);
    }
    else if (rc == 0) {
		// child: redirect standard output to a file
		close(STDOUT_FILENO);
		open("./p4.output", O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU);
		printf("made it");
		// now exec "wc"...
        char *myargs[3];
        myargs[0] = strdup("ls");   // program: "wc" (word count)
        myargs[1] = strdup("-al"); // argument: file to count
        myargs[2] = NULL;           // marks end of array
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
    buff[strlen(buff)-1] = '\0';
    return OK;
}
