#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

// Function that takes a pointer to an int and modifies the value it points to
char** tokenize(char *line,char *delims, int *argNum){
    char *token = NULL;
    int Size = 1;
    *argNum = 0;
    char** args;
    args = (char**)malloc(Size * sizeof(char*));

    while((token = strsep(&line, delims)) != NULL){
        //will remove consecutive delimiters
        if(*token != '\0'){
            if(*argNum >= Size){
                Size +=1;
                args = (char**)realloc(args, Size * sizeof(char*));
                args[*argNum] = (char*)malloc(strlen(token) + 1);
                strcpy(args[*argNum],token);
                //printf("%d: %s\n",*argNum, args[*argNum]);
                *argNum+=1;

            }
            else if(*argNum == 0){
                args[*argNum] = (char*)malloc(strlen(token) + 1);
                strcpy(args[*argNum],token);
                //printf("%d: %s\n",*argNum, args[*argNum]);
                *argNum+=1;
            }
            else{
                exit(1);
            }
        }
    }
    return args;
}

// Function to copy char** elements into a new char**
char** copyStringArray(char **original, int length) {
    char** copied = (char**)malloc(length * sizeof(char*));

    if (copied == NULL) {
        // Memory allocation failed
        return NULL;
    }

    for (int i = 0; i < length; i++) {
        if (original[i] != NULL) {
            copied[i] = (char*)malloc(strlen(original[i]) + 1);
            if (copied[i] == NULL) {
                // Memory allocation failed for a string, clean up and return NULL
                for (int j = 0; j < i; j++) {
                    free(copied[j]);
                }
                free(copied);
                return NULL;
            }
            strcpy(copied[i], original[i]);
        } else {
            copied[i] = NULL;
        }
    }

    return copied;
}

/*


*/
char** preProcess(char *line, int lineLen, int *argNum,
                bool *multiCheck, bool *redirectCheck, char **file){

    int numRedirect = 0;
    *multiCheck = false;
    *redirectCheck = false;

    //check for multiple redirect and multicmd commands
    //lineLen -1 to account for null terminator

    //used for consecutive > and & check
    char prev = '\0';
    char cur = line[0];
    bool multiReached = false; //will be used to make sure there
                            //is not multiple > before a new cmd start
    //just new line
    if(lineLen == 1){
        *argNum = 0;
        return NULL;
    }
    //line contine only &, newline, space, or tab
    else if(lineLen == 2 && (line[0] == '&' || line[0] == '\t' || line[0] == '\n' || line[0] == ' ')){
        *argNum = 0;
        return NULL;
    }


    for(int i = 1; i < lineLen; i++){
        //printf("prev: %c\n", prev);
        //printf("cur: %c\n", cur);
        if(cur == '>' || cur == '&'){

            if(cur == '&'){
                *multiCheck = true;
                multiReached=true;
                numRedirect = 0;
            }
            else if(cur == '>'){
                multiReached=false;
                *redirectCheck = true;
                numRedirect += 1;
            }
            if(numRedirect > 1 && multiReached == false){
                *argNum = -1;
                return NULL; //error multiple > before &
            }
            else if((prev == '>' && cur == '>')
                    || (prev == '>' && cur == '&')
                    || (prev == '&' && cur == '>')
                    || (prev == '&' && cur == '&')){

                *argNum = -1;
                return NULL; //error consecutive >
            }
        }
        if(cur != ' ' && cur != '\t' && cur != '\n'){
            prev = cur;
            cur = line[i];
        }
        else{
            cur = line[i];
        }
    }

    if(*redirectCheck == true && *multiCheck == false){
        char *redDelim = ">\n";
        char *delim = " \t\n"; //Used for strsep to break line into tokens
        int count = 0; //Number of tokens
        char **args; // will store the seperated arg and file
                    // for further processing

        //should be size of 2 where the first
        //elements is the cmd and args and the second is a file
        args = tokenize(line,redDelim,&count);
        //printf("\ncount: %d\n", count);
        if(count < 2){
            *argNum = -1;
            return NULL;
        }

        //check for multiple file passed
        char **temp = copyStringArray(args, count);
        int tempCount = 0;
        tokenize(temp[1],delim,&tempCount);
        if(tempCount>1){
            *argNum = -1;
            return NULL;
        }
        temp = tokenize(args[0], delim, &count);
        *file = args[1];
        *argNum = count;
        return temp;
    }
    else if(*multiCheck == true){
        char *multiDelim = "&\n";
        int count = 0; //Number of tokens
        char **temp; // will store the seperated arg and file

        temp = tokenize(line,multiDelim,&count);
        *argNum = count;
        return temp;
    }
    else{
        int count = 0;
        char *delim = " \t\n"; //Used for strsep to break line into tokens
        char **temp = tokenize(line, delim, &count);
        *argNum = count;
        return temp;
    }

    *argNum = -1;
    return NULL;
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
    path[0] = (char*)malloc(strlen("/bin") + 1);
    path[0] = "/bin";
    *Size+=1;
    return path;
}

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

    char *result = (char*)malloc(strlen(cmd) + 2);

    if (result == NULL) {
        // Handle memory allocation failure
        return NULL;
    }

    char *temp = "/";
    strcat(result,temp);
    strcat(result,cmd);
    for(int i = 0; i < pathSize; i++){
        // Calculate the length of the resulting string
        size_t len = strlen(result) + strlen(path[i]);
        // Allocate memory for the new string
        char *New = (char *)malloc(len + 1);

        if (New != NULL) {
        // Copy the path into the new string
            strcpy(New, path[i]);

            // Concatenate the command at the end
            strcat(New, result);
            if(access(New,X_OK) == 0){
                return New;
            }
        }
    }
    return "Empty";
}

