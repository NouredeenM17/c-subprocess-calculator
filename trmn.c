#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <limits.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>

#define NO_OF_FIFOS 4
#define MENU "\n****************\n1- Addition\n2- Subtraction\n3- Multiplication\n4- Division\n5- Exit\n\n"

char *g_fifos[] = {"/tmp/adder","/tmp/subtractor","/tmp/multiplier","/tmp/divider"};
char *g_prog_names[] = {"adder", "subtractor", "multiplier", "divider"};
pid_t g_pids[NO_OF_FIFOS];


void performOperation(int operation_num);
int* getUserInputArr();
int get1UserInput(const char *prompt);
int getMenuUserInput();
void handleMenuInput1to4(int user_input);
void forkProcess(int operation_num);
void initPipe(int operation_num);
void writeToPipe(const char *prog_name, const char *fifo, int *sent);
int readFromPipe(const char *prog_name, const char *fifo);
void ensureNoDivisionByZero(int *user_input, int operation_num);
void printOperationResult(int *user_input, int operation_num, int result);
void handleError(const char *prompt1, const char *prompt2);
void setup();
void terminate();


// main function
int main(){
    int user_input;

    // sets up named pipes (FIFO) and runs all sub-programs
    setup();

    // main loop
    while(1){
        // prints menu
        printf(MENU);

        // gets input and ensures it is an int, and ensures it is between 1 and 5
        user_input = getMenuUserInput();

        // checks if the exit option was selected
        if(user_input == 5){
            terminate();
            break;
        }

        // handles user input between 1 and 4
        handleMenuInput1to4(user_input);
            
    }
    return 0;
}

// runs at startup, initializes named pipes (FIFO) for each process and creates a fork to run each process
void setup(){
    
    for (int i = 0; i < NO_OF_FIFOS; i++){
        initPipe(i);
        forkProcess(i);
    }
    
}

// initializes the FIFO of specified operation number
void initPipe(int operation_num){

    // delete pipe FIFO file if it already exists
    if(access(g_fifos[operation_num], F_OK) != -1){
        remove(g_fifos[operation_num]);
    }

    // create FIFO
    if(mkfifo(g_fifos[operation_num], 0666) == -1){

        // error handling
        handleError("mkfifo in ", g_prog_names[operation_num]);
    }
}


// forks the main process and executes the specified operation number
void forkProcess(int operation_num){
    
    // forks child process to use for sub-program
    pid_t pid;
    pid = fork();

    if(pid == -1){

        // error handling
        handleError("fork in ", g_prog_names[operation_num]);
    }

    // child process
    if(pid == 0){
        // stores pid of the child process
        g_pids[operation_num] = getpid();

        // runs sub-program and sends fifo information to it in args
        char *sent_args[] = {g_prog_names[operation_num], g_fifos[operation_num], NULL};
        execv(g_prog_names[operation_num], sent_args);

    }
}

// gets user input while in menu, ensures input is an int between 1 and 5
int getMenuUserInput(){
    int user_input;
    while (1){
        user_input = get1UserInput("Choice: ");
        if(user_input > 5 || user_input < 1){
            printf("Invalid input! Please enter a number between 1 and 5.\n\n");
        } else {
            break;
        }
    }
    return user_input;
}

// gets 1 user input with a prompt, and ensures it is an int
int get1UserInput(const char *prompt){
    int result;

    // gets valid input from user
    while(1){
        printf("%s", prompt);
        if(scanf("%d", &result) == 0){
            printf("Invalid input! Please enter an integer value.\n\n");
            
            // clear input buffer
            while(getchar() != '\n');
        } else {
            break;
        }
    }
    return result;
}

// runs when exiting, sends exit signal to subprograms and unlinks all named pipes (FIFO) 
// and waits for subprograms to terminate
void terminate(){
    // creates exit signal which is a 2 element array where both values are INT_MIN
    int *exit_signal = malloc(2 * sizeof(int));
    exit_signal[0] = INT_MIN;
    exit_signal[1] = INT_MIN;

    // sends exit signal to each sub-program, unlinks all named pipes (FIFO) 
    for(int i = 0; i < NO_OF_FIFOS; i++){  
        writeToPipe(g_prog_names[i], g_fifos[i], exit_signal);
        unlink(g_fifos[i]);

        // waits for subprograms to terminate
        int status;
        waitpid(g_pids[i], &status, 0);
    }

    
   
    

    // frees dynamically allocated array
    free(exit_signal);
}

