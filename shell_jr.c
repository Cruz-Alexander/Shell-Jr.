#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <limits.h>
#include <errno.h>
#include <sysexits.h>
#include <sys/stat.h>

#define MAX_INPUT_SIZE 1024
#define MAX_DIR_SIZE PATH_MAX
#define MAX_DIR_STACK 16

void print_prompt() {
    printf("shell_jr: ");
    fflush(stdout);
}

void process_cd(char *input) {
    char directory[MAX_INPUT_SIZE];
    if (sscanf(input, "%*s %s", directory) == 1) {
        if (chdir(directory) != 0) {
            fprintf(stdout, "Cannot change to directory: %s\n", strerror(errno));
        }
    } else {
        printf("Invalid command format for cd\n");
    }
}

void process_pushd(char *input, char *stack[], int *stack_size) {
    char directory[MAX_INPUT_SIZE];
    if (sscanf(input, "%*s %s", directory) == 1) {
        char *absolute_path = realpath(directory, NULL);
        if (absolute_path != NULL) {
            if (*stack_size < MAX_DIR_STACK) {
                stack[(*stack_size)++] = strdup(absolute_path);
            } else {
                printf("Directory stack is full\n");
            }
            free(absolute_path);
        } else {
            perror("Invalid directory for pushd");
        }
    } else {
        printf("Invalid command format for pushd\n");
    }
}

void process_dirs(char *stack[], int stack_size) {
    for (int i = 0; i < stack_size; ++i) {
        printf("%s\n", stack[i]);
    }
}

void process_popd(char *stack[], int *stack_size) {
    if (*stack_size > 0) {
        char *directory = stack[--(*stack_size)];
        if (chdir(directory) != 0) {
            perror("Cannot change to directory");
        }
        free(stack[*stack_size]);
    } else {
        printf("Directory stack is empty\n");
    }
}

void execute_command(char *command, char *input) {
    pid_t pid = fork();

    if (pid == 0) {
        // Child process
        char *argv[MAX_INPUT_SIZE];
        char *token = strtok(input, " ");
        int i = 0;

        while (token != NULL && i < MAX_INPUT_SIZE - 1) {
            argv[i++] = token;
            token = strtok(NULL, " ");
        }

        argv[i] = NULL; // Null-terminate the array

        // Execute the command
        if (execvp(command, argv) == -1) {
            perror("Failed to execute");
            exit(EX_OSERR);
        }
    } else if (pid > 0) {
        // Parent process
        wait(NULL);
    } else {
        perror("Failed to fork");
        exit(EX_OSERR);
    }
}

// Helper function to construct an absolute path for the file
char *construct_absolute_path(char *directory, char *file) {
    // Adjust the buffer size according to your needs
    char *absolute_path = malloc(MAX_INPUT_SIZE);
    if (absolute_path == NULL) {
        perror("Memory allocation error");
        exit(EX_OSERR);
    }

    snprintf(absolute_path, MAX_INPUT_SIZE, "%s/%s", directory, file);
    return absolute_path;
}

int main() {
    char input[MAX_INPUT_SIZE];
    char *dir_stack[MAX_DIR_STACK];
    int dir_stack_size = 0;

    while (1) {
        print_prompt();

        if (fgets(input, sizeof(input), stdin) == NULL) {
            // End of file (Ctrl-D)
            printf("\nSee you\n");
            break;
        }

        char command[MAX_INPUT_SIZE];
        if (sscanf(input, "%s", command) == 1) {
            if (strcmp(command, "exit") == 0 || strcmp(command, "goodbye") == 0) {
                printf("See you\n");
                break;
            } else if (strcmp(command, "cat") == 0) {
                char path[MAX_INPUT_SIZE];

                // Process the cat command
                if (sscanf(input, "%*s %s", path) == 1) {
                    execute_command(command, path);
                } else if (strcmp(command, "cat") != 0) {
                    printf("Invalid command format for %s\n", command);
                }
            } else {
                execute_command(command, input);
            }
        } else if (strcmp(command, "cd") == 0) {
            process_cd(input);
        } else if (strcmp(command, "pushd") == 0) {
            process_pushd(input, dir_stack, &dir_stack_size);
        } else if (strcmp(command, "dirs") == 0) {
            process_dirs(dir_stack, dir_stack_size);
        } else if (strcmp(command, "popd") == 0) {
            process_popd(dir_stack, &dir_stack_size);
        } else {
            printf("Unknown command: %s\n", command);
            exit(EX_OSERR);
        }
    }
    return 0;
}
