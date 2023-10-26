#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

/*
Takes a string and break it up into its individual parts
based on a given delimiter or delimiters.
Return: An array of strings refrenced by a pointer is
returned, also note that the paramter argNum is passed as
a reference and is modified directly by this function
*/
char** tokenize(char *line,char *delims, int *argNum){

    char *token = NULL; //Will store individual parts as the line
                        //is broken up by the delimiters
    int Size = 1; //Used to initialize the array to store token strings
    *argNum = 0; //Makes sure argNum is definied before using
                //as a tally of tokens read
    char** args;
    args = (char**)malloc(Size * sizeof(char*));

    //Creates a copy of the strings so
    //the one passed in the paramter is not
    //modified directly by strsep
    char *line_copy = strdup(line);

    while((token = strsep(&line_copy, delims)) != NULL){
        //Doesn't store if delimiter is being read
        //delimiters are replaced by null termintor by strsep
        if(*token != '\0'){
            if(*argNum >= Size){
                Size +=1;
                args = (char**)realloc(args, Size * sizeof(char*));
                args[*argNum] = (char*)malloc(strlen(token) + 1);
                strcpy(args[*argNum],token);
                *argNum+=1;

            }
            else if(*argNum == 0){
                args[*argNum] = (char*)malloc(strlen(token) + 1);
                strcpy(args[*argNum],token);
                *argNum+=1;
            }
        }
    }
    free(line_copy);
    return args;
}

/*
Simple function to check if a string contains no characters
Used for input checking to countinue aceepting inputs
when the user types nothing into the shell prompt
*/
bool contains_only_whitespace(char *line) {
    while (*line) {
        if (*line != ' ' && *line != '\t' && *line != '\n') {
            return false; // Non-whitespace character found
        }
        line++;
    }
    return true; // All characters are whitespace
}

/*
Will process a given cmd and arg combination or sequence of arguments

if the entered cmd is either a unix based cmd(e.g. ls, cat) or a shell cmd
it will return an array of strings containing the cmd and args seperated out

if a redirect cmd is sent it will process it and return the cmd and args
and it will also set file in the paramters to the file to write to

if its a multi cmd sent it will return an array of strings containing intact
cmd and argument strings. The multi cmd function will send individual parts
back to this function to be processed into one of the cases above
*/
char** preProcess(char *line, int lineLen, int *argNum,
                bool *multiCheck, bool *redirectCheck, char **file){

    int numRedirect = 0; //tallys number of redirects read before a &
    *multiCheck = false; //sets flag that a multi cmd is being read
    *redirectCheck = false; //sets flag that a redirect is being read

    //check for multiple redirect and multicmd commands
    //lineLen -1 to account for null terminator
    //used for consecutive > and &
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
        exit(0);
        return NULL;
    }

    for(int i = 1; i < lineLen; i++){
        if(cur == '>' || cur == '&'){
            //sets flags and resets num of redirect
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
            //multiple redirects have been read before a multi
            //had been reached
            if(numRedirect > 1 && multiReached == false){
                *argNum = -1;
                return NULL; //error multiple > before &
            }
            //error for consecutive > and &
            else if((prev == '>' && cur == '>')
                    || (prev == '>' && cur == '&')
                    || (prev == '&' && cur == '>')
                    || (prev == '&' && cur == '&')){

                *argNum = -1;
                return NULL; //error consecutive >
            }
        }
        //only update previous after a new character has been read
        //other than space, tab, or newline
        if(cur != ' ' && cur != '\t' && cur != '\n'){
            prev = cur;
            cur = line[i];
        }
        else{
            cur = line[i];
        }
    }
    //if no & had been read in the line and a > had
    //it is a redirect cmd
    if(*redirectCheck == true && *multiCheck == false){
        char *redDelim = ">\n"; //Used for strsep to break into cmd arg pair and a file
        char *delim = " \t\n"; //Used for strsep to break line into tokens
        int count = 0; //Number of tokens
        char **args; // will store the seperated arg and file
                    // for further processing

        //should be size of 2 where the first
        //elements is the cmd and args and the second is a file
        args = tokenize(line,redDelim,&count);
        if(count < 2){
            *argNum = -1;
            return NULL;
        }

        //locate space for the string of files or file
        char *fileArgs = (char*)malloc(strlen(args[1]) * sizeof(char));
        fileArgs = args[1];
        int numFiles = 0;
        tokenize(fileArgs,delim,&numFiles);
        //multiple files present error
        if(numFiles>1){
            *argNum = -1;
            return NULL;
        }
        //breaks first string into cmd and args
        char ** cmdArgs = tokenize(args[0], delim, &count);
        int tempNum = 0;
        //gets the file removing whitespace
        char** destFile = tokenize(args[1],delim,&tempNum);
        *file = destFile[0]; //Set file for return
        *argNum = count; //number of args including cmd
        free(fileArgs); //done checking number of files so free memory
        return cmdArgs; //return arg and cmd array
    }
    else if(*multiCheck == true){
        char *multiDelim = "&\n";
        int count = 0; //Number of tokens
        char **temp; // will store the seperated arg and file

        //Only remove & the white space will be removed when the individual
        //request are processed in the multicmd function
        temp = tokenize(line,multiDelim,&count);
        *argNum = count;
        return temp;
    }
    //normal cmd request either unix based or built in
    else{
        int count = 0;
        char *delim = " \t\n"; //Used for strsep to break line into tokens
        char **temp = tokenize(line, delim, &count);
        *argNum = count;
        return temp;
    }
    //Error if not fitting into any case above
    //dont think I even need this but just incase
    *argNum = -1;
    return NULL;
}