// handles menu input from 1 to 4
void handleMenuInput1to4(int user_input){
    // handles user input from 1 to 4, performs selected operation and prints its title 
    switch(user_input){
        case 1:
            printf("\n****************\nAddition:\n");
            performOperation(0);
            break;
        case 2:
            printf("\n****************\nSubtraction:\n");
            performOperation(1);
            break;
        case 3:
            printf("\n****************\nMultiplication:\n");
            performOperation(2);
            break;
        case 4:
            printf("\n****************\nDivision:\n");
            performOperation(3);
            break;
    }
}

// performs operation according to operation number
void performOperation(int operation_num){
    int* user_input;
    int result;

    // gets user input
    user_input = getUserInputArr();

    // checks for division by zero
    ensureNoDivisionByZero(user_input, operation_num);
    
    // writes to pipe
    writeToPipe(g_prog_names[operation_num], g_fifos[operation_num], user_input);

    // reads result from pipe
    result = readFromPipe(g_prog_names[operation_num], g_fifos[operation_num]);

    // changes the printf statement according to operation
    printOperationResult(user_input, operation_num, result);
    
    // free dynamically allocated user input array
    free(user_input);
}

// gets 2 user inputs to use in operations, returns a dynamically allocated array
int *getUserInputArr(){

    // dynamically allocate 2 element int array for input
    int* result = malloc(2 * sizeof(int));

    result[0] = get1UserInput("Input 1: ");
    result[1] = get1UserInput("Input 2: ");

    return result;
}

// ensures the second user input is not zero
void ensureNoDivisionByZero(int *user_input, int operation_num){
    // checks for division by zero
    while(1){
        if(operation_num == 3 && user_input[1] == 0){
            printf("You cannot divide by zero! Please enter a non-zero value.\n\n");
            user_input[1] = get1UserInput("Input 2: ");
        } else {
            break;
        }
    }
}

// writes to specified pipe
void writeToPipe(const char *prog_name, const char *fifo, int *sent){
    int fd;

    // open fifo for writing
    fd = open(fifo, O_WRONLY);
    if(fd == -1){

        // error handling
        handleError("opening fifo in ", prog_name);
    }

    // write result into fifo    
    if(write(fd, sent, sizeof(sent)) == -1){

        // error handling
        handleError("writing to fifo in ", prog_name);
    }

    // close fifo
    close(fd);
}

// reads from specified pipe
int readFromPipe(const char *prog_name, const char *fifo){
    int fd;
    int buffer;

    // open fifo for reading
    fd = open(fifo, O_RDONLY);
    if(fd == -1){

        // error handling
        handleError("opening fifo in ", prog_name);
    }

    // read from fifo
    if(read(fd, &buffer, sizeof(int)) == -1){

        // error handling
        handleError("reading fifo in ", prog_name);
    }
    
    // close fifo
    close(fd);

    return buffer;
}

// prints result of an operation
void printOperationResult(int *user_input, int operation_num, int result){
    // changes the printf statement according to operation
    switch(operation_num){
        case 0:
            printf("%d + %d = %d\n", user_input[0], user_input[1], result);
            break;
        case 1:
            printf("%d - %d = %d\n", user_input[0], user_input[1], result);
            break;
        case 2:
            printf("%d * %d = %d\n", user_input[0], user_input[1], result);
            break;
        case 3:
            printf("%d / %d = %d\n", user_input[0], user_input[1], result);
    }
}

// prints error message with perror and combines 2 prompts to specify the location of the error, then exits the program
void handleError(const char *prompt1, const char *prompt2){
        char err_msg[50];
        sprintf(err_msg, "%s%s", prompt1, prompt2);
        perror(err_msg);
        exit(EXIT_FAILURE); // returns 1
}




