#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <limits.h>
#include <unistd.h>
#include <sys/stat.h>

int g_buffer[2];
char *g_fifo;

void recieveInput();
void sendOutput(int result);

int main(int argc, char *argv[]){
    // get fifo from arguments
    g_fifo = argv[1];
    
    // main loop
    while (1){
        int result;

        // reads input from FIFO
        recieveInput();

        // checks if the input is the exit signal
        if(g_buffer[0] == INT_MIN && g_buffer[1] == INT_MIN){
            break;
        }

        // performs the operation
        result = g_buffer[0] - g_buffer[1];

        // writes result to FIFO
        sendOutput(result);
    }
    exit(EXIT_SUCCESS);
}

// writes output to pipe
void sendOutput(int result){
    // open fifo
    int fd;
    fd = open(g_fifo, O_WRONLY);
    if(fd == -1){
        
        // error handling
        perror("opening fifo in adder subprogram");
        exit(EXIT_FAILURE);
    }

    // write result into fifo    
    if(write(fd, &result, sizeof(int)) == -1){

        // error handling
        perror("writing to fifo in adder subprogram");
        exit(EXIT_FAILURE);
    }
    
    // close fifo
    close(fd);
}

// reads input from pipe
void recieveInput(){
    // open fifo
    int fd;
    fd = open(g_fifo, O_RDONLY);
    if(fd == -1){

        // error handling
        perror("fifo open error in adder subprogram");
        exit(EXIT_FAILURE);
    }
    // read from fifo
    if(read(fd, g_buffer, sizeof(g_buffer)) == -1){

        // error handling
        perror("reading fifo in adder subprogram");
        exit(EXIT_FAILURE);
    }

    // close fifo
    close(fd);
}