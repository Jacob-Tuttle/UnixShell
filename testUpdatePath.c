#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
// Function that takes a pointer to an int and modifies the value it points to

char** update_path(char** currentPaths, int *pathSize, char *add) {
    char** newPaths = (char**)malloc((*pathSize + 1) * sizeof(char*));

    for (int i = 0; i < *pathSize; i++) {
        newPaths[i] = (char*)malloc(strlen(currentPaths[i]) + 1);
        strcpy(newPaths[i], currentPaths[i]);
    }
    (*pathSize)++;
    newPaths[*pathSize] = (char*)malloc(strlen(add) + 1);
    strcpy(newPaths[*pathSize], add);
    free(currentPaths);
    return newPaths;
}

char** initilize_path(){
    char** path = (char**)malloc(1 * sizeof(char*));
    path[0] = (char*)malloc(strlen("/bin/") + 1);
    path[0] = "/bin/";
    return path;
}

int main() {
    char **path = NULL;
    int pathSize;

    char *tokens[] = {"path", "home/jacob/C/"};

    path = update_path(path,&pathSize, tokens[1]);
    printf("%d", pathSize);
    printf("%s", path[0]);
    printf("%s", path[1]);

//    for(int m = 0; m<pathSize;m++){
//        printf("%s/n",path[m]);
//    }

    return 0;
}
