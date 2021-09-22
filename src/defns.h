#ifndef DEFINITIONS_H
#define DEFINITIONS_H
#ifndef _GNU_SOURCE
#define _GNU_SOURCE

// importing all the standard libraries to be used
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <limits.h>
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#define WELCOME "\033[32;1m\n\
   ██╗  ██╗ █████╗ ███████╗██╗  ██╗\n\
   ██║  ██║██╔══██╗██╔════╝██║  ██║\n\
   ███████║███████║███████╗███████║\n\
   ██╔══██║██╔══██║╚════██║██╔══██║\n\
   ██║  ██║██║  ██║███████║██║  ██║\n\
   ╚═╝  ╚═╝╚═╝  ╚═╝╚══════╝╚═╝  ╚═╝\n\
   \n\
   Weclome to Hrishi's Arbitrary SHell\n\
   © Hrishi Narayanan 2021\n\
   \033[0;0m\n\
"

// some macros used throughout the code
#define MAX_ARG (int) (100)
#define MAX_COMMAND  (int) (2 << 15)
#define MAX_HIST (int) (20)
#define MAX_LOC (int) (10000)
#define MAX_PID  (int) (2 << 20)
#define FILE_PERM 0644

#endif
#endif