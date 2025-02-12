#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "dshlib.h"

/*
 *  build_cmd_list
 *    cmd_line:     the command line from the user
 *    clist *:      pointer to clist structure to be populated
 *
 *  This function builds the command_list_t structure passed by the caller
 *  It does this by first splitting the cmd_line into commands by spltting
 *  the string based on any pipe characters '|'.  It then traverses each
 *  command.  For each command (a substring of cmd_line), it then parses
 *  that command by taking the first token as the executable name, and
 *  then the remaining tokens as the arguments.
 *
 *  NOTE your implementation should be able to handle properly removing
 *  leading and trailing spaces!
 *
 *  errors returned:
 *
 *    OK:                      No Error
 *    ERR_TOO_MANY_COMMANDS:   There is a limit of CMD_MAX (see dshlib.h)
 *                             commands.
 *    ERR_CMD_OR_ARGS_TOO_BIG: One of the commands provided by the user
 *                             was larger than allowed, either the
 *                             executable name, or the arg string.
 *
 *  Standard Library Functions You Might Want To Consider Using
 *      memset(), strcmp(), strcpy(), strtok(), strlen(), strchr()
 */
static char* trim(char* str) {
    while(isspace((unsigned char)*str)) str++;
    
    if(*str == 0) return str;
    
    char* end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    
    return str;
}

static void remove_spaces(char* str) {
    char* dest = str;
    while (*str != '\0') {
        if (!isspace(*str)) {
            *dest = *str;
            dest++;
        }
        str++;
    }
    *dest = '\0';
}

static void print_dragon() {
    puts("@%%%%");
    puts("                                                                     %%%%%%");
    puts("                                                                    %%%%%%");
    puts("                                                                 % %%%%%%%           @");
    puts("                                                                %%%%%%%%%%        %%%%%%%");
    puts("                                       %%%%%%%  %%%%@         %%%%%%%%%%%%@    %%%%%%  @%%%%");
    puts("                                  %%%%%%%%%%%%%%%%%%%%%%      %%%%%%%%%%%%%%%%%%%%%%%%%%%%");
    puts("                                %%%%%%%%%%%%%%%%%%%%%%%%%%   %%%%%%%%%%%% %%%%%%%%%%%%%%%");
    puts("                               %%%%%%%%%%%%%%%%%%%%%%%%%%%%% %%%%%%%%%%%%%%%%%%%     %%%");
    puts("                             %%%%%%%%%%%%%%%%%%%%%%%%%%%%@ @%%%%%%%%%%%%%%%%%%        %%");
    puts("                            %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% %%%%%%%%%%%%%%%%%%%%%%");
    puts("                            %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%");
    puts("                            %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%@%%%%%%@");
    puts("      %%%%%%%%@           %%%%%%%%%%%%%%%%        %%%%%%%%%%%%%%%%%%%%%%%%%%      %%");
    puts("    %%%%%%%%%%%%%         %%@%%%%%%%%%%%%           %%%%%%%%%%% %%%%%%%%%%%%      @%");
    puts("  %%%%%%%%%%   %%%        %%%%%%%%%%%%%%            %%%%%%%%%%%%%%%%%%%%%%%%");
    puts(" %%%%%%%%%       %         %%%%%%%%%%%%%             %%%%%%%%%%%%@%%%%%%%%%%%");
    puts("%%%%%%%%%@                % %%%%%%%%%%%%%            @%%%%%%%%%%%%%%%%%%%%%%%%%");
    puts("%%%%%%%%@                 %%@%%%%%%%%%%%%            @%%%%%%%%%%%%%%%%%%%%%%%%%%%%");
    puts("%%%%%%%@                   %%%%%%%%%%%%%%%           %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%");
    puts("%%%%%%%%%%                  %%%%%%%%%%%%%%%          %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%      %%%%");
    puts("%%%%%%%%%@                   @%%%%%%%%%%%%%%         %%%%%%%%%%%%@ %%%% %%%%%%%%%%%%%%%%%   %%%%%%%%");
    puts("%%%%%%%%%%                  %%%%%%%%%%%%%%%%%        %%%%%%%%%%%%%      %%%%%%%%%%%%%%%%%% %%%%%%%%%");
    puts("%%%%%%%%%@%%@                %%%%%%%%%%%%%%%%@       %%%%%%%%%%%%%%     %%%%%%%%%%%%%%%%%%%%%%%%  %%");
    puts(" %%%%%%%%%%                  % %%%%%%%%%%%%%%@        %%%%%%%%%%%%%%   %%%%%%%%%%%%%%%%%%%%%%%%%% %%");
    puts("  %%%%%%%%%%%%  @           %%%%%%%%%%%%%%%%%%        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%  %%%");
    puts("   %%%%%%%%%%%%% %%  %  %@ %%%%%%%%%%%%%%%%%%          %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%    %%%");
    puts("    %%%%%%%%%%%%%%%%%% %%%%%%%%%%%%%%%%%%%%%%           @%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%    %%%%%%%");
    puts("     %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%              %%%%%%%%%%%%%%%%%%%%%%%%%%%%        %%%");
    puts("      @%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%                  %%%%%%%%%%%%%%%%%%%%%%%%%");
    puts("        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%                      %%%%%%%%%%%%%%%%%%%  %%%%%%%");
    puts("           %%%%%%%%%%%%%%%%%%%%%%%%%%                           %%%%%%%%%%%%%%%  @%%%%%%%%%");
    puts("              %%%%%%%%%%%%%%%%%%%%           @%@%                  @%%%%%%%%%%%%%%%%%%   %%%");
    puts("                  %%%%%%%%%%%%%%%        %%%%%%%%%%                    %%%%%%%%%%%%%%%    %");
    puts("                %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%                      %%%%%%%%%%%%%%");
    puts("                %%%%%%%%%%%%%%%%%%%%%%%%%%  %%%% %%%                      %%%%%%%%%%  %%%@");
    puts("                     %%%%%%%%%%%%%%%%%%% %%%%%% %%                          %%%%%%%%%%%%%@");
    puts("                                                                                 %%%%%%%@");
}

