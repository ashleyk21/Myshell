#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>

#define MAX_COMMAND_LENGTH 1024
#define MAX_ARGUMENTS 32
#define MAX_PATH_LENGTH 256
#define INPUT_END 1
#define OUTPUT_END 0

char current_directory[MAX_PATH_LENGTH];


void get_current_directory() {
    if (getcwd(current_directory, MAX_PATH_LENGTH) == NULL) {
        perror("getcwd() error");
        exit(EXIT_FAILURE);
    }
}

void print_prompt() {
    printf("%s $ ", current_directory);
    fflush(stdout);
}

void parse_command(char* command, char** arguments) {
    char* token;
    int i = 0;

    token = strtok(command, " ");
    while (token != NULL && i < MAX_ARGUMENTS-1) {
        arguments[i] = token;
        i++;
        token = strtok(NULL, " ");
    }
    arguments[i] = NULL;
}

int execute_command(char** arguments) {
    pid_t pid;
    int fd[2];
    int status;

    pid = fork();
    if (pid == 0) {
        // child process
       dup2(fd[OUTPUT_END], STDIN_FILENO);
        if (execvp(arguments[0], arguments) == -1) {
            perror("execvp() error");
            exit(EXIT_FAILURE);
        }
    } else if (pid < 0) {
        perror("fork() error");
    } else {
        // parent process
        do {
            if (waitpid(pid, &status, WUNTRACED) == -1) {
                perror("waitpid() error");
            }
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
}

void change_directory(char* path) {
    if (chdir(path) == -1) {
        perror("chdir() error");
    }
}

int main(int argc, char *argv[]) {
    FILE *input_file;
    char line[MAX_COMMAND_LENGTH];
    char command[MAX_COMMAND_LENGTH];
    char *tokens[MAX_ARGUMENTS];
    int num_tokens;
    int status = 0;

    if (argc > 2) {
        fprintf(stderr, "Usage: %s [filename]\n", argv[0]);
        exit(1);
    }

    if (argc == 2) {
        input_file = fopen(argv[1], "r");
        if (input_file == NULL) {
            perror("fopen");
            exit(1);
        }
        while (fgets(line, MAX_COMMAND_LENGTH, input_file) != NULL) {
            num_tokens = 0;
            tokens[num_tokens] = strtok(line, " \t\n");
            while (tokens[num_tokens] != NULL && num_tokens < MAX_ARGUMENTS-1) {
                num_tokens++;
                tokens[num_tokens] = strtok(NULL, " \t\n");
            }
            tokens[num_tokens] = NULL;
            if (num_tokens > 0) {
                status = execute_command(tokens);
                if (status != 0) break;
            }
        }
        fclose(input_file);
    } else {
        printf("Welcome to my shell!\n");
        while (1) {
            print_prompt(status);
            if (fgets(line, MAX_COMMAND_LENGTH, stdin) == NULL) {
                printf("\n");
                break;
            }

               // remove newline character
        command[strcspn(command, "\n")] = '\0';

        // handle built-in commands
        if (strcmp(command, "exit") == 0) {
            break; // exit program
        } else if (strcmp(command, "cd") == 0) {
            change_directory(getenv("HOME")); // change to home directory
        }
            num_tokens = 0;
            tokens[num_tokens] = strtok(line, " \t\n");
            while (tokens[num_tokens] != NULL && num_tokens < MAX_ARGUMENTS-1) {
                num_tokens++;
                tokens[num_tokens] = strtok(NULL, " \t\n");
            }
            tokens[num_tokens] = NULL;
            if (num_tokens > 0) {
                status = execute_command(tokens);
                if (strcmp(tokens[0], "exit") == 0) break;
            }
        }
        printf("mysh: exiting\n");
    }

     return 0;
}
