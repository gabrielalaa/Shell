#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <pwd.h>
#include <limits.h>
#include<signal.h>


#define MAX_LENGTH 100 // max nr. of characters
#define MAX_ARGS 10 // max nr. of commands 


typedef struct {
    pid_t pid; // The process ID of a background process
    char *command;  // The command executed for the background process
} BackgroundProcess;

BackgroundProcess backgroundProcesses[MAX_ARGS]; // Array to hold background processes
int backgroundProcessCount = 0;  // Counter to keep track of the nr. of active background processes


// Function to print the shell's welcome message 
void init_shell() {
    printf("\n\n*** Welcome to IMCSH ***\n");
    printf("Type 'quit' to exit the shell.\n");
    printf("Enjoy!\n\n");

    // Add the current username and hostname 
    char username[256];
    char hostname[256];

    if (getlogin_r(username, sizeof(username)) != 0) {
        strcpy(username, "unknown");  // default
    }

    if (gethostname(hostname, sizeof(hostname)) != 0) {
        strcpy(hostname, "unknown");  // default
    }

   printf("Logged in as: %s@%s\n", username, hostname);
}


// Function to display the shell prompt with username and hostname
void display_prompt() {
    char username[256];
    char hostname[256];

    // retrieve the username 
    if (getlogin_r(username, sizeof(username)) != 0) {
        strcpy(username, "unknown");  // If username retrieval fails, fallback
    }

    // retrieve the hostname
    if (gethostname(hostname, sizeof(hostname)) != 0) {
        strcpy(hostname, "unknown");  // If hostname retrieval fails, fallback
    }

    printf("%s@%s> ", username, hostname);
}


// Function to read user input
void takeInput(char* input) {
    // Use dynamic prompt from display_prompt
    display_prompt(); 
    // Read up to MAX_LENGTH characters 
    if (fgets(input, MAX_LENGTH, stdin) == NULL) {
        printf("Error reading input.\n");
        // If an error occurs, set input to empty string
        input[0] = '\0';
    }
    else {
        // Remove trailing newline character
        input[strcspn(input, "\n")] = '\0';
    }
}


// Function to parse the command string into arguments and identify special cases 
int parseCommand(char *input, char **args, int *background, char **outputFile) {
    int i = 0;
    // Initialize background flag
    *background = 0;  
    // Initialize output file pointer
    *outputFile = NULL;
    
    // Tokenize the input
    char *token = strtok(input, " ");
    if (token != NULL && strcmp(token, "exec") == 0) { // Check for "exec"
        token = strtok(NULL, " "); // Get the actual command
    }

    // Process each token 
    while (token != NULL) {
        if (strcmp(token, "&") == 0) {
            *background = 1; // Bacground execution
        }
        else if (strcmp(token, ">") == 0) {
            // Get the next token for output file
            token = strtok(NULL, " "); 
            if (token == NULL) {
                printf("Error: Missing output file after '>'.\n");
                return -1;
            }
            // Set output file 
            *outputFile = token;
        }
        else {
            // Add token to args array
            args[i++] = token;
        }
        token = strtok(NULL, " ");
    }
    args[i] = NULL; // NULL terminate the array
    return i;  // Return the nr. of arguments
}


// Function to execute commands
void executeCommand(char **args, int background, char *outputFile) {
    pid_t pid = fork();

    if (pid < 0) {
        printf("Failed to fork process.\n");
    } else if (pid == 0) {
        // Child process: handle output redirection
        if (outputFile) {
            int fd = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) {
                printf("Error: Could not open file for redirection.\n");
                exit(1);
            }
            dup2(fd, STDOUT_FILENO); // Redirect stdout to file
            close(fd);
        }

        // Execute the command
        if (execvp(args[0], args) < 0) {
            printf("Command not found: %s\n", args[0]);
            exit(1);
        }
    } else {
        if (!background) {
            // Parent process: wait for the child process to finish
            waitpid(pid, NULL, 0);
        } else {
            // Parent process: handle background process
            printf("Process running in background (PID: %d)\n", pid);

            // Add the background process to the array
            if (backgroundProcessCount < MAX_ARGS) {
                backgroundProcesses[backgroundProcessCount].pid = pid;
                backgroundProcesses[backgroundProcessCount].command = strdup(args[0]); // Store the command
                backgroundProcessCount++;
            } else {
                printf("Error: Too many background processes.\n");
            }
        }

        // Check for any completed background processes
        for (int i = 0; i < backgroundProcessCount; i++) {
            int status;
            if (waitpid(backgroundProcesses[i].pid, &status, WNOHANG) > 0) {
                printf("Background process (PID: %d, Command: %s) has finished.\n",
                       backgroundProcesses[i].pid, backgroundProcesses[i].command);
                free(backgroundProcesses[i].command); // Free allocated memory
                // Shift remaining entries in the array
                for (int j = i; j < backgroundProcessCount - 1; j++) {
                    backgroundProcesses[j] = backgroundProcesses[j + 1];
                }
                // decrease the count
                backgroundProcessCount--; 
                // adjust index for shifted array
                i--; 
            }
        }
    }
}


// Main function to handle the shell's loop
int main() {
    char input[MAX_LENGTH];
    char *args[MAX_ARGS];
    char *outputFile;
    int background;

    init_shell();

    while (1) {
        takeInput(input);

        // Handle "quit" command
        if (strcmp(input, "quit") == 0) {
            if (backgroundProcessCount > 0) {
                char confirm;
                printf("The following processes are running, are you sure you want to quit? [Y/n]\n");
                for (int i = 0; i < backgroundProcessCount; i++) {
                    printf("PID: %d - Command: %s\n", backgroundProcesses[i].pid, backgroundProcesses[i].command);
                }
                scanf(" %c", &confirm);
                if (confirm == 'Y' || confirm == 'y') {
                    for (int i = 0; i < backgroundProcessCount; i++) {
                        kill(backgroundProcesses[i].pid, SIGTERM);
                        free(backgroundProcesses[i].command);
                    }
                    printf("Exiting IMCSH...\n");
                    break;
                } else if (confirm == 'N' || confirm == 'n') {
                    printf("Quit canceled. Returning to shell.\n");
                    continue;
                } else {
                    printf("Invalid input. Please type Y or n.\n");
                    continue;
                }
            } else {
                printf("Exiting IMCSH...\n");
                break;
            }
        }

        // Handle "globalusage" command
        if (strcmp(input, "globalusage") == 0) {
            printf("IMCSH Version 1.1 created by Alexandra and Gabriela\n");
            continue;
        }

        // Handle "bglist" command
        if (strcmp(input, "bglist") == 0) {
            if (backgroundProcessCount == 0) {
                printf("No background processes are currently running.\n");
            } else {
                printf("Background Processes:\n");
                for (int i = 0; i < backgroundProcessCount; i++) {
                    printf("PID: %d - Command: %s\n", backgroundProcesses[i].pid, backgroundProcesses[i].command);
                }
            }
            continue;
        }

        int argCount = parseCommand(input, args, &background, &outputFile);

        if (argCount > 0) {
            executeCommand(args, background, outputFile);
        }
    }

    return 0;
}