/*
Allocates memory for a new string contained in add to be added
to a new list of paths that will be returned.
*/
char** update_path(char** currentPaths, int *pathSize, char *add) {

    //Creates a new path array of strings which will hold one
    //additional string contained in add
    char** newPaths = (char**)malloc((*pathSize + 1) * sizeof(char*));



    for (int i = 0; i < *pathSize; i++) {
        newPaths[i] = (char*)malloc(strlen(currentPaths[i]) + 1);
        strcpy(newPaths[i], currentPaths[i]);
    }

    //free memory allocated for old path array
    for (int j = 0; j < *pathSize; j++) {
        free(currentPaths[j]);
    }
    free(currentPaths);

    //allocate space for a string which is the length of add
    newPaths[*pathSize] = (char*)malloc(strlen(add)+1);
    //copy the new path into the array
    strcpy(newPaths[*pathSize], add);
    //increase pathSize for newly added path
    (*pathSize)++;

    //return the new path array
    return newPaths;
}

/*
frees all memory associated with current path array
*/
char** clearPath(char **path, int *pathSize){
    for(int i = 0; i<*pathSize; i++){
        free(path[i]);
    }
    free(path);
    //update path to be size of zero
    *pathSize = 0;
    //return null / new path will point to null
    return NULL;
}

/*
Used on program start to initilize path
with /bin and update size of path array
*/
char** initilize_path(int *Size){
    char** path = (char**)malloc(1 * sizeof(char*));
    path[0] = (char*)malloc(strlen("/bin") + 1);
    strcpy(path[0],"/bin");
    *Size+=1;
    return path;
}

/*
returns whether or not the cmd being read is
a built in cmd or not
*/
bool check_Build_In(char *cmd){
    char *builtin[3] = {"exit","cd","path"};
    for(int i = 0; i < 3;i++){
        if(strcmp(cmd,builtin[i])==0){
            return true;
        }
    }
    return false;
}

/*
Receives a cmd and a list of paths then
check to see if a ( path/cmd ) file can be
found
*/
char* check_access(char **path, char *cmd, int pathSize) {

    for (int i = 0; i < pathSize; i++) {
        // Calculate the length of the resulting string
        size_t len = strlen(path[i]) + strlen(cmd) + 2;  // +2 for '/' and null terminator

        // Allocate memory for the new string
        char *New = (char *)malloc(len);

        if (New != NULL) {
            // Construct the full path by concatenating path[i] and cmd
            strcpy(New, path[i]);
            strcat(New, "/");
            strcat(New, cmd);
            // Check if the constructed path is executable
            if (access(New, X_OK) == 0) {
                return New;
            }
            //if cmd was not found free
            //current allocated memory
            //for next ( path/cmd ) check
            free(New);
        }
    }
    //If no cmd can be found with the
    //supplied paths then return empty(cmd not found)
    return "Empty";
}

