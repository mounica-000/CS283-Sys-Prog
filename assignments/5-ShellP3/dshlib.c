#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

#include "dshlib.h"


void remove_spaces(char* str) {

    int first, last, i;
    // to remove leading white spaces
    for (first = 0; str[first] != '\0'; first++) {
        if (str[first] != ' ' && str[first] != '\t' && str[first] != '\n') {
            break;
        }
    }
    // to remove trailing white spaces
    for (last = strlen(str) - 1; last >= 0; last--) {
        if (str[last] != ' ' && str[last] != '\t' && str[last] != '\n') {
            break;
        }
    }

    for (i=0; first <= last; first++, i++) {
        str[i] = str[first];
    }
    str[i] = '\0';

}

void remove_duplicate_spaces(char* str) {
    int index, next;
    bool in_quotes = false;
    int len = strlen(str);
    next = 0;

    for (index = 0; index < len; index++) {
        if (str[index] == '"') {
            in_quotes = !in_quotes;
        }
        if ((in_quotes == false) && 
            ((str[index] == ' ') ||
            (str[index] == '\t') ||
            (str[index] == '\n'))) {
                while (index + 1 < len && (str[index+1] == ' ' || str[index+1] == '\t' || str[index+1] == '\n')) {
                    index++;
                }
        }
        str[next++] = str[index];
    }
    str[next] = '\0';
}

int free_cmd_buff(cmd_buff_t *cmd_buff) {
    for (int i = 0; i < cmd_buff->argc; i++) {
        if (cmd_buff->argv[i] != NULL) {
            free(cmd_buff->argv[i]);
        }
    }
    return 0;
}

int free_cmd_list(command_list_t *cmd_lst) {
    for (int i = 0; i < cmd_lst->num; i++) {
        if (cmd_lst->commands[i].argc > 0) {
            free_cmd_buff(&cmd_lst->commands[i]);
        }
    }
    return 0;
}

int build_cmd_buff(char *cmd_line, cmd_buff_t *cmd_buff) {
    int rc = OK;
    char *cmd_copy;
    cmd_copy = (char*)malloc(strlen(cmd_line) + 1);
    // adding checks for memory leaks
    if (cmd_copy == NULL) { 
        return ERR_MEMORY;
    }
    strcpy(cmd_copy, cmd_line);

    //trim leading and trailing spaces
    remove_spaces(cmd_copy);

    // remove duplicate spaces outside quoted strings
    remove_duplicate_spaces(cmd_copy);
    
    int i = 0;
    int len = strlen(cmd_copy);
    char *arg_start = NULL;
    int in_quotes = 0;
    cmd_buff->argc = 0;

    if (len == 0 || cmd_copy[0] == '\0') {
        free(cmd_copy);
        return WARN_NO_CMDS;
    }

    while (i < len) {
        // if more than 8 commands:
        if (cmd_buff->argc >= CMD_MAX) {
            free(cmd_copy);
            return ERR_TOO_MANY_COMMANDS;
        }
        // opening quote
        if (cmd_copy[i] == '"' && !in_quotes) {
            in_quotes = 1;
            i++;
            arg_start = &cmd_copy[i];
            continue;
        }

        // closing quote
        if (cmd_copy[i] == '"' && in_quotes) {
            in_quotes = 0;
            cmd_copy[i] = '\0'; // end at quote
            cmd_buff->argv[cmd_buff->argc] = (char *)malloc(strlen(arg_start) + 1);
            if (cmd_buff->argv[cmd_buff->argc] == NULL) { 
                free(cmd_copy);
                return ERR_MEMORY;
            }
            strcpy(cmd_buff->argv[cmd_buff->argc++], arg_start);
            arg_start = NULL;
            i++;
            continue;
        }

        // if not in quotes and it's a space, end of the arg
        if ((cmd_copy[i] == ' ' || cmd_copy[i] == '\t' || cmd_copy[i] == '\n') && !in_quotes) {
            cmd_copy[i] = '\0'; // ends the arg
            if (arg_start != NULL) {
                cmd_buff->argv[cmd_buff->argc] = (char *)malloc(strlen(arg_start) + 1);
                if (cmd_buff->argv[cmd_buff->argc] == NULL) { 
                    free(cmd_copy);
                    return ERR_MEMORY;
                }
                strcpy(cmd_buff->argv[cmd_buff->argc++], arg_start);
                arg_start = NULL;
            }
            i++;
            continue;
        }
        // if at the start of an argument, save the position
        if (arg_start == NULL && cmd_copy[i] != ' ' && cmd_copy[i] != '\t' && cmd_copy[i] != '\n') {
            arg_start = &cmd_copy[i];
        }

        // now move to next character
        i++;
    }

    // to add the last arg if not added
    if (arg_start != NULL && cmd_buff->argc < CMD_MAX) {
        cmd_buff->argv[cmd_buff->argc] = (char *)malloc(strlen(arg_start) + 1);
        if (cmd_buff->argv[cmd_buff->argc] == NULL) { 
            free(cmd_copy);  
            return ERR_MEMORY;
        }
        strcpy(cmd_buff->argv[cmd_buff->argc++], arg_start);
    }
    cmd_buff->argv[cmd_buff->argc] = NULL;
    free(cmd_copy);
    return rc;
}

