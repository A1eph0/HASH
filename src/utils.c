#include "defns.h"
#include "funcs.h"
#include "utils.h"

extern char *START_LOC;
extern char HIST_LOC[];
extern char HIST[MAX_HIST+1][MAX_COMMAND]; 
extern int HIST_SIZE;

// clears the screen
void clear()
{
    printf("\033c");
}

// prints the prompt string
void prompt()
{
    struct utsname info;
    uname(&info);
    
    char *path = get_current_dir_name();
    char *username = getlogin();

    relative_path(path);   
    printf("\033[31;1m<\033[33;1m%s\033[35;1m@%s\033[37;1m:\033[36;1m%s\033[31;1m>\033[0;0m ", username, info.nodename, path);
    free(path);
}

// given a path, generates path relative to the starting location
void relative_path(char *path)
{
    int start_len = strlen(START_LOC), path_len = strlen(path);

    if (strncmp(START_LOC, path, start_len) == 0)
    {   
        char *temp = malloc(path_len+1);
        temp[0] = '~'; 

        for (int i = start_len, j = 1; i < path_len; i++, j++)
            temp[j] = path[i];

        temp[path_len-start_len+1] = '\0';

        strcpy(path, temp);
        free(temp);
    }
}

// takes input and sends it for further processing
int await_input()
{
    char *raw_input_string = malloc(MAX_COMMAND);

    if(take_input(raw_input_string) == -1)
    {
        free(raw_input_string);
        return -1;
    }

    else if (strcmp(raw_input_string, "\n") != 0)
        call_command(raw_input_string);
    else
        printf("\n");
    
    free(raw_input_string);
    return 0;
        
}

// logic for taking input
int take_input(char *output_string)
{
    char *input_string = malloc(MAX_COMMAND);
    size_t size = MAX_COMMAND;
    int flag = getline(&input_string, &size, stdin);
    
    strcpy(output_string, input_string);

    free(input_string);
    return flag;
}

// logic of processing and calling commands
void call_command(char *raw_string)
{
    add_history(raw_string);
    char *processed_string = malloc(MAX_COMMAND);
    int command_count = process_raw_string(raw_string, processed_string);
    
    char *command[MAX_ARG] = {NULL};
    int i = 0;
    
    // printf("%s\t%d\n", processed_string, command_count);

    // tokenising the commands for individual execution
    char *token;
    token = strtok(processed_string, ";");
    
    while(token != NULL)
    {
        command[i++] = token;
        token = strtok(NULL, ";");
    }
    
    for (i = 0; i < command_count; i++)
        exec_command(command[i]);
    
    free(processed_string);
}

// processing the raw string to remove tabs, extra space, trailing space, newline and empty commands
int process_raw_string(char *raw_string, char* processed_string)
{
    char *temp = malloc(MAX_COMMAND);
    int count = 0;
    int j = -1;

    for (int i = 0; i< strlen(raw_string); i++)
    {
        j++;

        if (raw_string[i]==' ' && (j == 0 || raw_string[i+1] == ' ' || raw_string[i+1] == '\t' || raw_string[i+1] == '\n' || raw_string[i+1] == ';' || temp[j-1] == ';'))
            j--;
        else if (raw_string[i] == '\t')
        {
            if (j == 0 || raw_string[i+1] == ' ' || raw_string[i+1] == '\t' || raw_string[i+1] == '\n' || raw_string[i+1] == ';' || temp[j-1] == ';')
                j--;
            else
                temp[j] = ' ';
        }
        else if (raw_string[i] == '\n')
        {
            if (j == 0 || temp[j-1] == ';')
                j--;
            else
            {
                // temp[j] = ' ';
                // j++;
                temp[j] = ';';
                count ++;           // counting the total number of commands
            }
        }
        else if (raw_string[i] == ';')
        {
            if (j == 0 || temp[j-1] == ';')
                j--;
            else
            {
                // temp[j] = ' ';
                // j++;
                temp[j] = ';';
                count++;            // counting the total number of commands
            }
        }
        else if (raw_string[i] == '&')
        {
            temp[j] = raw_string[i];
            j++;
            temp[j] = ';';
            count++;
        }
        else
            temp[j] = raw_string[i];
    }

    strcpy(processed_string, temp);
    free(temp);
    return count;
}

// executes each tokenised command
void exec_command(char *command_string)
{
    char *token;
    token = strtok(command_string, " ");
    
    char *command = token;
    char *args[MAX_ARG] = {NULL};

    token = strtok(NULL, " ");

    int i = 0;
    while(token != NULL && i < MAX_ARG)
    {
        args[i] = token;
        token = strtok(NULL, " ");
        i++;
    }

    dispatch(command, args);
}

// dispatches the function calls for the commands
void dispatch(char *command, char *args[])
{
    if (strcmp(command, "clear") == 0)
        clear();
    else if (strcmp(command, "exit") == 0 || strcmp(command, "quit") == 0)
        exit(0);
    else if (strcmp(command, "pwd") == 0)
        pwd();
    else if (strcmp(command, "echo") == 0)
        echo(args);
    else if (strcmp(command, "cd") == 0)
        cd(args);
    else if (strcmp(command, "ls") == 0)
        ls(args);
    else if (strcmp(command, "pinfo") == 0)
        pinfo(args);
    else if (strcmp(command, "history") == 0)
        history(args);
    else if (strcmp(command, "repeat") == 0)
        for(int i=0; i<atoi(args[0]); i++)
            dispatch(args[1], args+2);
    else 
        other_command(command, args);

}

void other_command(char *command, char *args[])
{
    int back_flag = 0;
    char* all_together[MAX_ARG+1]={NULL};

    if (command[0] == '~')
    {
        char temp[MAX_LOC];
        strcpy(temp, START_LOC);
        strcat(temp, command+1);
        command = temp;
    }

    all_together[0] = command;

    for(int i=0; i<MAX_ARG; i++)
    {
        if(args[i]==NULL)
            break;
        else if (strcmp(args[i], "&") == 0)
        {
            back_flag = 1;
            break;
        }

        all_together[i+1] = args[i];
    }

    if (back_flag)
        exec_back(all_together);
    else
        exec_fore(all_together);
}


void get_history()
{
    FILE *hist_file= fopen(HIST_LOC, "a+");
    if (hist_file == NULL)
    {
        perror("error getting history");
        exit(1);
    }
    
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    int i=0;
    while (i < 20) 
    {
        if (getline(&line, &len, hist_file) == -1)
            break;
        strcpy(HIST[i], line);
        i++;
    }
    HIST_SIZE = i;

    fclose(hist_file);
}

void add_history(char *command)
{
    if (HIST_SIZE!=0 && strcmp(command, HIST[HIST_SIZE-1]) == 0)
        return;
    if(HIST_SIZE < MAX_HIST)
    {
        strcpy(HIST[HIST_SIZE], command);
        HIST_SIZE++;
    }
    else
    {
        for(int i=1; i<HIST_SIZE; i++)
            strcpy(HIST[i-1], HIST[i]);
        
       strcpy(HIST[HIST_SIZE-1], command);
    }

    FILE *hist_file= fopen(HIST_LOC, "w+");
    if (hist_file == NULL)
    {
        perror("error writing history");
        exit(1);
    }

    for(int i=0; i<HIST_SIZE; i++)
        if (fprintf(hist_file, "%s", HIST[i]) < 0)
            perror("error while writing history");
    
    fclose(hist_file);
}