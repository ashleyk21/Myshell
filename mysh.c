#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <regex.h>

#define MAX_COMMAND_LENGTH 1024
#define MAX_ARGUMENTS 32
#define MAX_TOKENS 64
#define MAX_PATH_LENGTH 4096
#define INPUT_END 1
#define OUTPUT_END 0

// A type definition "Command" which contains the command
typedef struct {
	// args[0] represents the actual command
	// args[1-4] represent the arguments (max 4 args)
    // args[5] always NULL
    char *args[6];

    int isBackground;
    int amountofArgs;
} Command;

int execute_command(char *tokens[]);

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

// Checks if wildcards are present (0 wildcards, -1 no wildcards)
int wildcardCheck(Command* ourCommand, char currentDir[]) {

	// If the * symbol is present in the command
    if (ourCommand->args[0][0] == '*') {
        printf("Cannot use wildcard in the command.\n");
        return -1;
    }

	// Loop that checks if each argument has the wildcard symbol and acts accordingly
    int i;
    for (i = 1; i <= ourCommand->amountofArgs; i++) {

		// If the argument contains a wildcard, the argument is substituded with all the possible combinations
        if (ourCommand->args[i][0] == '*') {
            char wildcard[strlen(ourCommand->args[i])];
            strcpy(wildcard, (ourCommand->args[i]) + 1); // +1 excludes wildcard (*)
            regex_t regex;
            regcomp(&regex, strcat(wildcard, "$"), 0); 

			// Open current directory and get all entities
            struct dirent *ent;
            DIR *dir = opendir(currentDir);
            char *arg;
			// Checks if there are more files to be processed for matching
            while ((ent = readdir(dir)) != NULL) {
				// If the entity filename matches the regular expression
                if (regexec(&regex, ent->d_name, 0, NULL, 0) == 0) {
                    arg = (char*) malloc(strlen(ent->d_name) + 1);
                    strcpy(arg, ent->d_name);
					// Substitutes the argument containing the wildcard with the new filename
                    ourCommand->args[i] = arg;
                    // We only substitute one filename in this loop and add the rest later, hence break
					break;
                }
            }

			// If no matching filename was found
            if (ent == NULL) {
                int j = i;
                while (j <= ourCommand->amountofArgs) {
                    // Remove the wildcard argument and move the rest of the arguments one position backwards
                    ourCommand->args[j] = ourCommand->args[j + 1];
                    j++;
                }
                (ourCommand->amountofArgs)--;
                i--;
                continue; // Next instruction
            }

			// Continue checking for matching filenames and store them in empty args[] slots (still max 4 args)
            while ((ent = readdir(dir)) != NULL) {
                if (regexec(&regex, ent->d_name, 0, NULL, 0) == 0) {
                    if (ourCommand->amountofArgs < 4) {
                        (ourCommand->amountofArgs)++;
                        arg = (char*) malloc(strlen(ent->d_name) + 1);
                        strcpy(arg, ent->d_name);
                        ourCommand->args[ourCommand->amountofArgs] = arg;
                        ourCommand->args[ourCommand->amountofArgs + 1] = NULL;
                    } else {
                        printf("Too many arguments given.(4 max)\n");
                        closedir(dir);
                        regfree(&regex);
                        return -1;
                    }
                }
            }
            closedir(dir);
            regfree(&regex);
        }
    }
    return 0;
}


void change_directory(char* path) {
    if (chdir(path) == -1) {
        perror("chdir() error");
    }
}

int execute_command(char** arguments) {
    pid_t pid;
    int fd[2];
    int status;

    pipe(fd); /* create a pipe */
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


int main(int argc, char *argv[]) {
    FILE *input_file;
    char line[MAX_COMMAND_LENGTH];
    char command[MAX_COMMAND_LENGTH];
    char *tokens[MAX_TOKENS];
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
            while (tokens[num_tokens] != NULL && num_tokens < MAX_TOKENS-1) {
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
            while (tokens[num_tokens] != NULL && num_tokens < MAX_TOKENS-1) {
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
