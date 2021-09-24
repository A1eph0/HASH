#ifndef FUNCS_H
#define FUNCS_H

void echo(char *args[]);            // implementation of the echo command
void cd(char *args[]);              // implementation of the cd command
void pwd();                         // implementation of the pwd command
void ls(char *args[]);              // implementation of the ls command
int plain_ls(char *path, int all_flag, int print_path_flag);    // plain ls implementation
int long_ls(char *path, int all_flag, int print_path_flag);     // long ls implementation
int calculate_total(char * path, int all_flag);                 // calculates the total block value
void print_permissions(mode_t st_mode);                         // print permissions for ls -l command
void exit_print();                  // prints exit message for background process
void exec_back(char *args[]);       // executes background commands
void exec_fore(char *args[]);       // executes foreground commands
void pinfo(char *args[]);           // implementation of pinfo command
void history(char *args[]);         // implementation of history command
void jobs(char *args[]);
void sig(char *args[]);
void bg(char *args[]);

#endif