#include "defns.h"
#include "funcs.h"
#include "utils.h"

extern char *START_LOC;
extern char PREV_LOC[];
extern char HIST[MAX_HIST+1][MAX_COMMAND]; 
extern int HIST_SIZE;
extern char *PROC_NAME[];
extern int JOB_NUM[];
extern int JOB_VAL;
extern pid_t JOB_PID[];
extern int FORE_BACK[];
extern int errno;

// comparator for qsort
int cmp(const void* a, const void* b)
{
    int pid_a = *(const int *)a;
    int pid_b = *(const int *)b;

    return (strcmp(PROC_NAME[pid_a], PROC_NAME[pid_b]));
}

// implementation of the echo command
void echo(char *args[])
{
    for(int i = 0; i < MAX_ARG; i++)
    {
        if (args[i] == NULL)
            break;
        printf("%s ", args[i]);
    }

    printf("\n");
}

// implementation of the cd command
void cd(char *args[])
{
    if (args[1] != NULL)
    {
        errno = 5;
        perror("too many arguments");
        return;
    }    
    char *temp = args[0];
    char prev_cpy[MAX_LOC];
    strcpy(prev_cpy, PREV_LOC);

    char *cwd = get_current_dir_name();
    
    size_t flag;

    if (temp == NULL || strcmp(temp, "~") == 0)
        flag = chdir(START_LOC);
    else if(!strcmp(temp, "-"))
    {
        relative_path(prev_cpy);
        printf("%s\n", prev_cpy);
        flag = chdir(PREV_LOC);   
    }
    else if (temp[0] == '~')
    {
        flag = chdir(START_LOC);
        temp += 2;
        flag = chdir(temp);
    }
    else
        flag = chdir(temp);

    // checking for errors
    if(flag)
        perror("failed to call directory");
    else
        strcpy(PREV_LOC, cwd);      // storing previous location for next call 

    free(cwd);
}

// implementation of the pwd command
void pwd()
{
    char *path = get_current_dir_name();
    printf("%s\n", path);

    free(path);
}

// implementation of the ls command
void ls(char *args[])
{
    char *files[MAX_ARG]={NULL};
    char temp[MAX_LOC];
    int all_flag = 0, long_flag = 0;    // intiating the flags
    int j = 0;
    
    // searching for flags
    for (int i=0; i<MAX_ARG; i++)
    {
        if (args[i] == NULL)
            break;
        
        if (strcmp(args[i],"-a") == 0)
            all_flag = 1;

        else if (strcmp(args[i], "-l") == 0)
            long_flag = 1;

        else if (strcmp(args[i], "-la") == 0 || strcmp(args[i], "-al") == 0)
        {
            all_flag = 1;
            long_flag = 1;
        }

        else
        {   
            files[j] = args[i];
            j++;
        }
    }
    
    // if no file or directory is given as arguement
    if (files[0] == NULL)
    {       
        files[0] = ".";
        j++;
    }

    int print_path_flag = 0;

    // flag for printing file path if there are multiple directories
    if (j>1)
        print_path_flag = 1;

    if(long_flag)
        for(int i = 0; i < j; i++)
        {
            if (files[i][0] == '~')
            {
                strcpy(temp, START_LOC);
                strcat(temp, files[i]+1);
            }
            else 
                strcpy(temp, files[i]);

            long_ls(temp, all_flag, print_path_flag);
        }

    else
        for(int i = 0; i < j; i++)
        {
            if (files[i][0] == '~')
            {
                strcpy(temp, START_LOC);
                strcat(temp, files[i]+1);
            }
            else 
                strcpy(temp, files[i]);

            plain_ls(temp, all_flag, print_path_flag);
        }
}

// plain ls implementation
int plain_ls(char *path, int all_flag, int print_path_flag)
{
    if (print_path_flag)        // prints filepath directory path if multiple are given as arguement
        printf("%s:\n", path);
    struct dirent *directory_path;
    DIR *directory = opendir(path);

    if (directory == 0)
    {   
        // if it is not a directory, check whether it is a file
        FILE *file;
        if ((file = fopen(path, "r")) == NULL)     
        {
            perror("");
            return 1;
        } 
        else 
        {
            fclose(file);
            printf("%s\n\n", path);
            return 0;
        }
    } 

    while ((directory_path = readdir(directory)) != NULL)
        if (all_flag || directory_path->d_name[0] != '.')
            printf("%s\t", directory_path->d_name);

    printf("\n\n");

    closedir(directory);
}

