#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "dshlib.h"
#include <errno.h>


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
        free(cmd_buff->argv[i]);
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
        if (cmd_buff->argc > CMD_MAX) {
            // printf("IAM HERE\n");
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
    // printf("argc: %d\n", cmd_buff->argc);
    free(cmd_copy);
    return rc;
}


Built_In_Cmds exec_built_in_cmd(cmd_buff_t *cmd) {
    if (strcmp(cmd->argv[0], EXIT_CMD) == 0) {
        return BI_CMD_EXIT;
    }
    if (strcmp(cmd->argv[0], "cd") == 0) {
        // printf("Its 'CD'\n");
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

int exec_cmd(cmd_buff_t *cmd) {
    // class demo code
    int f_result, c_result;

    f_result = fork();
    if (f_result < 0) {
        exit(ERR_EXEC_CMD);  // fork failed, return
    }

    if (f_result == 0) {
        //Exec of child
        int rc;

        rc = execvp(cmd->argv[0], cmd->argv);
        // if (rc < 0) {
        //     return rc;   //fork failed, return
        // }
        if (rc == ENOENT) {
            printf("No such file or directory\n");
            exit(rc);
        }
        if (rc == EACCES) {
            printf("Permission denied\n");
            exit(rc);
        }
        if (rc == EBADF) {
            printf("Bad file descriptor\n");
            exit(rc);
        }
        if (rc < 0) {
            printf("Error executing command\n");
            exit(rc);   //fork failed, return
        }
    } 
    else {
        // printf("[p] Parent process id is %d\n", getpid());
        // printf("[p] Child process id is %d\n", f_result);
        wait(&c_result);

        // printf("[p] The child exit status is %d\n", WEXITSTATUS(c_result));
        
        return WEXITSTATUS(c_result);
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
    int built_in_rc = 0;
    int exec_rc = 0;
    cmd_buff_t cmd = {0};

    // TODO IMPLEMENT MAIN LOOP:
    while (1)
    {
        printf("%s", SH_PROMPT);
        if (fgets(cmd_buff, SH_CMD_MAX, stdin) == NULL)
        {
            printf("\n");
            break;
        }
        // remove the trailing \n from cmd_buff
        cmd_buff[strcspn(cmd_buff, "\n")] = '\0';

        // printf("Cmd: %s\n", cmd_buff);

        if (strcmp(cmd_buff, "") == 0) {
            // printf(CMD_WARN_NO_CMD);
            continue;
        }

        if (strcmp(cmd_buff, EXIT_CMD) == 0) {
            free(cmd_buff);
            rc = 0;
            break;
        }

        
        // parsing input to cmd_buff_t *cmd_buff
        result = build_cmd_buff(cmd_buff, &cmd);

        // DEBUG PRINTING:
        // printf("argc: %d\n[", cmd.argc);
        // for (int i = 0; i < cmd.argc; i++) {
        //     printf("'%s'", cmd.argv[i]);
        // }
        // printf("]\n");

        switch(result) {
            case ERR_TOO_MANY_COMMANDS:
                printf(CMD_ERR_PIPE_LIMIT, CMD_MAX);
                continue;
            case WARN_NO_CMDS:
                continue;
            case ERR_MEMORY:
                rc = ERR_MEMORY;
                break;
        }

        // if built-in command, execute builtin logic for exit, cd (extra credit: dragon)
        // the cd command should chdir to the provided directory; if no directory is provided, do nothing

        built_in_rc = exec_built_in_cmd(&cmd);

        if (built_in_rc == BI_CMD_EXIT) { // exit
            free_cmd_buff(&cmd);
            free(cmd_buff);
            rc = OK;
            return rc;;
        }
        if (built_in_rc == BI_EXECUTED) { // done with this command, get next
            continue;
        }
        if (built_in_rc == BI_RC) {
            printf("%d\n", exec_rc);
            continue;
        }
        
        // if not built-in command, fork/exec as an external command

        exec_rc = exec_cmd(&cmd);
        if (exec_rc == ERR_EXEC_CMD) {
            // printf("%d\n", exec_rc);
            printf("Error executing command\n");
        }

    }

    free_cmd_buff(&cmd);

    return rc;
}