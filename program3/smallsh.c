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
#include <stdio.h>     // for fgets (, fopen, fclose, fseek)
#include <string.h>    // for strcpy, strcat

/*
#include <unistd.h>    // for getpid
#include <sys/types.h> // for pid_t
#include <stdlib.h>    // for rand and srand
#include <sys/stat.h>  // for stat
#include <time.h>      // for time
#include <fcntl.h>     // for open
*/

#define MAX_ARGS        512 // maximum arguments accepted on command line
#define MAX_LENGTH     2048 // maximum length for a command line

/*
#define BUFFER_SIZE     512 // used for I/O operations
*/

// define bool as type
typedef enum { false, true } bool;



int main(int argc, char** argv) {

    // declare variables
    bool repeat = true;
    char command[MAX_LENGTH];
//    int argCount;

    // conditional check: is this child process
    // if so, behave appropriately

    do
    {

        // check to see if any bg process completed
            // by using waitpid
            // see point A below
        // print prompt ": symbol"
        // flush out prompt each time it is printed

        // get input

        // validate input?
            // do not need to do error checking for syntax


        // prompt user for input
        printf(": ");

        // get user input
        fgets(command, MAX_LENGTH, stdin);

        // loop to process args (up to 512)
            // is there prebuilt way to get / refer to args, similar to shell script?
        // make sure not more than max length for command line
            // error if too long? or just truncate and ignore excess?


        // parse user input and remove newline character at end of string
        strtok(command, "\n");

        printf("ok\n");

    } // repeat until user exits shell
    while(repeat == true);

    // respond appropriately to valid input

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

    // if one of three built in commands
        // do not need to support I/O redirection
        // do not have to set any exit status

    // if command is exit
    // then kill any processes or jobs that shell has started
    // and then exit the shell

    // if command is status
    // then print exit status or terminating signal of last fg command

    // if command is terminated by signal
    // then print message indicating which signal terminated the process

    // POINT A
    // if command is bg process
    // then print process id when begins
    // when bg process terminates
    // then print process id and exit status

    // catch CTRL-C interrupts from keyboard
    // make sure they do not terminate shell or bg commands, but only fg command

    // if command is cd
    // then change directories
    // if no args, change to directory specified in HOME env var
    // if one arg, change to dir provided
        // support absolute and relative paths
    // this is working directory
    // when process exits

    // any other command is passed on to member of exec() family of functions

    // if command is blank line or comment
    // then ignore it, do nothing, and reprompt for another command

    return 0;
}