// long ls implementation
int long_ls(char *path, int all_flag, int print_path_flag)
{
    if (print_path_flag)            // prints filepath directory path if multiple are given as arguement
        printf("%s:\n", path);

    struct dirent *directory_path;
    DIR *directory = opendir(path);

    if (directory == 0)
    {
        // if it is not a directory, check whether it is a file
        FILE *file;
        if ((file = fopen(path, "r")) == NULL)     
        {
            perror("");
            return 1;
        } 
        else 
        {
            fclose(file);

            struct stat file_stat;

            if (lstat(path, &file_stat))
                perror("");
            
            print_permissions(file_stat.st_mode);   // prints the file permissions
            printf("%lu \t", file_stat.st_nlink);
            printf("%s \t", getpwuid(file_stat.st_uid)->pw_name);
            printf("%s \t", getgrgid(file_stat.st_gid)->gr_name);
            printf("%lu \t", file_stat.st_size);

            char temp[MAX_LOC] = "";
            strcpy(temp, asctime(localtime(&(file_stat.st_mtime))));
            char time_str[MAX_LOC] = "";

            time_t curr_time;
            time(&curr_time);
            int format_flag = 0;

            if(difftime(mktime(localtime(&curr_time)),mktime(localtime(&(file_stat.st_mtime)))) < 0 || difftime(mktime(localtime(&curr_time)),mktime(localtime(&(file_stat.st_mtime)))) > 15778476)
                format_flag = 1;
            

            char *token;
            token = strtok(temp, " ");
            int i = 0;

            while(token != NULL)
            {
                if (i==1 || i==2 || (i == 4 && format_flag))
                {
                    strcat(time_str, token);
                    strcat(time_str, " ");
                }
                else if (i==3 && format_flag == 0)
                {
                    strcat(time_str, token);
                    time_str[strlen(time_str)-3]='\0';
                    strcat(time_str, " ");
                }

                i++;
                token = strtok(NULL, " ");
            }

            if (format_flag) 
            {
                time_str[strlen(time_str)-2] = ' ';
                time_str[strlen(time_str)-1] = '\0';
            }
                
            printf("%s \t", time_str);

            printf("%s\n", path);

            return 0;
        }
    }

    calculate_total(path, all_flag);    // calculates total block


    while ((directory_path = readdir(directory)) != NULL)
    {
        if (all_flag || directory_path->d_name[0] != '.')
        {
            char file[MAX_LOC] = "";
            strcat(file, path);
            strcat(file, "/");
            strcat(file, directory_path->d_name);

            struct stat file_stat;

            if (lstat(file, &file_stat))
                perror("");
            
            print_permissions(file_stat.st_mode);       // prints file permissons
            printf("%lu \t", file_stat.st_nlink);
            printf("%s \t", getpwuid(file_stat.st_uid)->pw_name);
            printf("%s \t", getgrgid(file_stat.st_gid)->gr_name);
            printf("%lu \t", file_stat.st_size);

            char temp[MAX_LOC] = "";
            strcpy(temp, asctime(localtime(&(file_stat.st_mtime))));
            char time_str[MAX_LOC] = "";

            time_t curr_time;
            time(&curr_time);
            int format_flag = 0;

            if(difftime(mktime(localtime(&curr_time)),mktime(localtime(&(file_stat.st_mtime)))) < 0 || difftime(mktime(localtime(&curr_time)),mktime(localtime(&(file_stat.st_mtime)))) > 15778476)
                format_flag = 1;
            
            char *token;
            token = strtok(temp, " ");
            int i = 0;

            while(token != NULL)
            {
                if (i==1 || i==2 || (i == 4 && format_flag))
                {
                    strcat(time_str, token);
                    strcat(time_str, " ");
                }
                else if (i==3 && format_flag == 0)
                {
                    strcat(time_str, token);
                    time_str[strlen(time_str)-3]='\0';
                    strcat(time_str, " ");
                }

                i++;
                token = strtok(NULL, " ");
            }

            if (format_flag) 
            {
                time_str[strlen(time_str)-2] = ' ';
                time_str[strlen(time_str)-1] = '\0';
            }

            printf("%s \t", time_str);

            printf("%s", directory_path->d_name);

            // prints the actual path if symbolic link
            if(S_ISLNK(file_stat.st_mode))
            {
                char temp[MAX_LOC]="";
                realpath(file, temp);
                printf(" -> %s\n", temp);
            }
            else
                printf("\n");

        }
    }
    printf("\n\n");

    closedir(directory);
}

