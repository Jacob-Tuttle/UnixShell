#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    while(1){
        char *line = NULL, *token = NULL; //contains the entire line entered || stores individual tokens of line
        size_t len = 0; //Size of the line
        char *delim = " \t\n"; //Used for strsep to break line into tokens
        int Size = 1;
        int count = 0;
        char **tokens;
        tokens = (char **)malloc(Size * sizeof(char*));

        //Error allocating
        if(tokens == NULL){
            exit(1);
        }

        printf("wish> ");
        // Use getline to read from stdin
        //and split line into tokens using strsep
        getline(&line, &len, stdin);
        while((token = strsep(&line, delim)) != NULL){
            //will remove consecutive delimiters
            if(*token != '\0'){
                if(count >= Size){
                    Size +=1;
                    tokens = (char**)realloc(tokens, Size * sizeof(char*));
                    tokens[count] = (char*)malloc(strlen(token) + 1);
                    strcpy(tokens[count],token);
                    //printf("%d: %s\n",count, tokens[count]);
                    count+=1;

                }
                else if(count == 0){
                    tokens[count] = (char*)malloc(strlen(token) + 1);
                    strcpy(tokens[count],token);
                    //printf("%d: %s\n",count, tokens[count]);
                    count+=1;
                }
                else{
                    exit(1);
                }
            }
        }

        char **arg = (char **)malloc((count + 1) * sizeof(char*)); // Allocate memory for arg

        if (arg == NULL) {
            exit(1);
        }

        for (int j = 0; j < count; j++) {
            arg[j] = (char *)malloc(strlen(tokens[j]) + 1);
            strcpy(arg[j], tokens[j]);
        }

        arg[count] = NULL;

        //process
        for(int j = 0; j < count; j++){
            token = tokens[j];
            if(strcmp(tokens[j],"exit") == 0){
                // Free dynamically allocated memory
                free(line);
                exit(0);
            }
            else if(strcmp(tokens[j], "ls") == 0){
                pid_t child_pid = fork();
                if (child_pid == 0) {
                    // This code block will be executed by the child process
                    if (access("/bin/ls", X_OK) == 0) {
                        execv("/bin/ls", arg);
                    }
                    else if (access("/usr/bin/ls", X_OK) == 0) {
                        execv("/usr/bin/ls",arg);
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
                if(count == 2){
                    chdir(tokens[j+1]);
                }
                else{
                    printf("Error\n");
                }
            }
            else{
                //printf("no matching cmd\n");
            }
        }
        free(line);
        free(tokens);
    }

    return 0;
}
