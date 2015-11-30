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
#include <fcntl.h>     // for open
#include <limits.h>    // for INT_MAX
#include <signal.h>    // for sigset_t
#include <stdio.h>     // for fgets (, fopen, fclose, fseek)
#include <stdlib.h>    // for getenv, malloc, free (, rand and srand)
#include <string.h>    // for strcpy, strcat
#include <sys/stat.h>  // for stat 
#include <sys/types.h> // for pid_t
#include <sys/wait.h>  // for waitpid
#include <unistd.h>    // for exec (,getpid)

#define DEBUG             0 // change to 1 for debugging print statements 
#define MAX_ARGS        512 // maximum arguments accepted on command line
#define MAX_LENGTH     2048 // maximum length for a command line
#define MAX_PIDS       1000 // maximum PIDs to track

// define bool as type
typedef enum { false, true } bool;


// declare global variables
// bool childCompleted = false;
int completed_cur = 0;
int cur = 0;                   // index to add next bg process in bgpid[]
int signalNum = 0;
pid_t bgpid[MAX_PIDS];         // array of open background process IDs
pid_t completed_pid[MAX_PIDS]; // array of completed bg process IDs
pid_t fgpid;                   // running foreground process



// add comment block here
void bgHandler(int sig, siginfo_t* info, void* vp);



// add comment block here
void sigintHandler(int sig, siginfo_t* info, void* vp);



