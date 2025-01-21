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
int reverse_string(char *, int, int);
int word_print(char *, int, int);


int setup_buff(char *buff, char *user_str, int len){
    //TODO: #4:  Implement the setup buff as per the directions
    int stringLen = 0;
    char *inputPtr = user_str;
    char *outputPtr = buff;
    int spaceFound = 0; 

    while(inputPtr[stringLen] != '\0'){
        stringLen++;
    }

    inputPtr = user_str;

    while(*inputPtr == ' ' || *inputPtr == '\t'){
        inputPtr++;
        stringLen--;
    }

    while(*inputPtr != '\0'){
        if(*inputPtr == ' ' || *inputPtr == '\t'){
            if(!spaceFound){
                *outputPtr = ' ';
                outputPtr++;
                spaceFound = 1;
            }
        } else {
            *outputPtr = *inputPtr;
            outputPtr++;
            spaceFound = 0;
        }
        inputPtr++;
    }

    while(outputPtr < buff + len){
        *outputPtr = '.';
        outputPtr++;
    }

    return stringLen;
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
    int count = 0;
    int letter = 0;
    char *pointer = buff;

    for(int i = 0; i < str_len; i++){
        if(*pointer == ' '){
            letter = 0;
        } else {
            if(!letter){
                count++;
                letter = 1;
            }
        }
        pointer++;
    }
    return count;
}

//ADD OTHER HELPER FUNCTIONS HERE FOR OTHER REQUIRED PROGRAM OPTIONS
int reverse_string(char *buff, int len, int str_len){
    char *reversed = malloc(str_len + 1);
    int j = 0;
    for (int i = str_len - 1; i >= 0; i--) {
        reversed[j] = buff[i];
        j++;
    }
    reversed[str_len] = '\0';
    printf("Reversed String: %s\n", reversed);
    return 0;
} 

int word_print(char *buff, int len, int str_len){
    printf("Word Print\n");
    printf("----------\n");
    char *start = buff;
    int wordNum = 1;
    int numChars = 0;
    char *next = start + 1;
    while(*start && (start - buff) < str_len){
        if(*start != ' '){
            numChars++;
            if(*next == ' ' || *next == '.' || *next == '\0'){
                printf("%d. ", wordNum++);
                char *temp = start - numChars + 1;
                while(temp <= start){
                    putchar(*temp++);
                }
                printf(" (%d)\n",numChars);
                numChars = 0;
            }
        }
        start++;
        next++;
    }
    printf("Number of words returned: %d\n", wordNum - 1);
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
    /*
        Thia is safe because the second condition of the if statement that calls for arv[1] will only execute if there
        is more than 1 argument. If there at least 1 argument, then arv[1] must exist.
    */
    if ((argc < 2) || (*argv[1] != '-')){
        usage(argv[0]);
        exit(1);
    }

    opt = (char)*(argv[1]+1);   //get the option flag

    //handle the help flag and then exit normally
    if (opt == 'h'){
        usage(argv[0]);
        exit(0);
    }

    //WE NOW WILL HANDLE THE REQUIRED OPERATIONS

    //TODO:  #2 Document the purpose of the if statement below
    //      PLACE A COMMENT BLOCK HERE EXPLAINING
    /*
        The if statement checks if there are lesss than 3 arguments in the command line which is needed to properly
        execute the files purpose. If there are less than 3 arguments then it will exit the program with the error 
        message that there is a command line issue. 
    */
    if (argc < 3){
        usage(argv[0]);
        exit(1);
    }

    input_string = argv[2]; //capture the user input string

    //TODO:  #3 Allocate space for the buffer using malloc and
    //          handle error if malloc fails by exiting with a 
    //          return code of 99
    // CODE GOES HERE FOR #3
    buff = malloc(BUFFER_SZ);
    if (buff == NULL) {
        exit(99);
    }

    user_str_len = setup_buff(buff, input_string, BUFFER_SZ);     //see todos
    if (user_str_len < 0){
        printf("Error setting up buffer, error = %d", user_str_len);
        exit(2);
    }

    switch (opt){
        case 'c':
            rc = count_words(buff, BUFFER_SZ, user_str_len);  //you need to implement
            if (rc < 0){
                printf("Error counting words, rc = %d", rc);
                exit(2);
            }
            printf("Word Count: %d\n", rc);
            break;

        //TODO:  #5 Implement the other cases for 'r' and 'w' by extending
        //       the case statement options
        case 'r':
            rc = reverse_string(buff, BUFFER_SZ, user_str_len);
            if (rc < 0){
                printf("Error reversing words, rc = %d", rc);
                exit(2);
            }
            break;
        case 'w':
            rc = word_print(buff, BUFFER_SZ, user_str_len);
            if (rc < 0){
                printf("Error printing words, rc = %d", rc);
                exit(2);
            }
        case 'x':
            if(argc != 5){
                printf("Not implemented!\n");
                exit(2);
            }
            printf("Not implemented!\n");
            exit(2);
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
 /*
    Providing both the pointer and the length is a good practice because it allows for overflow protection and flexibility. 
    The function accidentally trying to access beyond the allocated space is prevented when the length is passed explicitly to the buffer. 
    Along with this if the buffer size changes in the future, the functions don't have to be updated since 50 bytes is not hard coded into them.
 */