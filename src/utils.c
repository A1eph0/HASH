#include "defns.h"
#include "funcs.h"
#include "utils.h"

extern char *START_LOC;
extern char HIST_LOC[];
extern char HIST[MAX_HIST+1][MAX_COMMAND]; 
extern int HIST_SIZE;
extern char *PROC_NAME[];
extern int JOB_NUM[];
extern int JOB_VAL;
extern pid_t JOB_PID[];
extern int FORE_BACK[];
extern int PROMPT_FL;

// handler for SIGINT (Ctrl+C)
void int_handle()
{
    write(STDOUT_FILENO, "\n", 1);

    // reset values
    for(int i =1; i < JOB_VAL; i++)
    {
        if(JOB_PID[i] != -1 && FORE_BACK[JOB_PID[i]] == 0)
        {
            FORE_BACK[JOB_PID[i]] = 1;
            PROC_NAME[JOB_PID[i]] = NULL;
            return;
        }
    }

    // if called without process, print prompt
    if(PROMPT_FL)
        prompt();
}

// handler for SIGSTP (Ctrl+Z)
void tstp_handle()
{
    write(STDOUT_FILENO, "\n", 1);

    // reset values 
    for(int i =1; i < JOB_VAL; i++)
    {
        if(JOB_PID[i] != -1 && FORE_BACK[JOB_PID[i]] == 0)
        {
            FORE_BACK[JOB_PID[i]] = 1;
            return;
        }
    }

    // if called without process, print prompt
    if(PROMPT_FL)
        prompt();  
}

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
    fflush(stdin);
    fflush(stdout);
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
    int command_count = process_raw_string(raw_string, processed_string, ';');
    
    // fprintf(stderr,"%s\n", processed_string);
    char *command[MAX_ARG] = {NULL};
    int i = 0;

    // tokenising the commands for individual execution
    char *token;
    token = strtok(processed_string, ";");
    
    while(token != NULL)
    {
        command[i++] = token;
        token = strtok(NULL, ";");
    }
    
    for (i = 0; i < command_count; i++)
        handle_pipes(command[i]);
        // exec_command(command[i]);
    
    free(processed_string);
}

// processing the raw string to remove tabs, extra space, trailing space, newline, ampersand and empty commands
int process_raw_string(char *raw_string, char* processed_string, char delim)
{
    char *temp = malloc(MAX_COMMAND);
    int count = 0;
    int j = -1;

    for (int i = 0; i< strlen(raw_string); i++)
    {
        j++;

        if (raw_string[i]==' ' && (j == 0 || raw_string[i+1] == ' ' || raw_string[i+1] == '\t' || raw_string[i+1] == '\n' || raw_string[i+1] == delim || temp[j-1] == delim))
            j--;
        else if (raw_string[i] == '\t')
        {
            if (j == 0 || raw_string[i+1] == ' ' || raw_string[i+1] == '\t' || raw_string[i+1] == '\n' || raw_string[i+1] == delim || temp[j-1] == delim)
                j--;
            else
                temp[j] = ' ';
        }
        else if (raw_string[i] == '\n')
        {
            if (j == 0 || temp[j-1] == delim)
                j--;
            else
            {
                temp[j] = delim;
                count ++;           // counting the total number of commands
            }
        }
        else if (raw_string[i] == delim)
        {
            if (j == 0 || temp[j-1] == delim)
                j--;
            else
            {
                temp[j] = delim;
                count++;            // counting the total number of commands
            }
        }
        else if (raw_string[i] == '&')
        {
            temp[j] = raw_string[i];
            j++;
            temp[j] = delim;
            count++;
        }
        else if (raw_string[i] == '>')
        {
            if(temp[j-1] != '>' && temp[j-1] != ' ')
            {
                temp[j] = ' ';
                j++;
            }
            temp[j] = raw_string[i];
            if(raw_string[i+1]!='>' && raw_string[i+1]!=' ' && raw_string[i+1]!=delim && raw_string[i+1]!='\n' && raw_string[i+1]!='\t')
            {
                j++;
                temp[j] = ' ';
            }
        }
        else if (raw_string[i] == '<')
        {
            if(temp[j-1] != ' ')
            {
                temp[j] = ' ';
                j++;
            }
            temp[j] = raw_string[i];
            if(raw_string[i+1]!=' ' && raw_string[i+1]!=delim && raw_string[i+1]!='\n' && raw_string[i+1]!='\t')
            {
                j++;
                temp[j] = ' ';
            }
        }
        else
            temp[j] = raw_string[i];
    }

    j++;
    if(j !=0 && temp[j-1]!='\0')
        temp[j] = '\0';
    strcpy(processed_string, temp);
    free(temp);
    return count;
}

