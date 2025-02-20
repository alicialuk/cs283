#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>
#include "dshlib.h"

/*
 * Implement your exec_local_cmd_loop function by building a loop that prompts the
 * user for input.  Use the SH_PROMPT constant from dshlib.h and then
 * use fgets to accept user input.
 *
 *      while(1){
 *        printf("%s", SH_PROMPT);
 *        if (fgets(cmd_buff, ARG_MAX, stdin) == NULL){
 *           printf("\n");
 *           break;
 *        }
 *        //remove the trailing \n from cmd_buff
 *        cmd_buff[strcspn(cmd_buff,"\n")] = '\0';
 *
 *        //IMPLEMENT THE REST OF THE REQUIREMENTS
 *      }
 *
 *   Also, use the constants in the dshlib.h in this code.
 *      SH_CMD_MAX              maximum buffer size for user input
 *      EXIT_CMD                constant that terminates the dsh program
 *      SH_PROMPT               the shell prompt
 *      OK                      the command was parsed properly
 *      WARN_NO_CMDS            the user command was empty
 *      ERR_TOO_MANY_COMMANDS   too many pipes used
 *      ERR_MEMORY              dynamic memory management failure
 *
 *   errors returned
 *      OK                     No error
 *      ERR_MEMORY             Dynamic memory management failure
 *      WARN_NO_CMDS           No commands parsed
 *      ERR_TOO_MANY_COMMANDS  too many pipes used
 *
 *   console messages
 *      CMD_WARN_NO_CMD        print on WARN_NO_CMDS
 *      CMD_ERR_PIPE_LIMIT     print on ERR_TOO_MANY_COMMANDS
 *      CMD_ERR_EXECUTE        print on execution failure of external command
 *
 *  Standard Library Functions You Might Want To Consider Using (assignment 1+)
 *      malloc(), free(), strlen(), fgets(), strcspn(), printf()
 *
 *  Standard Library Functions You Might Want To Consider Using (assignment 2+)
 *      fork(), execvp(), exit(), chdir()
 */
int alloc_cmd_buff(cmd_buff_t *cmd_buff) {
    cmd_buff->_cmd_buffer = malloc(SH_CMD_MAX);
    if (!cmd_buff->_cmd_buffer) {
        return ERR_MEMORY;
    }
    cmd_buff->argc = 0;
    for (int i = 0; i < CMD_ARGV_MAX; i++) {
        cmd_buff->argv[i] = NULL;
    }
    return OK;
}

int free_cmd_buff(cmd_buff_t *cmd_buff) {
    if (cmd_buff->_cmd_buffer) {
        free(cmd_buff->_cmd_buffer);
        cmd_buff->_cmd_buffer = NULL;
    }
    cmd_buff->argc = 0;
    for (int i = 0; i < CMD_ARGV_MAX; i++) {
        cmd_buff->argv[i] = NULL;
    }
    return OK;
}

int clear_cmd_buff(cmd_buff_t *cmd_buff) {
    cmd_buff->argc = 0;
    for (int i = 0; i < CMD_ARGV_MAX; i++) {
        cmd_buff->argv[i] = NULL;
    }
    return OK;
}

int exec_local_cmd_loop()
{
    char *cmd_buff = malloc(SH_CMD_MAX);
    int rc = 0;
    cmd_buff_t cmd;
    int last_return_code = 0;

    if (!cmd_buff) {
        return ERR_MEMORY;
    }

    rc = alloc_cmd_buff(&cmd);
    if (rc != OK) {
        free(cmd_buff);
        return rc;
    }

    while(1) {
        printf("%s", SH_PROMPT);
        if (fgets(cmd_buff, ARG_MAX, stdin) == NULL) {
            printf("\n");
            break;
        }
        //remove the trailing \n from cmd_buff
        cmd_buff[strcspn(cmd_buff, "\n")] = '\0';

        if (strlen(cmd_buff) == 0) {
            printf(CMD_WARN_NO_CMD);
            continue;
        }

        clear_cmd_buff(&cmd);

        rc = build_cmd_buff(cmd_buff, &cmd);
        if (rc != OK) {
            continue;
        }

        if (cmd.argc > 0) {
            if (strcmp(cmd.argv[0], EXIT_CMD) == 0) {
                free_cmd_buff(&cmd);
                free(cmd_buff);
                return OK;
            } else if (strcmp(cmd.argv[0], "cd") == 0) {
                if (cmd.argc > 1) {
                    if (chdir(cmd.argv[1]) != 0) {
                        perror("cd");
                        last_return_code = errno;
                    } else {
                        last_return_code = 0;
                    }
                }
                continue;
            } else if (strcmp(cmd.argv[0], "rc") == 0) {
                printf("%d\n", last_return_code);
                continue;
            }

            pid_t pid = fork();
            if (pid < 0) {
                perror("fork");
                last_return_code = errno;
                continue;
            } else if (pid == 0) {
                execvp(cmd.argv[0], cmd.argv);

                switch (errno) {
                    case ENOENT:
                        fprintf(stderr, "Command not found in PATH\n");
                        exit(2);
                    case EACCES:
                        fprintf(stderr, "Permission denied\n");
                        exit(13);
                    case ENOEXEC:
                        fprintf(stderr, "Not an executable file\n");
                        exit(126);
                    default:
                        perror("Command execution failed");
                        exit(errno);
                }
            } else {
                int status;
                waitpid(pid, &status, 0);
                if (WIFEXITED(status)) {
                    last_return_code = WEXITSTATUS(status);
                } else if (WIFSIGNALED(status)) {
                    last_return_code = 128 + WTERMSIG(status);
                }
            }
        }
    }

    free_cmd_buff(&cmd);
    free(cmd_buff);
    return OK;
}

int build_cmd_buff(char *cmd_line, cmd_buff_t *cmd_buff) {
    char *pointer;
    int i = 0;
    bool in_quote = false;

    strncpy(cmd_buff->_cmd_buffer, cmd_line, SH_CMD_MAX - 1);
    cmd_buff->_cmd_buffer[SH_CMD_MAX - 1] = '\0';
    pointer = cmd_buff->_cmd_buffer;

    while (*pointer == ' ') pointer++;
    if (!*pointer) return OK;

    cmd_buff->argv[0] = pointer;

    while (*pointer) {
        if (*pointer == '"') {
            if (!in_quote) {
                memmove(pointer, pointer + 1, strlen(pointer));
                in_quote = true;
            } else {
                memmove(pointer, pointer + 1, strlen(pointer));
                in_quote = false;
                if (*pointer == ' ') {
                    *pointer = '\0';
                    pointer++;
                    while (*pointer == ' ') pointer++;
                    if (*pointer && i < CMD_ARGV_MAX - 1) {
                        i++;
                        cmd_buff->argv[i] = pointer;
                    }
                    continue;
                }
            }
            continue;
        }

        if (*pointer == ' ' && !in_quote) {
            *pointer = '\0';
            pointer++;
            while (*pointer == ' ') pointer++;
            if (*pointer && i < CMD_ARGV_MAX - 1) {
                i++;
                cmd_buff->argv[i] = pointer;
            }
        } else {
            pointer++;
        }
    }

    cmd_buff->argv[i + 1] = NULL;
    cmd_buff->argc = i + 1;
    return OK;
}