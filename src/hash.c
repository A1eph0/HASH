#include "defns.h"
#include "funcs.h"
#include "utils.h"

char *START_LOC = NULL;                 // pointer for storing start location
char PREV_LOC[MAX_LOC];                 // pointer for storing prev location
char HIST_LOC[MAX_LOC];                 // pointer for storing history location
char HIST[MAX_HIST+1][MAX_COMMAND];     // array for history
int HIST_SIZE = 0;                      // initiating history size
char *PROC_NAME[MAX_PID]={NULL};        // map from pid to process name
int JOB_NUM[MAX_PID];                   // map from pid to job number
pid_t JOB_PID[MAX_JOBS];                // map from job number to pid
int FORE_BACK[MAX_PID];                 // map from pid to job fore/back status
int JOB_VAL = 1;                        // initiating job counter
int PROMPT_FL = 1;                      // initiating flag for prompt printing during signals

signed main()
{
    // setting up signal handler for SIGINT and SIGTSTP
    signal(SIGINT, int_handle);     // setup handler for Ctrl+C
    signal(SIGTSTP, tstp_handle);   // setup handler for Ctrl+Z

    // set job of pid = 0 as invalid
    JOB_PID[0] = -1;

    // clearing screen and printing welcome message
    clear();
    printf("%s", WELCOME);

    // storing start location
    START_LOC = get_current_dir_name();
    strcpy(PREV_LOC, START_LOC);
    
    // storing the location of history
    strcpy(HIST_LOC, START_LOC);
    strcat(HIST_LOC, "/.hash_history");

    int backup_stdout = dup(STDOUT_FILENO);
    int backup_stdin = dup(STDIN_FILENO);

    // main loop for shell
    while(1)
    {
        PROMPT_FL = 1;

        dup2(backup_stdout, STDOUT_FILENO);
        dup2(backup_stdin, STDIN_FILENO);

        get_history();
        prompt();                   // print prompt
        int flag = await_input();   // take input
        
        if (flag == -1)             // break if Ctrl+D (EOF) is received
            break;
    }

    // killing all child processes, for clean exit
    killall();

    free(START_LOC);
}