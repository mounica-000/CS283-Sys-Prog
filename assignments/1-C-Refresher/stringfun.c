#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define BUFFER_SZ 50

//prototypes
void usage(char *);
void print_buff(char *, int);
int  setup_buff(char *, char *, int);

//prototypes for functions to handle required functionality
int  count_words(char *, int, int);
//add additional prototypes here
void reverse_string(char *, int);
int word_printer(char *, int);

int setup_buff(char *buff, char *user_str, int len){
    //TODO: #4:  Implement the setup buff as per the directions
    int number_of_spaces = 0;
    int buff_index = 0;
    // int given_str_len = 0;


    for (int i=0; i<len; i++) {
        if ((user_str[i] == ' ') && (number_of_spaces >= 1)) {
            number_of_spaces++;
            // printf("SPACES: %d\n", number_of_spaces);
        }
        else if ((user_str[i] == ' ') && (number_of_spaces < 1)) {
            buff[buff_index] = user_str[i];
            buff_index++;
            number_of_spaces++;
        }
        else if (user_str[i] == '\0') {
            // printf("End of string, buff_index: %d\n", buff_index);
            break;
        }
        else {
            buff[buff_index] = user_str[i];
            buff_index++;
            number_of_spaces = 0;
        }
    }
    if (buff_index >= 49) {
        return -1;
    }

    for (int j=buff_index; j<len; j++) {
        buff[j] = '.';
    }

    // printf("Buff is: %s\n", buff);
    return buff_index;  
}

void print_buff(char *buff, int len){
    printf("Buffer:  ");
    for (int i=0; i<len; i++){
        putchar(*(buff+i));
    }
    putchar('\n');
}

void usage(char *exename){
    printf("usage: %s [-h|c|r|w|x] \"string\" [other args]\n", exename);

}

int count_words(char *buff, int len, int str_len){
    //YOU MUST IMPLEMENT
    int word_count = 0;
    int word_start = 0;
    for (int i=0; i<str_len; i++) {
        if (word_start == 0) {
            if (buff[i] == ' ') {
                continue;
            }
            else {
                word_count++;
                word_start = 1;
            }
        }
        else {
            if (buff[i] == ' ') {
                word_start = 0;
            }
        }
    }

    return word_count;
}

//ADD OTHER HELPER FUNCTIONS HERE FOR OTHER REQUIRED PROGRAM OPTIONS

void reverse_string(char *buff, int str_len) {
    // char *reversed = (char*) malloc(str_len * sizeof(char));
    // int reversed_index = 0;
    // for (int i=str_len-1; i>=0; i--) {
    //     reversed[reversed_index] = buff[i];
    //     reversed_index++;
    // }
    // return reversed;

    int end_index = str_len-1;
    int start_index = 0;
    char tmp_char;

    while(end_index > start_index) {
        tmp_char = buff[start_index];
        buff[start_index] = buff[end_index];
        buff[end_index] = tmp_char;
        start_index++;
        end_index--;
    }

    printf("Reversed String: ");
    for (int i=0; i<str_len; i++){
        putchar(*(buff+i));
    }
    putchar('\n');


}

int word_printer(char *buff, int str_len) {
    int word_count = 0;
    int letter_count = 0;
    int word_start = 0; // 0=false, 1=true
    printf("Word Print\n");
    printf("----------\n");
    for (int i=0; i<str_len; i++) {
        if (word_start == 0) {
            if (buff[i] == ' ') {
                continue;
            }
            else {
                word_count++;
                printf("%d. ", word_count);
                word_start = 1;
                putchar(buff[i]);
                letter_count++;
            }
        }
        else {
            if ((buff[i] == ' ') || (i == str_len-1)) {
                putchar(buff[i]);
                if (i == str_len-1) {
                    letter_count++;
                    putchar(' ');
                }
                printf("(%d)\n", letter_count);
                letter_count = 0;
                word_start = 0;
            }
            else {
                putchar(buff[i]);
                letter_count++;
            }
        }
    }
    return 0;
}


int main(int argc, char *argv[]){

    char *buff;             //placehoder for the internal buffer
    char *input_string;     //holds the string provided by the user on cmd line
    char opt;               //used to capture user option from cmd line
    int  rc;                //used for return codes
    int  user_str_len;      //length of user supplied string

    //TODO:  #1. WHY IS THIS SAFE, aka what if arv[1] does not exist?
    //      PLACE A COMMENT BLOCK HERE EXPLAINING
    /** Explanation: This is because if argv[1] doesn't exist, the other condition which is checked first is fulfilled, argc would be less than 2, hence the next condition wouldn't be checked. So, doing this is safe.
     */
    if ((argc < 2) || (*argv[1] != '-')){
        usage(argv[0]);
        exit(1);
    }

    opt = (char)*(argv[1]+1);   //get the option flag

    //handle the help flag and then exit normally
    if (opt == 'h'){
        printf("See below\n");
        usage(argv[0]);
        exit(0);
    }

    //WE NOW WILL HANDLE THE REQUIRED OPERATIONS

    //TODO:  #2 Document the purpose of the if statement below
    //      PLACE A COMMENT BLOCK HERE EXPLAINING
    /** This gets triggered if the given number of arguments is less than the required number (3), that is, it is missing the input string, and prints the usage statement that says there should be an input string. */
    if (argc < 3){
        usage(argv[0]);
        exit(1);
    }

    input_string = argv[2]; //capture the user input string

    //TODO:  #3 Allocate space for the buffer using malloc and
    //          handle error if malloc fails by exiting with a 
    //          return code of 99
    // CODE GOES HERE FOR #3
    buff = (char*) malloc(BUFFER_SZ * sizeof(char));
    if (buff == NULL) {
        printf("MALLOC failed!\n");
        exit(99);
    }
    


    user_str_len = setup_buff(buff, input_string, BUFFER_SZ);     //see todos
    if (user_str_len < 0){
        printf("Error setting up buffer, error = %d\n", user_str_len);
        exit(2);
    }

    switch (opt){
        case 'c':
            rc = count_words(buff, BUFFER_SZ, user_str_len);  //you need to implement
            if (rc < 0){
                printf("Error counting words, rc = %d\n", rc);
                exit(2);
            }
            printf("Word Count: %d\n", rc);
            break;

        //TODO:  #5 Implement the other cases for 'r' and 'w' by extending
        //       the case statement options

        case 'r':
            reverse_string(buff, user_str_len);
            break;

        case 'w':
            rc = word_printer(buff, user_str_len);
            if (rc < 0) {
                printf("Error Printing.\n");
            }
            break;


        default:
            usage(argv[0]);
            exit(1);
    }

    //TODO:  #6 Dont forget to free your buffer before exiting
    print_buff(buff,BUFFER_SZ);
    free(buff);
    exit(0);
}

//TODO:  #7  Notice all of the helper functions provided in the 
//          starter take both the buffer as well as the length.  Why
//          do you think providing both the pointer and the length
//          is a good practice, after all we know from main() that 
//          the buff variable will have exactly 50 bytes?
//  
//          PLACE YOUR ANSWER HERE
/* This helps when we want to change the length of the buff, either globally or locally just for calling that function and hence it helps to pass it to the helper function to alwas
*/