int build_cmd_list(char *cmd_line, command_list_t *clist) {
    char* token;
    int count = 0;
    int rc = 0;
    char* outer_saveptr = NULL;

    char *cmd_copy;
    cmd_copy = (char*)malloc(strlen(cmd_line) + 1);
    if (cmd_copy == NULL) { 
        return ERR_MEMORY;
    }
    strcpy(cmd_copy, cmd_line);

    remove_spaces(cmd_copy);
    remove_duplicate_spaces(cmd_copy);
    if (strlen(cmd_copy) == 0) {
        return WARN_NO_CMDS;
    }

    const char *result = strchr(cmd_copy, PIPE_CHAR);
    
    if (result != NULL) { // pipe exists
        token = strtok_r(cmd_copy, PIPE_STRING, &outer_saveptr);

        while (token != NULL) {
            count += 1;
            if (count > CMD_MAX) {
                return ERR_TOO_MANY_COMMANDS;
            }

            char* token_copy = (char*)malloc(SH_CMD_MAX);
            strcpy(token_copy, token);
            remove_spaces(token_copy);
            // printf("Tokens: '%s'\n", token_copy);
            cmd_buff_t command = {0};
            rc = build_cmd_buff(token_copy, &command);
            if (rc != OK) {
                return rc;
            }

            clist->commands[count-1] = command;
            token = strtok_r(NULL, PIPE_STRING, &outer_saveptr);
        }
        clist->num = count;
    }

    else { // pipe doesn't exist
        clist->num = 1;
        cmd_buff_t command = {0};
        rc = build_cmd_buff(cmd_copy, &command);
        if (rc != OK) {
            return rc;
        }
        clist->commands[0] = command;
    }

    return OK;
}


Built_In_Cmds exec_built_in_cmd(cmd_buff_t *cmd) {
    if (strcmp(cmd->argv[0], EXIT_CMD) == 0) {
        return BI_CMD_EXIT;
    }
    if (strcmp(cmd->argv[0], "cd") == 0) {
        if (cmd->argc == 1) {
            return BI_EXECUTED;
        }
        if (chdir(cmd->argv[1]) != 0) {
            printf("Error changing dir to %s\n", cmd->argv[1]);
        }
        return BI_EXECUTED;
    }
    if (strcmp(cmd->argv[0], "dragon") == 0) {
        printf("Not implemented yet\n");
        return BI_EXECUTED;
    }
    if (strcmp(cmd->argv[0], "rc") == 0) {
        return BI_RC;
    }
    return BI_NOT_BI;
}