/*
Executes all cmds either build in, unix based, or a redirect of a unix cmd
pid parameter is not used and can be removed
*/
int executeCmd(char **args,char ***path, int *pathSize, int count, bool multiFlag, pid_t pid){

    //none built in
    if(check_Build_In(args[0]) == false){
        //wanting to execute multiple cmds
        //currently it doenst check for built in cmds
        //being read. It should not process built in cmds
        //in multi cmd calls. Behavior exibited hangs after
        //processing an actual cmd until user input usually enter
        if(multiFlag == true){
            char* Loc=check_access(*path,args[0],*pathSize);
            execv(Loc,args);
        }
        //executes all unix based cmds
        else{
            pid_t child_pid = fork();
            if (child_pid == 0) {
                // This code block will be executed by the child process
                //if access is found returns location of executable
                char* Loc=check_access(*path,args[0],*pathSize);
                //if location is found run cmd
                if(strcmp(Loc,"Empty") != 0){
                    //if error while executing exit rc:0
                    if(execv(Loc,args) == -1){
                        exit(0);
                    }
                }
                //error location of cmd not found
                else{
                    char error_message[30] = "An error has occurred\n";
                    write(STDERR_FILENO, error_message, strlen(error_message));
                    exit(0);
                }
            }
            //wait for child process to finish before continuing in main process
            else if (child_pid > 0) {
                // This code block will be executed by the parent process
                wait(NULL);  // Wait for the child process to finish
            }
        }
    }
    //run built in cmds
    else if(check_Build_In(args[0]) == true){
        //if cmd is exit
        if(strcmp(args[0],"exit") == 0){
            //number of arguments during exit read
            //if any arguments passed error
            if(count > 1){
                char error_message[30] = "An error has occurred\n";
                write(STDERR_FILENO, error_message, strlen(error_message));
                exit(0);
            }
            return 1;
        }
        //change directory cmd
        else if(strcmp(args[0],"cd") == 0){
            //if only a directory path and cd
            //are present on line execute cd cmd
            if(count == 2){
                chdir(args[1]);
            }
            //any other amount of arguments is an error
            else{
                char error_message[30] = "An error has occurred\n";
                write(STDERR_FILENO, error_message, strlen(error_message));
                exit(0);
            }
        }
        //executes path cmd
        else if(strcmp(args[0],"path") == 0){
            //will update path and pathSize(number of paths stored)
            if(count != 1){
                //adds all paths entered in path array
                for(int i = 1; i <count; i++){
                    *path = update_path(*path,*&pathSize, args[i]);
                }
            }
            //will either clear entire path
            //or or singular path
            else if(count == 1){
                if(*pathSize == 1){
                    //free memory of path
                    free(path[0]);
                    free(path);
                    //returns emtpy path
                    *path = NULL;
                    *pathSize = 0;
                }
                //clears path
                else{
                    *path = clearPath(*path,*&pathSize);
                }
            }
        }
    }
    //if something other than a built in cmd or unix cmd
    //is sent to execute error
    else{
        char error_message[30] = "An error has occurred\n";
        write(STDERR_FILENO, error_message, strlen(error_message));
    }

    return 0;
}

/*
Executes redirect runs unix cmd in function and writes it
to a file contained in parameter file
*/
int redirectCmdExec(char **args, char *file, int count, char **path, int pathSize){
    char **temp = (char **)malloc((count + 1) * sizeof(char*)); // Allocate memory for arg

    //if memory allocation fails
    if (temp == NULL) {
        exit(1);
    }

    //grab id of now open file
    int fd = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0644);

    //file did not open
    if (fd == -1) {

        return 1;
    }

    //adds a null character to the end of the arguments array
    for (int j = 0; j < count; j++) {
        temp[j] = (char *)malloc(strlen(args[j]) + 1);
        strcpy(temp[j], args[j]);
    }
    temp[count] = NULL;

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
    //frees allocated memory in temp after
    //process is finished
    for(int k = 0; k <(count+1);k++){
        free(temp[k]);
    }
    free(temp);

    return 0;
}

/*
Executes multiple unix cmds or other privided programs in
parallel with one another and wait for all to finish before
releasing control to the user or processing the next line from file
*/
int multiCmdExec(char **args, int count, char ***path, int *pathSize){

    pid_t child_pids[count]; //creats an array of process ids will be used
                            //by main process to check children have ended
    for(int i = 0; i < count; i++){
        int tempLen = strlen(args[i]);
        tempLen+=1; //account for missing null terminator

        int tempCount = 0; //number of arguments in individual cmd args pair
        bool multiCheck = false; //needed for preProcess call
        bool redirectCheck = false; //needed for preProcess call
        char *file  = NULL; //store file if redirect was read
        //array of cmd args pair
        char **temp = preProcess(args[i],tempLen,&tempCount,&multiCheck, &redirectCheck, &file);
        bool multiFlag = true; //Currently executing multi

        char **extend = (char **)malloc((tempCount + 1) * sizeof(char*)); // Allocate memory for arg

        //if allocation fails
        if (extend == NULL) {
            exit(1);
        }

        //adds null to the end of cmd
        for (int j = 0; j < tempCount; j++) {
            extend[j] = (char *)malloc(strlen(temp[j]) + 1);
            strcpy(extend[j], temp[j]);
        }
        extend[tempCount] = NULL;

        pid_t child_pid = fork();
        if(child_pid == -1){
            char error_message[30] = "An error has occurred\n";
            write(STDERR_FILENO, error_message, strlen(error_message));
            exit(1);
        }

        if(child_pid == 0){
            //executes redirect
            if(redirectCheck == true && count != -1){
                int fd = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0644);

                if (fd == -1) {
                    return 1;
                }
                //sets stdout output to point to opened file id
                dup2(fd, STDOUT_FILENO);
                close(fd);

                // This code block will be executed by the child process
                //if access is found returns location of executable
                char* Loc=check_access(*path,extend[0],*pathSize);
                if(strcmp(Loc,"Empty") != 0){
                    execv(Loc,extend);
                }
            }
            else if(count == -1){
                char error_message[30] = "An error has occurred\n";
                write(STDERR_FILENO, error_message, strlen(error_message));
            }
            else{
                int result = executeCmd(extend, *&path, *&pathSize, tempCount, multiFlag, child_pid);
                //exit executed
                if(result == 1){
                    exit(1);
                }
            }
        }
        //adds running child process to list of active child process
        else if(child_pid > 0){
            child_pids[i] = child_pid;
        }


    }

    //wait for all process to finish
    int status;
    for (int i = 0; i < count; i++) {
        waitpid(child_pids[i], &status, 0);

    }

    return 0;
}