// executes each tokenised command
void exec_command(char *command_string)
{
    int i_flag = 0, o_flag = 0;
    char i_file[MAX_LOC] = "";       // storing input file location
    char o_file[MAX_LOC] = "";       // storing output file location

    char *token;
    token = strtok(command_string, " ");
    
    char *command = token;
    char *args[MAX_ARG] = {NULL};

    token = strtok(NULL, " ");

    int i = 0;
    // handling redirection symbols in input (<, >>, >)
    while(token != NULL && i < MAX_ARG)
    {
        if(i_flag == 0 && strcmp(token, "<") == 0)
        {
            i_flag = 1;
            token = strtok(NULL, " ");

            if(token[0] == '~')
            {
                strcpy(i_file, START_LOC);
                token++;
            }
            strcat(i_file, token);
            token = strtok(NULL, " ");
        }
        else if(strcmp(token, ">") == 0)
        {
            o_flag = 1;
            token = strtok(NULL, " ");
            if(token[0] == '~')
            {
                strcpy(o_file, START_LOC);
                token++;
            }
            strcat(o_file, token);
            break;
        }
        else if(strcmp(token, ">>") == 0)
        {
            o_flag = 2;
            token = strtok(NULL, " ");
            if(token[0] == '~')
            {
                strcpy(o_file, START_LOC);
                token++;
            }
            strcat(o_file, token);
            break;
        }
        else
        {   
            args[i] = token;
            token = strtok(NULL, " ");
            i++;
        }
    }
    
    // handle dispatch according to whether redirection is needed
    if(i_flag == 0 && o_flag == 0)
        dispatch(command, args);
    else
        handle_redirection(command, args, i_flag, o_flag, i_file, o_file);
}

// dispatches the function calls for the commands
void dispatch(char *command, char *args[])
{
    PROMPT_FL = 0;
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
    else if (strcmp(command, "jobs") == 0)
        jobs(args);
    else if (strcmp(command, "sig") == 0)
        sig(args);
    else if (strcmp(command, "bg") == 0)
        bg(args);
    else if (strcmp(command, "fg") == 0)
        fg(args);
    else if (strcmp(command, "replay") == 0)
        replay(args);
    else if (strcmp(command, "repeat") == 0)
        for(int i=0; i<atoi(args[0]); i++)
            dispatch(args[1], args+2);
    else 
        other_command(command, args);

}

// handles commands that are not inbuilt
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

// gets content from history file (~/.hash_history)
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

// writes content to history file (~/.hash_history)
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

// handles redirection
void handle_redirection(char *command, char *args[], int i_flag, int o_flag, char* i_file, char* o_file)
{
    int backup_stdout, backup_stdin;
    int i_pointer, o_pointer;

    if (i_flag)
    {   
        backup_stdin = dup(STDIN_FILENO);
        i_pointer = open(i_file, O_RDONLY);
        if (i_pointer < 0)
        {
            perror("");
            return;
        }
        if (dup2(i_pointer, STDIN_FILENO) == -1)
        {
            perror("");
            return;
        }
    }

    if (o_flag)
    {
        backup_stdout = dup(STDOUT_FILENO);
        int app_trunc_flag = O_TRUNC;
        
        if(o_flag == 2)
            app_trunc_flag = O_APPEND;

        o_pointer = open(o_file, O_CREAT | O_WRONLY | app_trunc_flag, FILE_PERM);
        if (o_pointer < 0)
        {
            perror("");
            return;
        }
        if (dup2(o_pointer, STDOUT_FILENO) == -1)
        {
            perror("");
            return;
        }
    }

    dispatch(command, args);
    
    if(i_flag != 0)
    {
        close(i_pointer);
        dup2(backup_stdin, STDIN_FILENO);
    }
    if(o_flag != 0)
    {
        close(o_pointer);
        dup2(backup_stdout, STDOUT_FILENO);
    }
}

// handles pipes (if any)
void handle_pipes(char* raw_string)
{
    char *processed_string = malloc(MAX_COMMAND);
    char raw_temp[MAX_COMMAND] = "";
    strcpy(raw_temp, raw_string);
    strcat(raw_temp, "\n");

    int command_count = process_raw_string(raw_temp, processed_string, '|');

    if(command_count == 1)
    {
        exec_command(raw_string);
        free(processed_string);
        return;
    }

    char *command[MAX_ARG] = {NULL};
    int i = 0;

    // tokenising the commands for individual execution
    char *token;
    token = strtok(processed_string, "|");
    
    while(token != NULL)
    {
        command[i++] = token;
        token = strtok(NULL, "|");
    }

    int backup_stdout = dup(STDOUT_FILENO);
    int backup_stdin = dup(STDIN_FILENO);

    // intiating array of file descriptors for piping
    int file_desc[(command_count-1)][2];

    for(int i = 0; i <(command_count-1); i++)
    {
        if(pipe(file_desc[i]))
        {
            perror("");
            free(processed_string);
            return;
        }
    }

    for(int i = 0; i < command_count; i++)
    {
        // if it is the first command
        if (i == 0) 
        {
            dup2(file_desc[i][1], STDOUT_FILENO);
            exec_command(command[i]);
            close(file_desc[i][1]);
        }
        // if it is the last command
        else if (i == command_count-1) 
        {
            dup2(backup_stdout, STDOUT_FILENO);
            dup2(file_desc[i-1][0], STDIN_FILENO);
            exec_command(command[i]);
            close(file_desc[i-1][0]);
        }
        // if its neither first or last command
        else 
        {
            dup2(file_desc[i][1], STDOUT_FILENO);
            dup2(file_desc[i-1][0], STDIN_FILENO);
            exec_command(command[i]);
            close(file_desc[i][1]);
            close(file_desc[i-1][0]);
        }
    }
        
    dup2(backup_stdout, STDOUT_FILENO);
    dup2(backup_stdin, STDIN_FILENO);
    free(processed_string);
}

// kills all child process for cleanup during exit
void killall()
{
    for(pid_t i=0; i<MAX_PID; i++)
        if(PROC_NAME[i]!=NULL)
            kill(i, SIGKILL);
}