int execute_pipeline(command_list_t *clist) {

    if (clist->num == 1) {
        int built_in_rc = exec_built_in_cmd(&clist->commands[0]);
        if (built_in_rc != BI_NOT_BI) {
            return OK;
        }
    }

    int pipes[clist->num - 1][2];  
    pid_t pids[clist->num];        

    // Create all necessary pipes
    for (int i = 0; i < clist->num - 1; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    // Create processes for each command
    for (int i = 0; i < clist->num; i++) {
        pids[i] = fork();
        if (pids[i] == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        if (pids[i] == 0) {  // Child process
            // Set up input pipe for all except first process
            if (i > 0) {
                dup2(pipes[i-1][0], STDIN_FILENO);
            }

            // Set up output pipe for all except last process
            if (i < clist->num - 1) {
                dup2(pipes[i][1], STDOUT_FILENO);
            }

            // Close all pipe ends in child
            for (int j = 0; j < clist->num - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            // extra credit redirection
            // assignment instruction: modify command execution 
            // to use dup2() in a similar way as you did for pipes; 
            // you can copy the in and out fd's specified by the user 
            // on to STDIN or STDOUT of the child's forked process
            for (int m=0; m < clist->commands[i].argc; m++) {
                // > : write to the file given
                if (strcmp(clist->commands[i].argv[m], ">") == 0) {
                    int flags = O_WRONLY | O_CREAT | O_TRUNC;
                    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;
                    // open next arg as file to write to
                    int fd = open(clist->commands[i].argv[m+1], flags, mode);
                    if (fd < 0) {
                        perror("open");
                        exit(EXIT_FAILURE);
                    }
                    // printf("I AM here\n");
                    dup2(fd, STDOUT_FILENO);
                    // close source fd
                    close(fd);
                    // stop the argv here for execvp
                    clist->commands[i].argv[m] = NULL;
                    break;
                }
                // < : read from the file
                if (strcmp(clist->commands[i].argv[m], "<") == 0) {
                    int flags = O_RDONLY;
                    // open next arg as file to read from
                    int fd = open(clist->commands[i].argv[m+1], flags);
                    if (fd < 0) {
                        perror("file open");
                        exit(EXIT_FAILURE);
                    }
                    dup2(fd, STDIN_FILENO);
                    // close souce fd
                    close(fd);
                    // remove < from argv for execvp
                    clist->commands[i].argv[m] = NULL;
                    break;
                }
            }

            // Execute command
            execvp(clist->commands[i].argv[0], clist->commands[i].argv);
            perror("execvp");
            exit(EXIT_FAILURE);
        }
    }

    // Parent process: close all pipe ends
    for (int i = 0; i < clist->num - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    // Wait for all children
    for (int i = 0; i < clist->num; i++) {
        waitpid(pids[i], NULL, 0);
    }
    return OK;
}




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
int exec_local_cmd_loop()
{

    char *cmd_buff = (char*)malloc(SH_CMD_MAX);
    int rc = 0;
    int result = 0;

    // MAIN LOOP:
    while (1)
    {
        command_list_t cmds = {0};

        printf("%s", SH_PROMPT);
        if (fgets(cmd_buff, SH_CMD_MAX, stdin) == NULL)
        {
            printf("\n");
            free(cmd_buff);
            return OK;
            // break;
        }
        
        // remove the trailing \n from cmd_buff
        cmd_buff[strcspn(cmd_buff, "\n")] = '\0';

        // printf("Cmd: %s\n", cmd_buff);

        if (strcmp(cmd_buff, "") == 0) {
            // printf(CMD_WARN_NO_CMD);
            continue;
        }

        if (strcmp(cmd_buff, "\0") == 0) {
            return OK;
        }

        if (strcmp(cmd_buff, EXIT_CMD) == 0) {
            free(cmd_buff);
            // rc = OK;
            // break;
            return OK;
        }

        // build command list -> build command buff 
        result = build_cmd_list(cmd_buff, &cmds);
        

        // DEBUG PRINTING:
        // int i, j;
        // printf("argc: %d\n", cmds.num);
        // for (i = 0; i < cmds.num; i++) {
        //     for (j = 0; j < cmds.commands[i].argc; j++) {
        //         printf("'%s', ", cmds.commands[i].argv[j]);
        //     }
        //     printf("| \n");
        // }
        // printf("\n");

        switch(result) {
            case ERR_TOO_MANY_COMMANDS:
                printf(CMD_ERR_PIPE_LIMIT, CMD_MAX);
                free_cmd_list(&cmds);
                continue;
            case WARN_NO_CMDS:
                free_cmd_list(&cmds);
                continue;
            case ERR_MEMORY:
                free_cmd_list(&cmds);
                rc = ERR_MEMORY;
                break;
        }


        // exec pipeline -> exec built-in / exec cmd
        rc = execute_pipeline(&cmds);

        free_cmd_list(&cmds);

    }

    
    free(cmd_buff);
    return rc;
}
