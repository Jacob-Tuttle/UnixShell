#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


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

        //process entered command
        while((token = strsep(&line, &delim)) != NULL){
            if(strcmp(token,"exit") == 0){
                // Free dynamically allocated memory
                free(line);
                exit(0);
            }
            else if(strcmp(token, "ls") == 0){
                char *argv[] = {"ls", "-1", NULL};
                if(access("/bin/ls", X_OK) == 0){

                    execv("/bin/ls", argv);
                }
                else if (access( "/usr/bin/ls", X_OK) == 0){
                    execv("/bin/ls", argv);
                }
                else{
                    printf("Error: command not found\n");
                }
            }
            else{
                printf("%s\n", token);
            }
        }
    }
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

