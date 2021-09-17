#include "defns.h"
#include "funcs.h"
#include "utils.h"

extern char *START_LOC;
extern char PREV_LOC[];
extern char HIST[MAX_HIST+1][MAX_COMMAND]; 
extern int HIST_SIZE;
extern int errno;

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
    int all_flag = 0, long_flag = 0;
    int j = 0;
    
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
            // printf("here");
            // if (args[i][0] == '~')
            // {
            //     strcpy(temp, START_LOC);
            //     strcat(temp, args[i]+1);
            // }
            // else 
            //     strcpy(temp, args[i]);
            
            files[j] = args[i];
            j++;
        }
        // printf("%s", args[i]);
    }
    
    if (files[0] == NULL)
    {       
        files[0] = ".";
        j++;
    }

    int print_path_flag = 0;

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
    if (print_path_flag)
        printf("%s:\n", path);
    struct dirent *directory_path;
    DIR *directory = opendir(path);

    if (directory == 0)
    {
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
    if (print_path_flag)
        printf("%s:\n", path);

    struct dirent *directory_path;
    DIR *directory = opendir(path);

    if (directory == 0)
    {
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
            
            print_permissions(file_stat.st_mode);
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

    calculate_total(path, all_flag);


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
            
            print_permissions(file_stat.st_mode);
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

void exec_back(char *args[])
{
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
            printf("Command not found\n");
            return;
        }
    }

    printf("%d ", pid);
    printf("\n\n");
}

void exec_fore(char *args[])
{
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
            printf("Command not found\n");
            return;
        }
    }
    else
    {
        int status;
        waitpid(pid, &status, WUNTRACED);
    }
}

void pinfo(char *args[])
{
    pid_t pid;
    int flag = 0;

    if (args[0] != NULL)
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
    sprintf(status_path, "/proc/%d/stat", pid);
    

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

    sprintf(status_path, "/proc/%d/exe", pid);
    char exec_path[MAX_LOC];
    
    if(readlink(status_path, exec_path, MAX_LOC) == -1)
        strcpy(exec_path, "Does not exist");
    else 
        relative_path(exec_path);
    
    printf("Executable Path -- %s\n", exec_path);
}

void history(char *args[])
{
    if (args[0] == HIST[HIST_SIZE-1])
        return;
    int temp = HIST_SIZE;
    if(args[0] != NULL)
        temp = atoi(args[0]);

    for(int i=0; i<temp; i++)
        printf("%s", HIST[i]);
}