// calculates the total block value
int calculate_total(char * path, int all_flag)
{
    struct dirent *directory_path;
    DIR *directory = opendir(path);
    int total = 0;
    
    while ((directory_path = readdir(directory)) != NULL)
    {
        if (all_flag || directory_path->d_name[0] != '.')
        {
            char file[MAX_LOC] = "";
            strcat(file, path);
            strcat(file, "/");
            strcat(file, directory_path->d_name);

            struct stat file_stat;

            if (lstat(file, &file_stat))
                perror("");

            total += (file_stat.st_blocks * 512 + 1023) / 1024;
        }
    }

    printf("total %d\n", total);

    closedir(directory);
}

// print permissions for ls -l command
void print_permissions(mode_t st_mode)
{
    char temp[] = " ---------";

    if (S_ISDIR(st_mode))
        temp[0] = 'd';
    else if (S_ISLNK(st_mode))
        temp[0] = 'l';
    else if (S_ISBLK(st_mode))
        temp[0] = 'b';
    else if (S_ISCHR(st_mode))
        temp[0] = 'c';
    else if (S_ISSOCK(st_mode))
        temp[0] = 's';
    else if (S_ISFIFO(st_mode))
        temp[0] = 'f';
    else if (S_ISREG(st_mode))
        temp[0] = '-';
    else
        temp[0] = '?';

    if (st_mode & S_IRUSR)
        temp[1] = 'r';
    if (st_mode & S_IWUSR)
        temp[2] = 'w';
    if (st_mode & S_IXUSR)
        temp[3] = 'x';
    if (st_mode & S_IRGRP)
        temp[4] = 'r';
    if (st_mode & S_IWGRP)
        temp[5] = 'w';
    if (st_mode & S_IXGRP)
        temp[6] = 'x';
    if (st_mode & S_IROTH)
        temp[7] = 'r';
    if (st_mode & S_IWOTH)
        temp[8] = 'w';
    if (st_mode & S_IXOTH)
        temp[9] = 'x';

    printf("%s \t", temp);
}

// prints exit message for background process
void exit_print()
{
    int status;
    pid_t pid = waitpid(-1, &status, WNOHANG);

    if (FORE_BACK[pid] == 1)
    {
        if (WIFEXITED(status))
            fprintf(stderr, "\n%s with pid %d ended normally\n", PROC_NAME[pid], pid);
        else
            fprintf(stderr, "\n%s with pid %d ended abnormally\n", PROC_NAME[pid], pid);
    }

    if(PROC_NAME[pid] != NULL)
    {
        free(PROC_NAME[pid]);
        PROC_NAME[pid] = NULL;
        prompt();
    }
    JOB_PID[JOB_NUM[pid]] = -1;
    PROC_NAME[pid] = NULL;
    FORE_BACK[pid]==-1;
}

// executes background commands
void exec_back(char *args[])
{
    signal(SIGCHLD, exit_print);
    pid_t pid = fork();
    if (pid < 0)
    {
        perror("");
        return;
    }

    else if (pid == 0)
    {
        setpgid(0,0);
        if (execvp(args[0], args))
        {
            fprintf(stderr, "Command not found\n");
            free(PROC_NAME[pid]);
            return;
        }
    }
    else
    {
        char *process_name = malloc(MAX_ARG);
        strcpy(process_name, args[0]);
        char temp[MAX_ARG]="";
        for(int i=1; args[i]!=NULL; i++)
        {
            strcat(temp, " ");
            strcat(temp, args[i]);
        }
        strcpy(process_name+strlen(process_name)+1, temp);
        PROC_NAME[pid] = process_name;
        JOB_NUM[pid] = JOB_VAL;
        FORE_BACK[pid] = 1;
        JOB_PID[JOB_VAL] = pid;
        JOB_VAL++;
    }

    printf("%d ", pid);
    printf("\n\n");
}

