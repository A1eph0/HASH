#include "defns.h"
#include "funcs.h"
#include "utils.h"

char *START_LOC = NULL; // pointer for storing start location
char PREV_LOC[MAX_LOC];  // pointer for storing prev location
char HIST_LOC[MAX_LOC];  // pointer for storing history location
char HIST[MAX_HIST+1][MAX_COMMAND]; 
int HIST_SIZE = 0;
char *PROC_NAME[MAX_PID]={NULL};

signed main()
{
    // clearing screen and printing welcome message
    clear();
    printf("%s", WELCOME);

    // storing start location
    START_LOC = get_current_dir_name();
    strcpy(PREV_LOC, START_LOC);
    
    strcpy(HIST_LOC, START_LOC);
    strcat(HIST_LOC, "/.hash_history");


    // main loop for shell
    while(1)
    {
        get_history();
        prompt(START_LOC);          // print prompt
        int flag = await_input();   // take input
        
        if (flag == -1)             // break if Ctrl+D (EOF) is received
            break;
    }

    free(START_LOC);
}