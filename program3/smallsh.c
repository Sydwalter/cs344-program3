/*****************************************************************************
 ** Filename:    smallsh.c
 ** Author:      Scott Milton
 ** Date:        11/23/15
 **
 ** Description: This program is a small shell that runs command line
 **              instructions and returns results. It allows redirection of
 **              standard input and standard output and supports both
 **              foreground and background processes. The shell supports three
 **              built in commands: exit, cd, and status. It also supports
 **              comments, which are lines beginning with the # character.
 **
 ** Input:       from the keyboard: type char*
 **              from files:        type char*
 **
 ** Output:      to the console and files : type char*, int
 **              to files:                  type char*, int
 **
 **
 **
 *****************************************************************************/

#include <errno.h>     // for errno
#include <signal.h>
#include <stdio.h>     // for fgets (, fopen, fclose, fseek)
#include <stdlib.h>    // for getenv, malloc, free (, rand and srand)
#include <string.h>    // for strcpy, strcat
#include <sys/types.h> // for pid_t
#include <sys/wait.h>  // for waitpid
#include <unistd.h>    // for exec (,getpid)

/*
#include <unistd.h>    // getpid
#include <sys/stat.h>  // for stat
#include <time.h>      // for time
#include <fcntl.h>     // for open
*/

#define DEBUG             0 

#define MAX_ARGS        512 // maximum arguments accepted on command line
#define MAX_LENGTH     2048 // maximum length for a command line

/*
#define BUFFER_SIZE     512 // used for I/O operations
*/

// define bool as type
typedef enum { false, true } bool;