// executes foreground commands
void exec_fore(char *args[])
{
    signal(SIGCHLD, exit_print);
    pid_t pid = fork();
    if (pid < 0)
    {
        perror("");
        return;
    }

    else if (pid == 0)
    {
        if (execvp(args[0], args))
        {
            fprintf(stderr, "Command not found\n");
            return;
        }
    }
    else
    {
        FORE_BACK[pid] = 0;
        int status;
        if(waitpid(pid, &status, WUNTRACED) > 0 && WIFSTOPPED(status) != 0)
        {
            char *process_name = malloc(MAX_ARG);
            strcpy(process_name, args[0]);
            char temp[MAX_ARG]="";
            for(int i=1; args[i]!=NULL; i++)
            {
                strcat(temp, " ");
                strcat(temp, args[i]);
            }
            strcpy(process_name+strlen(process_name)+1, temp);
            PROC_NAME[pid] = process_name;
            JOB_NUM[pid] = JOB_VAL;
            FORE_BACK[pid] = 1;
            JOB_PID[JOB_VAL] = pid;
            JOB_VAL++;
        }
    }
}

// implementation of pinfo command
void pinfo(char *args[])
{
    pid_t pid;
    int flag = 0;

    if (args[0] != NULL)    // if no pid is provided
    {
        pid = atoi(args[0]);
        if (pid == getpid())
            flag = 1;
    }
    else
    {
        pid = getpid();
        flag = 1;
    }
    printf("pid -- %d\n", pid);

    char status_path[MAX_LOC];
    sprintf(status_path, "/proc/%d/stat", pid);     // path to stat file (/proc/pid/stat)
        
    FILE *status = fopen(status_path, "r");
    if (status == NULL)
    {
        perror("");
        return;
    }

    char file_stat[MAX_COMMAND];
    fread(file_stat, MAX_COMMAND, 1, status);
    fclose(status);

    char *token = strtok(file_stat," ");
    
    int j = 0;
    while(token != NULL)
    {
        if(j == 2 && flag)
            printf("Process Status -- %s+\n", token);
        else if (j == 2)
            printf("Process Status -- %s\n", token);
        else if (j == 22)
        {
            printf("memory -- %s {Virtual Memory}\n", token);
            break;
        }

        token = strtok(NULL, " ");
        j++;
    }

    sprintf(status_path, "/proc/%d/exe", pid);       // path to exe symbolic link file (/proc/pid/exe)
    char exec_path[MAX_LOC];
    
    if(readlink(status_path, exec_path, MAX_LOC) == -1)
        strcpy(exec_path, "Does not exist");
    else 
        relative_path(exec_path);
    
    printf("Executable Path -- %s\n", exec_path);
}

// implementation of history command
void history(char *args[])
{
    int temp = HIST_SIZE;
    if(args[0] != NULL)
        temp = atoi(args[0]);

    for(int i=0; i<temp; i++)
        printf("%s", HIST[i]);
}

// implementation of jobs command
void jobs(char *args[])
{
    int flag = 0;

    // check for flags
    if(args[0]!=NULL)
    {
        if(strcmp(args[0], "-r")==0)
            flag = 1;
        else if(strcmp(args[0], "-s")==0)
            flag = 2;
    }

    pid_t all_proc[MAX_JOBS];      // array to store all jobs
    
    int proc_count = 0;
    for(pid_t i=0; i<MAX_PID; i++)
    {
        if(PROC_NAME[i]!=NULL)
        {
            all_proc[proc_count] = i;
            proc_count++;
        }
    }

    // sorting the jobs
    qsort(all_proc, proc_count, sizeof(pid_t), cmp);

    // printing the jobs
    for(int i=0; i<proc_count; i++)
    {
        pid_t pid = all_proc[i];

        char status_path[MAX_LOC];
        sprintf(status_path, "/proc/%d/stat", pid);     // path to stat file (/proc/pid/stat)
            
        FILE *status = fopen(status_path, "r");
        if (status == NULL)
        {
            perror("");
            return;
        }

        char file_stat[MAX_COMMAND];
        fread(file_stat, MAX_COMMAND, 1, status);
        fclose(status);

        char *token = strtok(file_stat," ");
        
        int j = 0;
        while(token != NULL)
        {
            if(j == 2)
            {
                if((strcmp(token, "S") == 0 || strcmp(token,"R") == 0) && flag !=2)
                    printf("[%d] Running ", JOB_NUM[pid]);
                else if(flag != 1)
                    printf("[%d] Stopped ", JOB_NUM[pid]);
                break;
            }
            token = strtok(NULL, " ");
            j++;
        }

        printf("%s%s [%d]\n", PROC_NAME[pid], PROC_NAME[pid]+strlen(PROC_NAME[pid])+1, pid);
    }
}