void printPath(char **path, const int pathSize){
    for(int i = 0; i < pathSize; i++){
        printf("path %d: %s\n", i+1, path[i]);
    }
}

int executeCmd(char **args,char ***path, int *pathSize, int count){
     char **temp = (char **)malloc((count + 1) * sizeof(char*)); // Allocate memory for arg

    if (temp == NULL) {
        exit(1);
    }

    for (int j = 0; j < count; j++) {
        temp[j] = (char *)malloc(strlen(args[j]) + 1);
        strcpy(temp[j], args[j]);
    }
    temp[count] = NULL;

    if(check_Build_In(temp[0]) == false){
        pid_t child_pid = fork();
        if (child_pid == 0) {
            // This code block will be executed by the child process
            //if access is found returns location of executable
            char* Loc=check_access(*path,temp[0],*pathSize);
            if(strcmp(Loc,"Empty") != 0){
                execv(Loc,temp);
            }
        }
        else if (child_pid > 0) {
            // This code block will be executed by the parent process
            wait(NULL);  // Wait for the child process to finish
        }
    }
    else if(check_Build_In(temp[0]) == true){
        if(strcmp(temp[0],"exit") == 0){
            return 1;
        }
        else if(strcmp(temp[0],"cd") == 0){
            if(count == 2){
                chdir(temp[1]);
            }
            else{
                char error_message[30] = "An error has occurred\n";
                write(STDERR_FILENO, error_message, strlen(error_message));
            }
        }
        else if(strcmp(temp[0],"path") == 0){
            //will update path and pathSize(number of paths stored)
            if(count != 1){
                //adds all paths entered in path array
                for(int i = 1; i <count; i++){
                    *path = update_path(*path,*&pathSize, temp[i]);
                }
            }
            else if(count == 1){
                if(*pathSize == 1){
                    *path = NULL;
                    *pathSize = 0;
                }
                else{
                    *path = clearPath(*path,*&pathSize);
                }
            }
        }
    }
    else{
        char error_message[30] = "An error has occurred\n";
        write(STDERR_FILENO, error_message, strlen(error_message));
    }

    for(int i = 0; i<count; i++){
        free(temp[i]);
    }
    free(temp);

    return 0;
}