int main(int argc, char** argv) {

    // declare variables
//    bool isBackgroundProcess = false;
    bool repeat = true;
//    char args[MAX_ARGS + 1][MAX_LENGTH];
    char* args[MAX_ARGS + 1];
    char input[MAX_LENGTH];
    char* token;
    pid_t bgpid;
    pid_t cpid;
    int bgExitStatus;
    int exitStatus;
    int i;
    int numArgs;
    int bgStatus;
    int status;

    // allocate memory for arg array
    for (i = 0; i <= MAX_ARGS; i++)
    {
        args[i] = (char*) malloc((MAX_LENGTH + 1) * sizeof(char)); 
    }  

    do
    {
       do
        {
            // check to see if any bg process completed by using waitpid
            bgpid = waitpid(-1, &bgStatus, 0); // WNOHANG);

            // if so print process id and exit status
            if (bgpid > 0 && WIFEXITED(bgStatus)) 
            {
                bgExitStatus = WEXITSTATUS(bgStatus);
                printf("process %d exited with exit status of %d.\n", bgpid, bgExitStatus);
            }
        }
        while (bgpid > 0); // continue until all completed bg processes reporte

        // POINT A
        // if command is bg process
        // then print process id when begins
        // when bg process terminates
        // then print process id and exit status

        // flush out prompt each time it is printed
        fflush(stdout);

        // prompt user for input
        printf(": ");

        // get user input
        fgets(input, MAX_LENGTH, stdin);

        // validate input?
            // do not need to do error checking for syntax

        // check for blank line
        if (input[0] == '\n')
        {
            continue;
        }

        // process and parse input
        numArgs = 0;
        token = strtok(input, " "); // check for multiple args

        if (DEBUG)
        {
            printf("token is %s\n", token); 
        }

        // loop to process args (up to 512)
        while (token != NULL && numArgs < MAX_ARGS)  
        {

            if (DEBUG)
            {
                printf("prior to strcpy: strlen(args[%d]) = %d, strlen(token) = %d\n", numArgs, strlen(args[numArgs]), strlen(token)); 
            }

            // copy current arg to arg array
            strcpy(args[numArgs], token); 

            if (DEBUG)
            {
                printf("args[%d] is: %s\n", numArgs, args[numArgs]); 
            }

            // increment counter
            numArgs++;
 
            // get next arg, if any
            token = strtok(NULL, " ");
        }

        // remove newline char from last arg
        token = strtok(args[numArgs - 1], "\n");
        strcpy(args[numArgs - 1], token);

        if (DEBUG)
        {
            printf("args[%d] is: %s\n", numArgs - 1, args[numArgs - 1]); 
        }

        // add NULL to array index after last arg to signal end of args
 //       strcpy(args[numArgs], "\0"); 
        args[numArgs] = NULL;

        // be better to remove leading space(s) before 1st command
            // implement this if time permits

        // if one of three built in commands
        // do not need to support I/O redirection
        // do not have to set any exit status

        if (strncmp(args[0], "#", 1) == 0)
        {
            // do nothing
        }
        else if (strcmp(args[0], "exit") == 0)
        {
            // if command is exit
            // then kill any processes or jobs that shell has started
            kill(0, SIGTERM);       

            // free allocated memory
            for (i = 0; i <= MAX_ARGS; i++)
            {
                free(args[i]); 
            }  

            // and then exit the shell
            repeat = false;
        }
        else if (strcmp(args[0], "cd") == 0)
        {
            // change working directories

            // if no args, change to directory specified in HOME env var
            if (numArgs == 1)
            {
                chdir(getenv("HOME"));
            }
            // if one arg, change to dir provided
            else
            {
                chdir(args[1]);
            }
            // support absolute and relative paths
        }
        else if (strcmp(args[0], "status") == 0)
        {
            // if command is status
            // then print exit status or terminating signal of last fg command
            // does this have to be manually stored by the program ahead of time
            // or is there some standard built-in functionality to get it?
            // should it check the last one prior to this iteration or
            // should it check the exit status of the last one on the next iteration
            // and if nothing there then fall back to the last one from whenever?    

            if (WIFEXITED(status)) // lecture 9 page 5
            {
                exitStatus = WEXITSTATUS(status);
                printf("exit value %d\n", exitStatus);
            }
            else
            {
                printf("terminating signal was \n");
            } 
            // no other option??
            // what if no prcesses have been created??
            // then neither case would apply, right?
        }
        else
        {
            // pass through to BASH to interpret command there

            // need fork and exec
            cpid = fork();

            if (cpid == 0) // child process
            {
 //               printf("child process running exec: %s\n", token);

                // exec using path version in order to use Linux built-ins
 //               execlp(args[0], args[0], NULL);
                execvp(args[0], args);

                // will never run unless error (i.e.- bad filename)
                printf("%s:", args[0]);
                fflush(stdout);
                perror(" ");  
 //               printf("\n");
                return(1); // end child process
            }
            else if (cpid == -1) // parent process
            {   
                // if unable to fork print error
                printf("%s:", args[0]);
                fflush(stdout);  
                perror(" ");
            } 
            else
            {
                // parent process continues here

                // wait for child process if fg
                waitpid(cpid, &status, 0);
            }
        }

    } // repeat until user exits shell
    while(repeat == true);

    // use fork, exec, and waitpid to execute commands
    // conditional check: foreground or background
    // shell will wait for fg commands before prompting for next command
    // shell will not wait for bg commands before prompting for next command
    // conditional check: did user specify file to take standard input?
    // if so, use that
    // if not, bg command redirect standard input from /dev/null

    // use the PATH variable to look for commands
    // allow shell scripts to be executed
    // the right version of exec will do this automatically
    // if command fails b/c shell could not find command to run
    // then print error message
    // exit with status 1

    // after fork
    // input/output redirection
    // redirected input file is opened for reading only
    // if cannot open file for reading
    // then print error message
    // and exit with status 1
    // before exec


    // if command is terminated by signal
    // then print message indicating which signal terminated the process

    // catch CTRL-C interrupts from keyboard
    // make sure they do not terminate shell or bg commands, but only fg command

    // any other command is passed on to member of exec() family of functions

    // if command is blank line or comment
    // then ignore it, do nothing, and reprompt for another command

    return 0;
}
