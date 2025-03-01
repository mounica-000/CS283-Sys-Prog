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
    printf("Length in remove dup space: %d\n", len);
    next = 0;

    for (index = 0; index < len; index++) {
        if (str[index] == '"') {
            in_quotes = !in_quotes;
            // printf("FOund quotes at: %d\n", index);
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
    int rc = 0;
    // int count = 0;
    // bool in_quotes = false;

    char *cmd_copy;
    cmd_copy = (char*)malloc(strlen(cmd_line)+1);
    strcpy(cmd_copy, cmd_line);

    //trim leading and trailing spaces
    remove_spaces(cmd_copy);

    // remove duplicate spaces outside quoted strings
    remove_duplicate_spaces(cmd_copy);
    // printf("Removed duplicate spaces: %s\n", cmd_copy);
    
    // using _cmd_buff to store the command??
    // cmd_buff->_cmd_buffer = (char*)malloc(SH_CMD_MAX);
    // strncpy(cmd_buff->_cmd_buffer, cmd_copy, strlen(cmd_copy));

    // quoted strings with spaces are single arguments
    // const char *start = cmd_copy;
    // const char *end;
    // cmd_buff->argc = 0;

    // while (*start != '\0') {
    //     if (cmd_buff->argc > CMD_MAX) {
    //         rc = ERR_TOO_MANY_COMMANDS;
    //         break;
    //     }
    //     // Check for a quoted string
    //     if (*start == '"') {
    //         in_quotes = true;
    //         *start++;
    //         end = strchr(start, '"');
    //     } else {
    //         end = strchr(start, ' ');
    //         in_quotes = false;
    //     }

    //     if (end == NULL || *end == '\0') {
    //         end = start + strlen(start);
    //     }

    //     // Copy the argument
    //     int length = end - start;
    //     cmd_buff->argv[cmd_buff->argc] = (char*)malloc(length + 1);
    //     strncpy(cmd_buff->argv[cmd_buff->argc], start, length);
    //     cmd_buff->argv[cmd_buff->argc][length] = '\0';

    //     cmd_buff->argc += 1;
    //     if (in_quotes == true) {
    //         *start = *(end + 1);
    //     }
    //     else {
    //         *start = *end;
    //     }
    // } 
    // // cmd_buff->argc = count;
    // printf("argc valueeeeee: %d\n", cmd_buff->argc);
    // cmd_buff->argv[cmd_buff->argc] = NULL;

    int i = 0;
    int len = strlen(cmd_copy);
    char *arg_start = NULL;
    int inside_quotes = 0;

    // Initialize argc to 0
    cmd_buff->argc = 0;

    while (i < len && cmd_buff->argc < CMD_MAX)
    {

        // Check for opening quote
        if (cmd_copy[i] == '"' && !inside_quotes)
        {
            inside_quotes = 1;
            i++;
            arg_start = (char *)&cmd_copy[i];
            continue;
        }

        // Check for closing quote
        if (cmd_copy[i] == '"' && inside_quotes)
        {
            inside_quotes = 0;
            cmd_copy[i] = '\0'; // Terminate the argument at the quote position
            cmd_buff->argv[cmd_buff->argc++] = (char *)arg_start;
            i++;
            continue;
        }

        // If not inside quotes and we hit a space, end of the argument
        if (isspace(cmd_copy[i]) && !inside_quotes)
        {
            cmd_copy[i] = '\0'; // Null-terminate the argument
            cmd_buff->argv[cmd_buff->argc++] = (char *)&cmd_copy[arg_start - cmd_copy];
            i++;
            continue;
        }

        // Otherwise, just move to the next character
        i++;
    }

    // If there is an argument left that wasn't added
    if (i < len && cmd_buff->argc < CMD_MAX)
    {
        cmd_buff->argv[cmd_buff->argc++] = (char *)&cmd_copy[arg_start - cmd_copy];
    }
    cmd_buff->argv[cmd_buff->argc] = NULL;

    
    return rc;
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
            printf(CMD_WARN_NO_CMD);
            continue;
        }

        if (strcmp(cmd_buff, EXIT_CMD) == 0) {
            free(cmd_buff);
            rc = 0;
            break;
        }

        
        // parsing input to cmd_buff_t *cmd_buff
        result = build_cmd_buff(cmd_buff, &cmd);

        printf("argc: %d\n", cmd.argc);
        for (int i = 0; i < cmd.argc; i++) {
            printf("argv[%d]: %s\n", i, cmd.argv[i]);
        }

        // TODO IMPLEMENT if built-in command, execute builtin logic for exit, cd (extra credit: dragon)
        // the cd command should chdir to the provided directory; if no directory is provided, do nothing

        // TODO IMPLEMENT if not built-in command, fork/exec as an external command
        // for example, if the user input is "ls -l", you would fork/exec the command "ls" with the arg "-l"

    }

    free_cmd_buff(&cmd);

    return rc;
}
