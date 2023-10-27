#include "kernel/types.h"
#include "user/user.h"
// #include "kernel/stat.h"
// #include "fcntl.h"

#define MAX_WORDS 25
#define MAX_INPUT 256

#define SPACE ' '
#define RIGHT_REDIR '>'
#define LEFT_REDIR '<'
#define PIPE '|'
#define SEMI ';'


// including "fcntl.h manually"
#define O_RDONLY 0x000
#define O_WRONLY 0x001
#define O_RDWR 0x002
#define O_CREATE 0x200

void collect_input(char *input){
    printf(">>> ");
    gets(input, MAX_INPUT);

    int len = strlen(input);
    if (len > 0 && input[len - 1] == '\n') {
        input[len - 1] = '\0';
    }
}

int exit_shell(char *input)
{
    if (strcmp(input, "exit") == 0)
    {
        printf("exiting!\n");
        return 1;
    }
    return 0;
}

int cd(char *input)
{
    if (input[0] == 'c' && input[1] == 'd' && input[2] == ' ')
    {
        // everything after "cd " (directory path)
        char *dir_path = input + 3;

        // Change directory
        if (chdir(dir_path) < 0)
        {
            printf("cd: cannot change directory to %s\n", dir_path);
        }
        return 1;
    }
    return 0;
}

char *remove_spaces(char *input)
{
    int len = strlen(input);
    char *result = (char *)malloc(len + 1); // malloc for new string
    int i = 0, j = 0;
    int word_started = 0;

    for (i = 0; i < len; i++)
    {
        if (input[i] != ' ')
        {
            word_started = 1;
            result[j++] = input[i];
        }
        else
        {
            if (word_started)
            {
                // Only one space between words
                word_started = 0;
                result[j++] = ' ';
            }
        }
    }

    // Remove trailing spaces
    if (j > 0 && result[j - 1] == ' ')
    {
        result[j - 1] = '\0';
    }
    else
    {
        result[j] = '\0';
    }
    return result;
}

char **split_string(const char *str)
{
    char **result = malloc(MAX_WORDS * sizeof(char *));
    if (!result)
    {
        printf("Failed to allocate memory");
        exit(1);
    }

    int start = 0;
    int wordCount = 0;
    int len = strlen(str);

    for (int end = 0; end <= len; end++)
    {
        if (str[end] == ' ' || str[end] == '\0')
        {
            int wordLength = end - start;
            result[wordCount] = malloc(wordLength + 1); // +1 for the null
            if (!result[wordCount])
            {
                printf("Failed to allocate memory for word");
                exit(1);
            }
            memcpy(result[wordCount], &str[start], wordLength);
            result[wordCount][wordLength] = '\0'; // Null terminate the string
            wordCount++;
            start = end + 1;
        }
    }

    result[wordCount] = 0; // Mark the end of the array of strings
    return result;
}

int run_adv_cmd(char **cmd)
{
    int i;
    char *input_file = 0;
    char *output_file = 0;

    // Loop through cmd to identify '<' and/or '>'
    for (i = 0; cmd[i] != 0; ++i)
    {
        if (strcmp(cmd[i], "<") == 0)
        {
            input_file = cmd[i + 1];
            cmd[i] = 0; // Nullify the '<' symbol
        }
        if (strcmp(cmd[i], ">") == 0)
        {
            output_file = cmd[i + 1];
            cmd[i] = 0; // Nullify the '>' symbol
        }
    }

    int pid = fork();
    if (pid == 0)
    { // Child process
        if (input_file)
        {
            close(0); // Close stdin
            if (open(input_file, O_RDONLY) < 0)
            { // Open the file for reading
                printf("Failed to open input file: %s\n", input_file);
                exit(1);
            }
        }
        if (output_file)
        {
            close(1);                                      // Close stdout then
            int fd = open(output_file, O_CREATE | O_RDWR); // Open the file for writing
            if (fd < 0)
            {
                printf("Failed to open output file: %s\n", output_file);
                exit(1);
            }
        }

        exec(cmd[0], cmd); // Execute the command
        printf("exec %s failed\n", cmd[0]);
        exit(1);
    }
    else if (pid > 0)
    { // Parent process
        wait(0);
    }
    else
    { // Fork failed
        printf("fork failed\n");
        return 1;
    }

    return 0;
}

int main(int argc, char *argv[])
{
    char input[MAX_INPUT];

    // loop indefinitly until exit command used
    while (1)
    {
        // collect input and adds null termination
        collect_input(input);

        // removes spaces from input
        char *new_input = remove_spaces(input);
        strcpy(input, new_input);
        free(new_input);

        // exits on "exit"
        if (exit_shell(input) == 1)
        {
            exit(0);
        };

        // cd functionality
        if (cd(input) == 1)
        {
            continue;
        }

        // splits strings by spaces (and tokens? TODO)
        char **cmd = split_string(input);
        // for(int i = 0; cmd[i] != 0; i++){
        //     printf("%s ", cmd[i]);
        // }

        // run advanced commands, (redir and piping)
        run_adv_cmd(cmd);

        // Free the memory allocated for cmd
        for (int i = 0; cmd[i]; i++)
        {
            free(cmd[i]);
        }
        free(cmd);
    }
    exit(0);
    return 0;
};