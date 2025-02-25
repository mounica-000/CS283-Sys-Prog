#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

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
int build_cmd_list(char *cmd_line, command_list_t *clist)
{
    char* token;
    int count = 0;
    char* outer_saveptr = NULL;
    char* inner_saveptr = NULL;

    char *cmd_copy;
    cmd_copy = (char*)malloc(SH_CMD_MAX);
    strcpy(cmd_copy, cmd_line);

    // printf("CMD COPY: '%s'\n", cmd_copy);
    remove_spaces(cmd_copy);
    // printf("REMOVED SPACES: '%s'\n", cmd_copy);


    
    const char *result = strchr(cmd_copy, PIPE_CHAR);
    
    if (result != NULL) { // pipe exists
        token = strtok_r(cmd_copy, PIPE_STRING, &outer_saveptr);
        ///// don't use tokkennnnnnn more than onceeeeeeeeeee

        while (token != NULL) {
            count += 1;
            if (count > 8) {
                return ERR_TOO_MANY_COMMANDS;
            }

            char* token_copy = (char*)malloc(SH_CMD_MAX);
            strcpy(token_copy, token);
            remove_spaces(token_copy);
            // printf("Tokens: '%s'\n", token_copy);
            command_t command = {0};

            char *result2 = strchr(token_copy, SPACE_CHAR);
            if (result2 != NULL) { // there are spaces ==> args
                char *token_with_args = strtok_r(token_copy, " ", &inner_saveptr);
                int exe_name = 1;
                char *args = (char*)malloc(ARG_MAX);
                memset(args, ' ', (ARG_MAX-1)*sizeof(char));
                int arg_index = 0;

                while (token_with_args != NULL) {
                    char* token_copy2 = (char*)malloc(SH_CMD_MAX);
                    strcpy(token_copy2, token_with_args);
                    remove_spaces(token_copy2);
                    if (exe_name == 1) {
                        if(strlen(token_copy2) > EXE_MAX) {
                            return ERR_CMD_OR_ARGS_TOO_BIG;
                        }
                        // printf("EXE NAME: '%s'\n", token_copy2);
                        strncpy(command.exe, token_copy2, sizeof(command.exe));
                        exe_name = 0;
                    }
                    else {
                        int i;
                        if (strlen(token_copy2) > ARG_MAX) {
                            return ERR_CMD_OR_ARGS_TOO_BIG;
                        }
                        // printf("Args: '%s'\n", token_copy2);
                        for(i=0; i<(int)strlen(token_copy2); i++) {
                            *(args + arg_index) = token_copy2[i];
                            // args[arg_index] = token_copy2[i];
                            arg_index += 1;
                        }
                        arg_index += 1;
                        // arg_index += 1;
                    }
                    
                    token_with_args = strtok_r(NULL, " ", &inner_saveptr);
                }
                *(args + arg_index - 1) = '\0';
                strncpy(command.args, args, sizeof(command.args));
                // printf("ARGS IN PIPE CMD: '%s'\n", args);
            } else { /// No args in pipeeee
                // printf("token in no args: '%s'\n", token_copy);
                // command_t command = {0};
                char *empty = "";
                strncpy(command.exe, token_copy, sizeof(command.exe));
                strncpy(command.args, empty, sizeof(command.args));

            }
            clist->commands[count-1] = command;
            token = strtok_r(NULL, PIPE_STRING, &outer_saveptr);
        }

        clist->num = count;
    }

    else { // pipe doesn't exist
        count = 1;
        token = strtok(cmd_copy, " ");
        char *token_copy = (char*)malloc(SH_CMD_MAX);
        strcpy(token_copy, token);
        remove_spaces(token_copy);
        if (strlen(token_copy) > EXE_MAX) {
            return ERR_CMD_OR_ARGS_TOO_BIG;
        }
        char *args = cmd_copy+strlen(token)+1;
        remove_spaces(args);

        clist->num = count;
        command_t command = {0};
        // printf("Token no pipe: '%s'\n", token_copy);
        // printf("Args in no pipe: '%s'\n", args);
        strncpy(command.exe, token_copy, sizeof(command.exe));
        strncpy(command.args, args, sizeof(command.args));
        clist->commands[0] = command;
        
    }
    
    
    
    
    
    
    
    // char cmd_copy[strlen(cmd_line)+1];
    // strncpy(cmd_copy, cmd_line, strlen(cmd_line));
    // cmd_copy[strlen(cmd_line)] = '\0';
    // char *cmd_ptr = cmd_copy;

    // int count = 0;
    // const char SPACE[4] = " ";
    // // const char PIPE[12] = " | ";

    // const char *result = strchr(cmd_line, PIPE_CHAR);
    // printf("CMD: %s\n", cmd_line);

    // if (result != NULL) { // pipe exists
    //     char *token = strtok(cmd_line, PIPE_STRING); 

    //     while (token != NULL) {
    //         count += 1;
    //         if (count > 8) {
    //             return ERR_TOO_MANY_COMMANDS;
    //         }
    //         char *trimmed_token = remove_spaces(token);
    //         command_t command = {0};
    //         printf ("TOKEN IN PIPE: %s\n", trimmed_token);
    //         const char *result2 = strchr(trimmed_token, SPACE_CHAR);

    //         if (result2 != NULL) { // there are spaces ==> args

    //             char token_copy[strlen(trimmed_token)+1];
    //             strncpy(token_copy, trimmed_token, strlen(trimmed_token)+1);
    //             token_copy[strlen(trimmed_token)+1] = '\0';
    //             char *token_ptr = token_copy;
    //             char *arg = strtok(token_copy, SPACE);
                
    //             printf("TOKEN WITH ARGS: %s ;;;; REST: %s\n", arg, token_ptr + strlen(arg) + 1);
    //             if (strlen(arg) > EXE_MAX) {
    //                 return ERR_CMD_OR_ARGS_TOO_BIG;
    //             }  
    //             strncpy(command.exe, arg, sizeof(command.exe));
    //             strncpy(command.args, token_ptr + strlen(arg) + 1, sizeof(command.args)); 
    //             // free(token_ptr);
    //         }

    //         else { /// no args
    //             char *trimmed_token = remove_spaces(token);
    //             printf("TOKEN IN NO ARGS: %s\n", trimmed_token);
    //             char *empty = "";
    //             strncpy(command.exe, trimmed_token, sizeof(command.exe));  
    //             strncpy(command.args, empty, sizeof(command.args)); 
    //         }
    //         token = strtok(NULL, PIPE_STRING);
    //         clist->commands[count-1] = command;
    //     }
    //     clist->num = count;
    //     // free(token);
    // }

    // else {  // no pipe
        
    //     count = 1;
    //     char *token = strtok(cmd_line, SPACE);
    //     clist->num = count;
    //     command_t command = {0};

    //     if (strlen(token) > EXE_MAX) {
    //         return ERR_CMD_OR_ARGS_TOO_BIG;
    //     }
    //     strncpy(command.exe, token, sizeof(command.exe));
    //     strncpy(command.args, cmd_ptr + strlen(token) + 1, sizeof(command.args));
    //     clist->commands[0] = command;
        // token = strtok(NULL, PIPE_STRING);
        // printf("TOKEN: %s\nREST: %s\n", token, cmd_ptr + strlen(token) + 1);

        // while (token != 0) {
        //     if (exe_name == 1) { // if it the first argument, it the executable name
        //         if (strlen(token) > EXE_MAX) {
        //             return ERR_CMD_OR_ARGS_TOO_BIG;
        //         }
        //         // printf("%s [", token);
        //         strncpy(command.exe, token, sizeof(command.exe));
        //         exe_name = 0;
        //     }
        //     else {
        //         // printf("%s ", token);
        //         if (strlen(token) > ARG_MAX) {
        //             return ERR_CMD_OR_ARGS_TOO_BIG;
        //         }
        //         int i;
        //         for (i = 0; i < strlen(token); i++) {
        //             strncpy()
        //         }

        //         strncpy(command.args, token, sizeof(command.args));
        //     }
        //     token = strtok(0, SPACE);
        // }
        // printf("]\n");
        // free(token);
    // }
    
    return OK;
}