// include paths
#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"

// numerical constants
#define MAX_WORDS 25
#define MAX_INPUT 256

// char constants
#define SPACE ' '
#define RIGHT_REDIR '>'
#define LEFT_REDIR '<'
#define PIPE '|'
#define SEMI ';'

// function prototypes
void collect_input(char *input);
int exit_shell(char *input);
int cd(char **cmd);
int fork_and_exec(char **cmd, int in_fd, int out_fd);
int redirect_find_next_cmd(char **cmd, int *in_fd, int *out_fd, char ***next_cmd);
int run_adv_cmd_recursively(char **cmd, int in_fd);
void run_adv_cmd(char **cmd);

// collects and cleans input
void collect_input(char *input)
{
    printf(">>> ");
    gets(input, MAX_INPUT);

    int len = (input != 0) ? strlen(input) : 0;
    if (len > 0 && input[len - 1] == '\n')
    {
        input[len - 1] = '\0';
    }
}

// exits shell with ternary operator when exit command run
int exit_shell(char *input)
{
    return (input && strcmp(input, "exit") == 0) ? (printf("exiting!\n"), 1) : 0;
}

// handles unique cd cmd
int cd(char **cmd)
{
    // Ensure at least two tokens, "cd" and the directory path
    if (cmd[0] && cmd[1] && strcmp(cmd[0], "cd") == 0)
    {
        // cmd[1] should be the directory path
        if (chdir(cmd[1]) < 0)
        {
            printf("cd: cannot change directory to %s\n", cmd[1]);
        }
        return 1;
    }
    return 0;
}


// creates forks and execs commands
int fork_and_exec(char **cmd, int in_fd, int out_fd)
{
    int process_id = fork();

    if (process_id == 0)
    { // Child process
        if (in_fd != 0)
        { // Replace standard input with in_fd
            close(0);
            dup(in_fd);
            close(in_fd);
        }

        if (out_fd != 1)
        { // Replace standard output with out_fd
            close(1);
            dup(out_fd);
            close(out_fd);
        }

        exec(cmd[0], cmd);
        printf("exec %s failed\n", cmd[0]);
        exit(1);
    }
    return process_id;
}

int redirect_find_next_cmd(char **cmd, int *in_fd, int *out_fd, char ***next_cmd)
{
    int i = 0;
    while (cmd[i] != 0)
    {
        switch (cmd[i][0])
        {
        case LEFT_REDIR:
            cmd[i] = 0; // Terminate cmd here
            *in_fd = open(cmd[i + 1], O_RDONLY);
            if (*in_fd < 0)
            {
                printf("Failed to open input file: %s\n", cmd[i + 1]);
                exit(1);
            }
            break;

        case RIGHT_REDIR:
            cmd[i] = 0;
            *out_fd = open(cmd[i + 1], O_CREATE | O_RDWR);
            if (*out_fd < 0)
            {
                printf("Failed to open output file: %s\n", cmd[i + 1]);
                exit(1);
            }
            break;
        case PIPE:
        {
            cmd[i] = 0;
            *next_cmd = cmd + i + 1; // Set pointer to the next cmd
            return 1;
        }
        }
        i++;
    }
    *next_cmd = (char **)0; // set to 0 ("NULL"), end of commands
    return 0;
}

// runs command recursively to handle multipipes
int run_adv_cmd_recursively(char **cmd, int in_fd)
{
    int out_fd = 1; // Default as std out
    int p[2];
    int process_id;
    char **next_cmd = (char **)0;

    if (!redirect_find_next_cmd(cmd, &in_fd, &out_fd, &next_cmd))
    {
        // run last command or the only command if no pipes
        process_id = fork_and_exec(cmd, in_fd, out_fd);
        if (in_fd != 0)
            close(in_fd);
        if (out_fd != 1)
            close(out_fd);
        return process_id;
    }
    else
    {
        pipe(p);
        fork_and_exec(cmd, in_fd, p[1]);
        close(p[1]); // Close write end of pipe in the parent
        if (in_fd != 0)
            close(in_fd); // Close  previous read end if it's not std input

        // recursive call to handle next command
        return run_adv_cmd_recursively(next_cmd, p[0]);
    }
}

// calls the run command and waits for child processes to finish
// TODO remove this function
void run_adv_cmd(char **cmd)
{
    // Start recursive pipes and redir
    int last_pid = run_adv_cmd_recursively(cmd, 0);

    while (wait(0) != last_pid)
    {
    }
}


char** tokenise(const char *input) {
    char **tokens = malloc(MAX_WORDS * sizeof(char *));
    if (!tokens) {
        printf("Failed to allocate memory for tokens.\n");
        exit(1);
    }

    char token[MAX_INPUT]; // buffer to hold tokens
    int n = 0, i = 0; // n: for tokens, i: for token buffer

    // tokenise input 
    while (*input && n < MAX_WORDS - 1) {
        // ignore spaces
        while (*input == ' ') {
            ++input;
        }

        // Reset index
        i = 0;

        // Process special characters
        if (*input == LEFT_REDIR || *input == RIGHT_REDIR || *input == PIPE) {
            // Single char token
            token[i++] = *input++;
        } else {
            // regular token
            while (*input && *input != SPACE && *input != LEFT_REDIR && *input != RIGHT_REDIR && *input != PIPE) {
                token[i++] = *input++;
            }
        }

        // null terminate the token
        token[i] = '\0';

        // allocate space for the new token
        // copy from the buffer
        tokens[n] = malloc((i + 1) * sizeof(char));
        if (!tokens[n]) {
            printf("Failed to allocate memory for a token.\n");
            exit(1);
        }
        for (int j = 0; j <= i; j++) {
            tokens[n][j] = token[j];
        }

        // increment token count
        n++;
    }

    // null terminate tokens array
    tokens[n] = 0;

    return tokens;
}

void free_mem(char **tokens) {
    for (int i = 0; tokens[i] != 0; i++) {
        free(tokens[i]);
    }
    free(tokens);
}

int main(int argc, char *argv[]) {
    char input[MAX_INPUT];

    // loop indefinitely until exit command used
    while (1) {
        // collect input and adds null termination
        collect_input(input);

        // tokenize input, handling spaces and special characters
        char **cmd = tokenise(input);

        // exits on "exit"
        if (exit_shell(cmd[0])) {
            free_mem(cmd);
            exit(0);
        }

        // cd functionality
        if (cd(cmd)) {
            free_mem(cmd);
            continue;
        }

        // run advanced commands, (redirects and piping)
        run_adv_cmd(cmd);

        // Free the memory allocated for cmd
        free_mem(cmd);
    }
    exit(0);
    return 0;
}