// implementation of sig command
void sig(char *args[])
{
    // check whether given job number is valid
    if(args[0] == NULL || args[1] == NULL || atoi(args[0]) >= JOB_VAL || atoi(args[1]) > 28)
    {
        fprintf(stderr, "Invalid input\n");
        return;
    }

    pid_t pid = JOB_PID[atoi(args[0])];

    // check whether given signal number is valid
    if(pid == -1 || atoi(args[1])>31)
    {
        fprintf(stderr, "Invalid input\n");
        return;
    }

    kill(pid, atoi(args[1]));
}

// implementation of bg command
void bg(char *args[])
{
    // check whether given job is valid
    if(args[0] == NULL || atoi(args[0])>=JOB_VAL)
    {
        fprintf(stderr, "Invalid input\n");
        return;
    }

    pid_t pid = JOB_PID[atoi(args[0])];

    // check whether given job has not terminated
    if(pid == -1)
    {
        fprintf(stderr, "Invalid input\n");
        return;
    }

    kill(pid, SIGCONT);
}

// implementation of fg command
void fg(char *args[])
{
    // check whether given job is valid
    if(args[0] == NULL || atoi(args[0])>=JOB_VAL)
    {
        fprintf(stderr, "Invalid input\n");
        return;
    }
    
    pid_t pid = JOB_PID[atoi(args[0])];

    // check whether given job has not terminated
    if(pid == -1)
    {
        fprintf(stderr, "Invalid input\n");
        return;
    }

    // save necessary values
    char *process_name = PROC_NAME[pid];
    PROC_NAME[pid] = NULL;
    FORE_BACK[pid] = 0;
    int temp_job = atoi(args[0]);
    
    // ignore output and input signals
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);

    // set process group id as the same as that of the shell
    tcsetpgrp(STDIN_FILENO, getpgid(pid));
    kill(pid, SIGCONT);

    int status;
    if(waitpid(pid, &status, WUNTRACED) > 0 && WIFSTOPPED(status) != 0)
    {
        PROC_NAME[pid] = process_name;
        JOB_NUM[pid] = temp_job;
        FORE_BACK[pid] = 1;
        JOB_PID[temp_job] = pid;
    }

    // set process group id to 0,0;
    tcsetpgrp(STDIN_FILENO, getpgid(0));

    signal(SIGTTOU, SIG_DFL);
    signal(SIGTTIN, SIG_DFL);
    // set the input output signals to default
}

// implementation of replay command
void replay(char *args[])
{
    char *command[MAX_COMMAND] = {NULL};
    int interval;
    int period;

    int command_flag = 0;
    int interval_flag = 0;
    int period_flag = 0;
    int j=0;

    // checking whether all of command, interval and period have been provided
    for(int i =0; args[i]!=NULL; i++)
    {
        if(strcmp(args[i], "-command") == 0 && command_flag == 0)
        {
            command_flag = 1;
            i++;
            while(args[i]!=NULL)
            {
                if(strcmp(args[i], "-interval") == 0 || strcmp(args[i], "-period") == 0 )
                {
                    i--;
                    break;
                }
                else
                {
                    command[j] = args[i];
                    j++;
                    i++;
                }
            }
        }
        else if(strcmp(args[i], "-interval") == 0 && interval_flag == 0)
        {
            interval_flag = 1;
            i++;
            if(args[i]!=NULL && strcmp(args[i], "-command") != 0 && strcmp(args[i], "-period") != 0 )
                interval = atoi(args[i]);
        }
        else if(strcmp(args[i], "-period") == 0 && period_flag == 0)
        {
            period_flag = 1;
            i++;
            if(args[i]!=NULL && strcmp(args[i], "-command") != 0 && strcmp(args[i], "-interval") != 0 )
                period = atoi(args[i]);
        }
    }

    // run command at time t = 0
    dispatch(command[0], command+1);

    // if interval is greater than period
    if(interval> period)
        return;
    
    clock_t start = clock();    // stores start time
    clock_t keep = start;       // stores time since last operation
    while(((double)(clock() - start)/CLOCKS_PER_SEC) - (double) (period - interval) < -0.0005)
    {
        // if period is reached
        if(((double)(clock() - keep)/CLOCKS_PER_SEC) - (double) interval < 0.0005 && ((double)(clock() - keep)/CLOCKS_PER_SEC) - (double) interval > -0000.5)
        {
            dispatch(command[0], command+1);
            keep = clock();
        }
    }   
}