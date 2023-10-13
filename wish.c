int main(int argc, char *argv[]){
  while(1){
    char *line = NULL;
    size_t len = 0;

    printf("wish> ");

    // Use getline to read from stdin
    getline(&line, &len, stdin);

    // Remove the trailing newline character
    line[strcspn(line, "\n")] = '\0';
    if(strcmp(line,"exit") == 0){
        // Free dynamically allocated memory
        free(line);
        exit(0);

    }
  }
  return 0;
}