int redirectCmdExec(char **args, char *file, int count, char **path, int pathSize){
    char **temp = (char **)malloc((count + 1) * sizeof(char*)); // Allocate memory for arg

    if (temp == NULL) {
        exit(1);
    }

    for (int j = 0; j < count; j++) {
        temp[j] = (char *)malloc(strlen(args[j]) + 1);
        strcpy(temp[j], args[j]);
    }
    temp[count] = NULL;
    //grab id of now open file
    int fd = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (fd == -1) {
        return 1;
    }

    pid_t child_pid = fork();
    if (child_pid == 0) {
        //sets stdout output to point to opened file id
        dup2(fd, STDOUT_FILENO);
        close(fd);

        // This code block will be executed by the child process
        //if access is found returns location of executable
        char* Loc=check_access(path,temp[0],pathSize);
        if(strcmp(Loc,"Empty") != 0){
            execv(Loc,temp);
        }
    }
    else if (child_pid > 0) {
        // This code block will be executed by the parent process
        wait(NULL);  // Wait for the child process to finish
        close(fd);
    }
    return 0;
}

int multiCmdExec(char **args, int count, char ***path, int *pathSize){

    pid_t child_pids[count];

    for(int i = 0; i < count; i++){
        int tempLen = strlen(args[i]);
        tempLen+=1; //account for missing null terminator

        int tempCount = 0;
        bool multiCheck = false;
        bool redirectCheck = false;
        char *file  = NULL;
        char **temp = preProcess(args[i],tempLen,&tempCount,&multiCheck, &redirectCheck, &file);

        pid_t child_pid = fork();
        if(child_pid == -1){
            char error_message[30] = "An error has occurred\n";
            write(STDERR_FILENO, error_message, strlen(error_message));
            exit(1);
        }

        if(child_pid == 0){
            if(redirectCheck == true && count != -1){
                redirectCmdExec(temp,file,count, *path, *pathSize);
            }
            else if(count == -1){
                char error_message[30] = "An error has occurred\n";
                write(STDERR_FILENO, error_message, strlen(error_message));
            }
            else{
                int result = executeCmd(temp, *&path, *&pathSize, count);
                //exit executed
                if(result == 1){
                    exit(1);
                }
                else{
                    printPath(*path,*pathSize);
                }

            }
        }
        else{
            child_pids[i] = child_pid;
        }

    }

    int status;
    for (int i = 0; i < count; i++) {
        waitpid(child_pids[i], &status, 0);
    }

    return 0;
}


void interactive(){
    char **path = NULL;//starting path
    int pathSize=0; //nubmer of paths stored
    path = initilize_path(&pathSize);

    while(1){
        char *line = NULL;//contains the entire line entered || stores individual tokens of line
        size_t len = 0; //Size of the line
        int count = 0;
        bool multiCheck = false;
        bool redirectCheck = false;
        char *file = NULL;
        char** args;

        printf("wish> ");
        // Use getline to read from stdin
        //and split line into tokens using strsep
        getline(&line, &len, stdin);

        int Length = strlen(line);
        //printf("%s\n", );
        args = preProcess(line, Length, &count, &multiCheck,&redirectCheck,&file);

        //printf("%d:%d\n", multiCheck,count);
        if(multiCheck == true && count != -1){
            int result = multiCmdExec(args, count, &path, &pathSize);
            if(result == 1){
                free(line);
                exit(0);
            }
        }
        else if(redirectCheck == true && count != -1){
            redirectCmdExec(args,file,count, path, pathSize);
        }
        else if(count == -1){
            char error_message[30] = "An error has occurred\n";
            write(STDERR_FILENO, error_message, strlen(error_message));
        }
        else{
            int result = executeCmd(args, &path, &pathSize, count);
            //exit executed
            if(result == 1){
                free(line);
                exit(0);
            }
            else{
                printPath(path,pathSize);
            }

        }
        free(line);
        free(args);
    }
    exit(0);
}


int main(int argc, char*argv[]){

    if(argc == 1){
        interactive();
    }
    return 0;
}
