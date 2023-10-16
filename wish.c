#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

void interactiveMode(){

    while(1){
        char *line = NULL, *token = NULL;
        size_t len = 0;
        char delim = ' ';

        printf("wish> ");

        // Use getline to read from stdin
        getline(&line, &len, stdin);
        // Remove the trailing newline character
        line[strcspn(line, "\n")] = '\0';

        int argNum = 0;
        char *temp = line;
        char **tokens = malloc(sizeof(char *) * len); // Store tokens in an array

        //separates individual tokens into tokens array
        //will be used for execution below
        while ((token = strsep(&temp, &delim)) != NULL) {
            tokens[argNum] = token;
            argNum += 1;
        }

        //executes given tokens
        for(int j = 0; j < argNum; j++){
            if(strcmp(tokens[j],"exit") == 0){
                // Free dynamically allocated memory
                free(line);
                exit(0);
            }
            else if(strcmp(tokens[j], "ls") == 0){
                pid_t child_pid = fork();
                if (child_pid == 0) {
                    // This code block will be executed by the child process
                    char *argv[] = {"ls", "-1", NULL};
                    if (access("/bin/ls", X_OK) == 0) {
                        execv("/bin/ls", argv);
                    }
                    else if (access("/usr/bin/ls", X_OK) == 0) {
                        execv("/usr/bin/ls", argv);
                    }
                    else {
                        printf("Error: command not found\n");
                    }
                }
                else if (child_pid > 0) {
                    // This code block will be executed by the parent process
                    wait(NULL);  // Wait for the child process to finish
                }
            }
            else if(strcmp(tokens[j], "cd") == 0){
                if(argNum == 2){
                    chdir(tokens[j+1]);
                    break;
                }
                else{
                    printf("Error\n");
                }
            }
            else{
                printf("%s\n", token);
            }
        }
    }

    free(line);
    free(tokens);
}

void batchRead(){
}

int main(int argc, char *argv[]){

    //No arguemnt
    if(argc == 1){
        interactiveMode();
    }
    //batch file
    else if(argc == 2){
        batchRead();
    }
    //error
    else{
        exit(1);
    }
  return 0;
}
