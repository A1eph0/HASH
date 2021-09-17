#ifndef FUNCS_H
#define FUNCS_H

void echo(char *args[]);            // implementation of the echo command
void cd(char *args[]);              // implementation of the cd command
void pwd();                         // implementation of the pwd command
void ls(char *args[]);              // implementation of the ls command
int plain_ls(char *path, int all_flag, int print_path_flag);    // plain ls implementation
int long_ls(char *path, int all_flag, int print_path_flag);     // long ls implementation
int calculate_total(char * path, int all_flag);
void print_permissions(mode_t st_mode);
void exec_back(char *args[]);
void exec_fore(char *args[]);
void pinfo(char *args[]);
void history(char *args[]);

#endif