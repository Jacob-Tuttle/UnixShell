#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

bool check_Build_In(char *cmd){
    char *builtin[3] = {"exit","cd","path"};
    for(int i = 0; i < 3;i++){
        if(strcmp(cmd,builtin[i])==0){
            return true;
        }
    }
    return false;
}

char* check_access(char **path, char *cmd, int pathSize){

    for(int i = 0; i < pathSize; i++){

        // Calculate the length of the resulting string
        size_t len = strlen(cmd) + strlen(path[i]);

        // Allocate memory for the new string
        char *New = (char *)malloc(len + 1);

        if (New != NULL) {
        // Copy the path into the new string
            strcpy(New, path[i]);

            // Concatenate the command at the end
            strcat(New, cmd);
            if(access(New,X_OK) == 0){
                return New;
            }
        }
    }
    return "Empty";
}

char** update_path(char** currentPaths, int *pathSize, char *add) {

    //Creates a new path array of strings which will hold one
    //additional string contained in add
    char** newPaths = (char**)malloc((*pathSize + 1) * sizeof(char*));



    for (int i = 0; i < *pathSize; i++) {
        newPaths[i] = (char*)malloc(strlen(currentPaths[i]) + 1);
        strcpy(newPaths[i], currentPaths[i]);
    }
    //allocate space for a string which is the length of add
    newPaths[*pathSize] = (char*)malloc(strlen(add+1));
    strcpy(newPaths[*pathSize], add);
    //increase pathSize for newly added path
    (*pathSize)++;
    free(currentPaths);
    return newPaths;
}

char** clearPath(char **path, int *pathSize){
    for(int i = 0; i<*pathSize; i++){
        free(path[i]);
    }
    free(path);
    *pathSize = 0;
    return NULL;
}


char** initilize_path(int *Size){
    char** path = (char**)malloc(1 * sizeof(char*));
    path[0] = (char*)malloc(strlen("/bin/") + 1);
    path[0] = "/bin/";
    *Size+=1;
    return path;
}

void printPath(char **path, const int pathSize){
    for(int i = 0; i < pathSize; i++){
        printf("path %d: %s\n", i+1, path[i]);
    }
}

int main() {
    char **path;//starting path
    int pathSize=0; //nubmer of paths stored
    path = initilize_path(&pathSize);

    while(1){
        char *line = NULL, *token = NULL; //contains the entire line entered || stores individual tokens of line
        size_t len = 0; //Size of the line
        char *delim = " \t\n"; //Used for strsep to break line into tokens
        int Size = 1;
        int count = 0; //Number of tokens
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


        //creates argument array that is null terminated inorder to run execv
        char **arg = (char **)malloc((count + 1) * sizeof(char*)); // Allocate memory for arg

        if (arg == NULL) {
            exit(1);
        }

        for (int j = 0; j < count; j++) {
            arg[j] = (char *)malloc(strlen(tokens[j]) + 1);
            strcpy(arg[j], tokens[j]);
        }

        arg[count] = NULL;

        //process line entered
        for(int j = 0; j < count; j++){
            token = tokens[j]; //current token being read

            //if cmd is not build in cmd
            if(check_Build_In(tokens[j]) == false){
                pid_t child_pid = fork();
                if (child_pid == 0) {
                    // This code block will be executed by the child process
                    //if access is found returns location of executable
                    char* Loc=check_access(path,tokens[j],pathSize);
                    if(strcmp(Loc,"Empty") != 0){
                        execv(Loc,arg);
                    }
                }
                else if (child_pid > 0) {
                    // This code block will be executed by the parent process
                    wait(NULL);  // Wait for the child process to finish
                }

            }
            else if(check_Build_In(tokens[j]) == true){
                if(strcmp(tokens[j],"exit") == 0){
                    free(line);
                    exit(0);
                }
                else if(strcmp(tokens[j],"cd") == 0){
                    if(count == 2){
                        chdir(tokens[j+1]);
                    }
                    else{
                        //error
                    }
                }
                else if(strcmp(tokens[j],"path") == 0){
                    //will update path and pathSize(number of paths stored)
                    if(count != 1){
                        //adds all paths entered in path array
                        for(int i = 1; i <count; i++){
                            path = update_path(path,&pathSize, tokens[i]);
                        }
                        printPath(path,pathSize);
                    }
                    else{
                        path = clearPath(path,&pathSize);
                        printPath(path,pathSize);
                    }
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
