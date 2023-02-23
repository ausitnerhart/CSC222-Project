#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdbool.h>

const bool DEBUG = true;
const int MAX_LEN = 256;

char* CommandPrompt(){
	char cwd[MAX_LEN];
	char* input = malloc(MAX_LEN * sizeof(char));

	getcwd(cwd, sizeof(cwd));
	printf("%s $ ", cwd);

	if(fgets(input, MAX_LEN, stdin) == NULL) {
		return NULL;
	}

	input[strcspn(input, "\n")] = 0;
	return input;
}

struct ShellCommand{
	int argc;
	char** argv;
	int inputRedirect;
	char* inputFile;
	int outputRedirect;
	char* outputFile;
};

struct ShellCommand ParseCommandLine(char* input){

	struct ShellCommand sc = {0};
	char* token;
	char* nextToken;
	int i;

	sc.argc = 0;
	sc.argv = (char**) malloc(sizeof(char*));
	sc.inputRedirect = 0;
	sc.inputFile = NULL;
	sc.outputRedirect = 0;
	sc.outputFile = NULL;

	token = strtok(input, " \t");
	while(token != NULL){
		if(strcmp(token, "<") == 0){
			sc.inputRedirect = 1;
			nextToken = strtok(NULL, " \t");
			if(nextToken == NULL){
				fprintf(stderr, "Error no input file specified \n");
				exit(0);
			}
			sc.inputFile = strdup(nextToken);
		} else if(strcmp(token, ">") == 0){
			sc.outputRedirect = 1;
			nextToken = strtok(NULL, " \t");
			if(nextToken == NULL){
				nextToken = "a.out";
			}
			sc.outputFile = strdup(nextToken);
		} else{
			sc.argv[sc.argc] = strdup(token);
			sc.argc++;
			sc.argv = (char**)realloc(sc.argv, (sc.argc + 2) * sizeof(char*));
		}

		sc.argv[sc.argc] = NULL;
		token = strtok(NULL, "\t");

	}
	return sc;
}

void ChangeDir(struct ShellCommand command){
	if(command.argc != 2) {
		fprintf(stderr, "Error: cd command requires exactly one argument\n");
		return;
	}

	int status = chdir(command.argv[1]);
	if(status != 0) {
		perror("Error changing directory");
	}
}

void ExecuteCommand(struct ShellCommand command){
	pid_t pid = fork();
	int status;

	if(pid == 0){
		// Child process
		if(command.inputRedirect){
			FILE* fileIn = fopen(command.inputFile, "r");
			dup2(fileno(fileIn), 0);
			fclose(fileIn);
		}

		if(command.outputRedirect){
		}
		
		execvp(command.argv[0], command.argv);
		perror("Error executing command");
		exit(1);
	} else if(pid > 0){
		//Parent process
		waitpid(pid, &status, 0);
	} else{
		//Forking error
		perror("Error forking process");
		exit(1);
	}

}




int main(){
	char* input;
	struct ShellCommand command;

	while(true){
		input = CommandPrompt();

		command = ParseCommandLine(input);
		free(input);
		
		if(strcmp(command.argv[0], "cd") == 0){
			ChangeDir(command);
			
		} else{
			ExecuteCommand(command);
		}
	}

	exit(0);
}
