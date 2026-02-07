#include <sys/types.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "consts.h"

/**
 * Function gets the array of args.
 * returns the index of the parameter containing the redirection if found, else -1.
*/
int find_redirection(const char const **argv, int argc)
{
    int redirection_index = -1;
    for (int i = 1; i < argc; ++i)
    {
        if (NULL != strstr(argv[i], ">") || NULL != strstr(argv[i], "<"))
        {
            redirection_index = i;
            break;
        }
    }
    return redirection_index;
}

char **split_string(char *input, const char *delim, int *count)
{
    int max_tokens = 10;
    int current_tokens = 0;
    char **result = malloc(max_tokens * sizeof(char *));
    char *token;
    char *string_ptr = input;

    while ((token = strsep(&string_ptr, delim)) != NULL)
    {
        // Resize the array if we run out of space
        if (current_tokens >= max_tokens)
        {
            max_tokens *= 2;
            result = realloc(result, max_tokens * sizeof(char *));
        }

        result[current_tokens++] = token;
    }

    *count = current_tokens;
    return result;
}

int parse_command(char **command)
{
    char *full_path = NULL;
    int ret_val = 0;
    int argc, redirection_index;
    pid_t cpid;
    char **argv = split_string(*command, " ", &argc);
    char *bin_name = argv[0];

    if (0 == strcmp(EXIT_COMMAND, bin_name))
    {
        ret_val = 1;
        goto cleanup;
    }
    if (access(argv[0], X_OK) == 0) goto run_proc;
    if (asprintf(&full_path, "%s%s", BINARIES_PATH, argv[0]) < sizeof(BINARIES_PATH))
    {
        ret_val = -1;
        goto cleanup;
    }
    if (access(full_path, X_OK) != 0)
    {
        ret_val = -1;
        goto cleanup;
    }

    argv[0] = full_path;
run_proc:
    redirection_index = find_redirection(argv, argc);
    cpid = fork();
    if (cpid == -1)
    {
        perror("fork");
        ret_val = -1;
        goto cleanup;
    }
    if (cpid == 0)
    {
        execve(full_path, argv, NULL);
    } else
    {
        wait(NULL);
    }

cleanup:
    free(full_path);
    return ret_val;
}

int main()
{
    int ret_val = 0;
    char *command = NULL;
    size_t cmd_len = 0;
    ssize_t read_bytes;

    while (true)
    {
        printf("%s ", SHELL_PROMPT);
        read_bytes = getline(&command, &cmd_len, stdin);
        if (read_bytes < 0)
        {
            goto cleanup;
        }
        command[read_bytes - 1] = '\0';
        int command_status = parse_command(&command);
        if (command_status < 0)
        {
            printf("command not found!\n");
        }
        else if (command_status == 1)
        {
            goto cleanup;
        }

    next_command:
        free(command);
        command = NULL;
        cmd_len = 0;
    }

cleanup:
    free(command);
    return ret_val;
}