/*
runs loop for interactive mode of the shell
*/
void interactive(){
    char **path = NULL;//starting path
    int pathSize=0; //nubmer of paths stored
    path = initilize_path(&pathSize);

    while(1){
        char *line = NULL;//contains the entire line entered || stores individual tokens of line
        size_t len = 0; //Size of the line
        int count = 0; //number of arguments
        bool multiCheck = false;
        bool redirectCheck = false;
        char *file = NULL; //file of redirect cmd
        char** args; //array of cmd and args pair
        char *Empty = "not"; //used to check if line is empty

        printf("wish> ");
        // Use getline to read from stdin
        //and split line into tokens using strsep
        getline(&line, &len, stdin);
        if(contains_only_whitespace(line)){
            Empty = "empty";
        }
        //restart at the top if line is empty
        if(strcmp(Empty,"empty") != 0){
            //length of stuff enter into prompt
            int Length = strlen(line);
            //make sure input is correct and tokenize it
            args = preProcess(line, Length, &count, &multiCheck,&redirectCheck,&file);
            //if the number of thing processed is not 0
            if(count != 0){
                //if multi cmd was found to be entered
                if(multiCheck == true && count != -1){
                    int result = multiCmdExec(args, count, &path, &pathSize);
                    if(result == 1){
                        free(line);
                        exit(0);
                    }
                }
                //if redirect was read
                else if(redirectCheck == true && count != -1){
                    redirectCmdExec(args,file,count, path, pathSize);
                }
                //error found in preprocess
                else if(count == -1){
                    char error_message[30] = "An error has occurred\n";
                    write(STDERR_FILENO, error_message, strlen(error_message));
                }
                //execute normal unix cmds or built in cmds
                else{
                    int result = executeCmd(args, &path, &pathSize, count, false, -1);
                    //exit executed
                    if(result == 1){
                        free(line);
                        exit(0);
                    }

                }

            }
        }
    }
    exit(0);
}

/*
run batch mode where cmd are exectued line by line
from a file passed on program initial call
*/
void batchRead(char *filename){

    char **path = NULL;//starting path
    int pathSize=0; //nubmer of paths stored
    path = initilize_path(&pathSize);

    //open file in read binary mode
    FILE *FILE = fopen(filename, "rb");

    //if file sent to read from is not found
    //error rc:1
    if (FILE == NULL) {
        char error_message[30] = "An error has occurred\n";
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(1);
    }

    //initilize values to be used for getline
    char *line = NULL;
    size_t len = 0;

    //read till end of file is reached
    while (getline(&line, &len, FILE) != -1) {

        //check for only whitespace on cmd line
        //skip to next line if true
        char *Empty = "not";
        if(contains_only_whitespace(line)){
            Empty = "empty";
        }
        //skips if line is empty
        if(strcmp(Empty, "empty") != 0){

            int count = 0; //number of cmd and arguments entered
            bool multiCheck = false;
            bool redirectCheck = false;
            char *file = NULL;
            char** args;

            //length of cmd entered
            int Length = strlen(line);
            //validate entered cmd and tokenizes if it is valid
            args = preProcess(line, Length, &count, &multiCheck,&redirectCheck,&file);

            //executes the corresponding process whether redirect, multi, or single cmd
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
                int result = executeCmd(args, &path, &pathSize, count, false, -1);
                //exit executed
                if(result == 1){
                    free(line);
                    exit(0);
                }

            }
        }
    }
    fclose(FILE);
}

int main(int argc, char*argv[]){

    if(argc == 1){
        interactive();
    }
    else if(argc == 2){
        batchRead(argv[1]);
    }
    else{
        char error_message[30] = "An error has occurred\n";
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(1);
    }

    return 0;
}