int build_cmd_list(char *cmd_line, command_list_t *clist)
{
    memset(clist, 0, sizeof(command_list_t));
    
    char cmd_line_copy[SH_CMD_MAX];
    strncpy(cmd_line_copy, cmd_line, SH_CMD_MAX - 1);
    cmd_line_copy[SH_CMD_MAX - 1] = '\0';
    
    char *trimmed_cmd = trim(cmd_line_copy);
    
    if (strlen(trimmed_cmd) == 0) {
        return WARN_NO_CMDS;
    }

    if (strcmp(trimmed_cmd, "dragon") == 0) {
        print_dragon();
        clist->num = 1;
        strcpy(clist->commands[0].exe, "dragon");
        clist->commands[0].args[0] = '\0';
        printf("PARSED COMMAND LINE - TOTAL COMMANDS %d", clist->num);
        printf("\n<%d>%s", 1, clist->commands[0].exe);
        return OK;
    }
    
    char *saveptr1;
    char *cmd_str = strtok_r(trimmed_cmd, PIPE_STRING, &saveptr1);
    while (cmd_str != NULL) {
        if (clist->num >= CMD_MAX) {
            return ERR_TOO_MANY_COMMANDS;
        }
        
        command_t *curr_cmd = &clist->commands[clist->num];
        
        cmd_str = trim(cmd_str);
        
        char cmd_copy[SH_CMD_MAX];
        strncpy(cmd_copy, cmd_str, SH_CMD_MAX - 1);
        cmd_copy[SH_CMD_MAX - 1] = '\0';
        
        char *saveptr2;
        char *token = strtok_r(cmd_copy, " ", &saveptr2);
        if (token == NULL) {
            continue;
        }
        
        if (strlen(token) >= EXE_MAX) {
            return ERR_CMD_OR_ARGS_TOO_BIG;
        }
        
        strcpy(curr_cmd->exe, token);
        
        char *args_start = cmd_str + strlen(token);
        while (*args_start == ' ') args_start++;
        
        if (*args_start != '\0') {
            if (strlen(args_start) >= ARG_MAX) {
                return ERR_CMD_OR_ARGS_TOO_BIG;
            }
            strcpy(curr_cmd->args, args_start);
        } else {
            curr_cmd->args[0] = '\0';
        }
        
        clist->num++;
        
        cmd_str = strtok_r(NULL, PIPE_STRING, &saveptr1);
    }
    
    printf("PARSED COMMAND LINE - TOTAL COMMANDS %d", clist->num);
    
    if (clist->num > 0) {
        for (int i = 0; i < clist->num; i++) {
            printf("<%d> %s", i + 1, clist->commands[i].exe);
            if (strlen(clist->commands[i].args) > 0) {
                char args_copy[ARG_MAX];
                strcpy(args_copy, clist->commands[i].args);
                remove_spaces(args_copy);
                printf("[%s]", args_copy);
            }
            printf("\n");
        }
    }
    
    return OK;
}
