#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    int pipeToSquare[2], pipeToPlus[2], pipeToParent[2];
    int squareStatus, plusStatus;
    pid_t square, plus;

    // Create the pipes
    if(pipe(pipeToSquare) == -1 || pipe(pipeToPlus) == -1 || pipe(pipeToParent) == -1) {
        perror("Pipe creation failure");
        return 1;
    }

    square = fork();

    if(square == -1) {
        perror("Fork failure");
        return 1;
    }

    if (square == 0) {
        // This is the first child process
        // close unused pipes
        close(pipeToSquare[1]);
        close(pipeToPlus[0]);
        close(pipeToParent[0]);
        close(pipeToParent[1]);
        int value;
        while (read(pipeToSquare[0], &value, sizeof(value)) > 0) {
            value *= value; // Square the value
            write(pipeToPlus[1], &value, sizeof(value));
        }
        // close used pipes
        close(pipeToSquare[0]);
        close(pipeToPlus[1]);
    } else {
        
        plus = fork();

        if(plus == -1) {
            perror("Fork failure");
            return 1;
        }

        if (plus == 0) {
            // This is the second child process
            // close unused pipes
            close(pipeToSquare[0]);
            close(pipeToSquare[1]);
            close(pipeToPlus[1]);
            close(pipeToParent[0]);
            int value;
            while (read(pipeToPlus[0], &value, sizeof(value)) > 0) {
                value += 1; // Add one to the value
                write(pipeToParent[1], &value, sizeof(value));
            }
            //close used pipes
            close(pipeToPlus[0]);
            close(pipeToParent[1]);
        } else {
            // This is the parent process
            // close unused pipes
            close(pipeToSquare[0]);
            close(pipeToPlus[0]);
            close(pipeToPlus[1]);
            close(pipeToParent[1]);
            int value;
            int status;
            while ((status = scanf("%d", &value)) != EOF) {
                if (status == 1) {
                    write(pipeToSquare[1], &value, sizeof(value));
                    read(pipeToParent[0], &value, sizeof(value));
                    printf("%d\n", value);
                } else {
                    printf("Invalid input. Please enter an integer.\n");
                    
                    //ignore new line input
                    while(getchar() != '\n');
                }
            }

            // Close the used pipes and wait for the children to exit
            close(pipeToSquare[1]);
            close(pipeToParent[0]);
            waitpid(square, &squareStatus, 0);
            waitpid(plus, &plusStatus, 0);
        }
    }
    return 0;
}
