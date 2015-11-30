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
#include <limits.h>
#include <signal.h>    // for sigset_t
#include <stdio.h>     // for fgets (, fopen, fclose, fseek)
#include <stdlib.h>    // for getenv, malloc, free (, rand and srand)
#include <string.h>    // for strcpy, strcat
#include <sys/types.h> // for pid_t
#include <sys/wait.h>  // for waitpid
#include <unistd.h>    // for exec (,getpid)

#define DEBUG             0 // change to 1 for debugging print statements 
#define MAX_ARGS        512 // maximum arguments accepted on command line
#define MAX_LENGTH     2048 // maximum length for a command line

// define bool as type
typedef enum { false, true } bool;


// declare global variables
bool childCompleted = false;
int bgStatus; 
pid_t bgpid[1000];         // array of open background process IDs
pid_t completed_pid[1000]; // array of completed bg process IDs



// add comment block here
void bgHandler(int sig, siginfo_t* info, void* vp);



int main(int argc, char** argv)
{
    // declare variables
    bool isBackgroundProcess = false;
    bool repeat = true;
    char *args[MAX_ARGS + 1];
    char input[MAX_LENGTH];
    char *token;
    pid_t cpid;
    pid_t fgpid;
    int bgExitStatus;
    int exitStatus;
    int i;
    int numArgs;
    int status;

    // create instance of sigaction struct
    struct sigaction background_act;
    background_act.sa_sigaction = bgHandler; // SIG_IGN    
    background_act.sa_flags = SA_SIGINFO|SA_RESTART;
    sigemptyset(&(background_act.sa_mask));

    // set up signal handler for completed child process
    sigaction(SIGCHLD, &background_act, NULL);

    // initialize first indices of process arrays
    completed_pid[0] = bgpid[0] = INT_MAX; 

    // allocate memory for arg array
    for (i = 0; i <= MAX_ARGS; i++)
    {
        args[i] = (char*) malloc((MAX_LENGTH + 1) * sizeof(char)); 
    }  

    do
    {
        // create array of pointers to the strings in the arg array
        char **next = args;

        // clear input buffer each iteration
        strcpy(input, "\0");
 
//       do
        if (completed_pid[0] != INT_MAX)
        {
            // check to see if any bg process completed by using waitpid
            completed_pid[0] = waitpid(completed_pid[0], &bgStatus, 0); // WNOHANG);

            // if so print process id and exit status
            if (WIFEXITED(bgStatus)) 
            {
                bgExitStatus = WEXITSTATUS(bgStatus);
                printf("background pid %d is done: exit value %d.\n", completed_pid[0], bgExitStatus);
            }
            // reset the value of the boolean variable
//            childCompleted = false;
            completed_pid[0] = INT_MAX;
        }
//        while (bgpid > 0); // continue until all completed bg processes reporte

        // flush out prompt each time it is printed
//        fflush(stdin);
//        fflush(stdout);

        // prompt user for input
        printf(": ");

        // get user input
//        do
//        {
            fgets(input, MAX_LENGTH, stdin);
//        }
//        while (input == NULL);

        fflush(stdin);

        // check for blank line
        if (input[0] == '\n' || input[0] == '\0')
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
            *next = token;

            if (DEBUG)
            {
                printf("args[%d] is: %s\n", numArgs, args[numArgs]); 
            }

            // increment argument  counter
            numArgs++;
 
            // get next arg, if any
            token = strtok(NULL, " ");

            // increment pointer unless last iteration
            if (token != NULL)
            {
                *next++;
            } 
        }

        // remove newline char from last arg
        *next = strtok(*next, "\n"); 

        if (DEBUG)
        {
            printf("args[%d] is: %s\n", numArgs - 1, args[numArgs - 1]); 
        }

        // if command is bg process
        if (strcmp(args[numArgs - 1], "&") == 0)
        {
            // set variable appropriately for later 
            isBackgroundProcess = true;

            // decrement number of args since ampersand will be removed
            numArgs--; 

            // do not increment pointer so that ampersand is removed  
        }
        else
        {
            // increment pointer
            *next++;
        }
         // add NULL to array index after last arg to signal end of args
        *next = NULL;

        if (strncmp(args[0], "#", 1) == 0)
        {
            // do nothing for comments
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
        else // pass through to BASH to interpret command there
        {
            // need fork and exec
            cpid = fork();

            if (cpid == 0) // child process
            {
                // exec using path version in order to use Linux built-ins
                execvp(args[0], args);

                // will never run unless error (i.e.- bad filename)
                printf("%s:", args[0]);
//                fflush(stdin);
//                fflush(stdout);
                perror(" ");  
 
                return(1); // end child process
            }
            else if (cpid == -1) // parent process
            {   
                // if unable to fork print error
                printf("%s:", args[0]);
//                fflush(stdin);
//                fflush(stdout);  
                perror(" ");
            } 
            else
            {
                // parent process continues here

                // if command is bg process
                if (isBackgroundProcess == true)
                {
                    // then print process id when begins
                    printf("background pid is %d\n", cpid);

                    // reset boolean value for next iteration
                    isBackgroundProcess = false;

                    // add process id to array of background processes
                    bgpid[0] = cpid;

                } 
                else
                {
                    // wait for fg child process
                    fgpid = waitpid(cpid, &status, 0);
                }
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



void bgHandler(int sig, siginfo_t* info, void* vp)
{
    pid_t ref_pid = info->si_pid; 

    // cycle through array of open bg processes and look for match 

    // if match found, move pid to array of completed bg processes
    if (ref_pid == bgpid[0])
    {
        completed_pid[0] = bgpid[0];
 
        bgpid[0] = INT_MAX;
    } 

    return;
}