int main(int argc, char** argv)
{
    // declare variables
    bool isBackgroundProcess = false;
    bool repeat = true;
    char *args[MAX_ARGS + 1];
    char input[MAX_LENGTH];
    char *token;
    pid_t cpid;
    int bgExitStatus;
    int bgStatus; 
    int exitStatus;
    int fd;
    int fd2;
    int i;
    int j;
    int numArgs;
    int status;

    // create instance of sigaction struct for background processes
    struct sigaction background_act;
    background_act.sa_sigaction = bgHandler;     
    background_act.sa_flags = SA_SIGINFO|SA_RESTART;
    sigemptyset(&(background_act.sa_mask));
    // set up signal handler for completed child process
    sigaction(SIGCHLD, &background_act, NULL);

    // create instance of sigaction struct for foreground processes
    struct sigaction foreground_act;
    foreground_act.sa_sigaction = sigintHandler;
    foreground_act.sa_flags = SA_SIGINFO|SA_RESTART;
    sigemptyset(&(foreground_act.sa_mask));
    sigaction(SIGINT, &foreground_act, NULL); 

    // initialize arrays for bg processes
    for (i = 0; i < MAX_PIDS; i++)
    {
        completed_pid[i] = bgpid[i] = INT_MAX;
    }   

    // allocate memory for arg array
    for (i = 0; i <= MAX_ARGS; i++)
    {
        args[i] = (char *) malloc((MAX_LENGTH + 1) * sizeof(char)); 

        if (DEBUG)
        {
            strcpy(args[i], "filler");
            printf("args[%d] = %s\n", i, args[i]);  
        } 
    }  

    do
    {
        // create array of pointers to the strings in the arg array
        char **next = args;

        // clear input buffer each iteration
        strcpy(input, "\0");
 
        i = 0;

        // this loop cleans up zombies, waiting for all ps in completed array
        while (i < MAX_PIDS && completed_pid[i] != INT_MAX)
        {
            if (DEBUG) 
            {
                printf("Now cleaning up process %d\n", completed_pid[i]);
            }

            // wait on current process
            completed_pid[i] = waitpid(completed_pid[i], &bgStatus, 0);

            // print process id and exit status
            if (WIFEXITED(bgStatus)) 
            {
                bgExitStatus = WEXITSTATUS(bgStatus);
                printf("background pid %d is done: exit value %d.\n", completed_pid[i], bgExitStatus);
            }

            // remove current ps from open background process array
            j = 0;
            while (j < MAX_PIDS && bgpid[j] != INT_MAX)
            { // cycle through array of bg processes
                // and look for match 
                if (bgpid[j] == completed_pid[i])
                {
                    if (DEBUG)
                    {
                        printf("Now removing process %d from array.\n", bgpid[j]);
                    }                   

                    // replace value of current bg process 
                    bgpid[j] = INT_MAX;
 
                    // shift all subsequent PIDs down to fill `gap`
                    int k = j;                       
                    while (k + 1 < MAX_PIDS && bgpid[k+1] != INT_MAX)
                    {
                        bgpid[k] = bgpid[k+1];
                        bgpid[k+1] = INT_MAX;
                        k++;
                    }    
                    // adjust cur index value & make room for new PID  
                    cur--; 
                }
                // increment counter
                j++;
            }

            // replace value of current completed process
            completed_pid[i] = INT_MAX;

            // increment counter
            i++; 
        }

        // reset completed bg process array index tracker
        completed_cur = 0;

        // flush out prompt each time it is printed
        fflush(stdin);
        fflush(stdout);

        // prompt user for input
        printf(": ");

        // get user input
        fgets(input, MAX_LENGTH, stdin);

        // flush out prompt
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
                printf("overwriting %s with %s\n", *next, token);
            }

            // copy current arg to arg array
            strcpy(*next, token);

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

        if (DEBUG)
        {
            printf("overwriting %s", *next);
        }

        // remove newline char from last arg
        token = strtok(*next, "\n"); 

        strcpy(*next, token);

        if (DEBUG)
        {
            printf(" with %s\n", *next);
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

        if (DEBUG)
        {
            printf("overwriting %s with NULL\n", *next);
        }

        // add NULL to array index after last arg to signal end of args
//        strcpy(*next, "\0");
//        char *temp = *next; // save pointer to dynamic memory for later
//        *next = NULL;

        if (strncmp(args[0], "#", 1) == 0)
        {
            // do nothing for comments
        }
        else if (strcmp(args[0], "exit") == 0)
        {

            // kill any processes or jobs that shell has started
            i = 0;
            while (i < MAX_PIDS && bgpid[i] != INT_MAX)
            {
                if (DEBUG)
                {
                    printf("Now killing process %d\n", bgpid[i]);
                }
 
                kill(bgpid[i], SIGKILL);
                i++;
            }

            // restore the pointer to allocated memory
//            *next = temp; 

            // free allocated memory
            for (i = 0; i <= MAX_ARGS; i++)
            {
                if (DEBUG)
                {
                    printf("Now freeing memory for args[%d], which has a value of %s\n",i, args[i]);
                } 
                free(args[i]); 
            }  

            // then kill any processes or jobs that shell has started
            // kill(0, SIGTERM);       

            // exit the shell
            repeat = false;

        }
        else if (strcmp(args[0], "cd") == 0)
        { // change working directories

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
        { // print exit status or terminating signal of last fg command

            if (WIFEXITED(status))
            {
                exitStatus = WEXITSTATUS(status);
                printf("exit value %d\n", exitStatus);
            }
            else if (signalNum != 0)
            {
                printf("terminating signal was %d\n", signalNum);
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
                // check to see if command is for bg process
                if (isBackgroundProcess == true)
                {
                    // if so, redirect stdin to /dev/null
                    fd = open("/dev/null", O_RDONLY);
                    if (fd == -1)
                    {
                        perror("open");
                        exit(1); 
                    }

                    fd2 = dup2(fd, 0);
                    if (fd2 == -1)
                    {
                        perror("dup2");
                        exit(1);
                    }   
                }

                // add NULL pointer to last array index (only in child)
                *next = NULL;

                // exec using path version in order to use Linux built-ins
                execvp(args[0], args);

                // will never run unless error (i.e.- bad filename)
                printf("%s", args[0]);
//                fflush(stdin);
//                fflush(stdout);
                fflush(NULL);
                perror("exec");  
 
                exit(1); // end child process
            }
            else if (cpid == -1) // parent process
            {   
                // if unable to fork print error
                printf("%s", args[0]);
//                fflush(stdin);
//                fflush(stdout); 
                fflush(NULL);                 
                perror(" ");
            } 
            else
            {
                // parent process continues here

                // restore the pointer to allocated memory
//                *next = temp; 

                // if command is bg process
                if (isBackgroundProcess == true)
                {
                    // then print process id when begins
                    printf("background pid is %d\n", cpid);

                    // reset boolean value for next iteration
                    isBackgroundProcess = false;

                    // add process id to array of background processes
                    if (cur < MAX_PIDS)
                    {  
                        bgpid[cur++] = cpid;
                    }
                } 
                else
                {
                    // reset value of signal number
                    signalNum = 0;                     

                    // assign cpid to global variable
                    // for access in signal handlers  
                    fgpid = cpid;

                    // wait for fg child process
                    fgpid = waitpid(fgpid, &status, 0);

                    // reset global variable so signal handlers know
                    // there is no active fg process
                    fgpid = INT_MAX;

                    // if process was terminated by signal, print message
                    if (signalNum != 0)
                    {
                        printf("terminated by signal %d\n", signalNum);
                    }   
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

    // if signal is not from fg process, process it here
    if (ref_pid != fgpid && completed_cur < MAX_PIDS)
    {
        // add to completed bg process array so message can
        // be displayed in main loop
        completed_pid[completed_cur++] = ref_pid;
    } 

    return;
}



void sigintHandler(int sig, siginfo_t* info, void* vp)
{
    // if interrupt signal occurs while fg process is running, kill it
    if (fgpid != INT_MAX)
    {
        // kill the foreground process
        kill(fgpid, SIGKILL);
 
        // set global variable for status messages
        signalNum = 2;  
    }  

    // ignore interrupt signal for all other processes
    // and simply return
    return;
}
