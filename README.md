# Hrishi's Arbitrary SHell (HASH): a functional shell implementation


> "One OS to rule them all,<br>
> One OS to find them. <br>
> One OS to call them all, <br>
> And in salvation bind them. <br>
> In the bright land of Linux, <br>
> Where the hackers play." <br>
> <br>
>~ J. Scott Thayer (with apologies to J.R.R. Tolkien)

<img src="https://cdn.discordapp.com/attachments/880398206817685514/888403502618009650/unknown.png">

# About

**H**rish's **A**rbitrary **SH**ell (**HASH**) is a functional implementation of a shell for Linux based systems. Coded completely in C language, the shell replicates the basic functionality of other popular shells like Bash and Zsh. 

For instructions running the shell, click [here](#running-the-shell).

## Table of contents
- [About](#About)
- [Structuring of Code](#Structuring-of-Code)
- [Salient Features](#Salient-Features)
- [Functionalities](#Functionalities)
- [Running the Shell](#Running-the-Shell)

# Structuring of Code
- The code is written in **C language**.
- The code files are present in the the `src` directory.
- The driver of the shell is coded in the `hash.c` file.
- The utilities required in the internal function of the code are coded in `utils.c`.
- The functions implemented within the shell are present in `funcs.c`.
- Standard C libraries are called within `defns.h`. 
- Global macros are also defined within `defns.h`.

# Salient Features
- The shell is coded from scratch using **C language**.
- The shell has a **built-in parser** which processes the input, taking into account extra spaces and tabs, semi-colons, ampersand etc. The same parser is used to parse pipes as well.
- Has a **colorised** prompt for better user experience.
- The prompt is of the form `<username@hostname:current_working_directory>`.
- `current_working_directory` is **relative to the execution location of the shell**, if it is below the execution location in the file tree. Otherwise, it shows the **absolute path** of the current working directory. 
- Built-in commands are implemented within the shell, which include:
  - `echo`
  - `ls`
  - `clear`
  - `cd`
  - `pwd`
  - `exit` / `quit`
  - `history`
  - `pinfo`
  - `repeat`
  - `jobs`
  - `sig`
  - `fg`
  - `bg`
  - `replay`
- Commands that are not builtin are executed using `execvp()` function.
- All commands accept paths in **relative to current working directory**, **relative to invocation location** (`~`) and **absolute** forms.
- Handles both piping and redirection, as well as combinations of both.
- Handles `SIGINT` (Ctrl+C), `SIGTSTP` (Ctrl+D) and `EOF` (Ctrl+D) in a manner similar to that by other shells like zsh or bash.

# Functionalities
In this section, we will explore the functionalities implemented in the shell in detail.

## Input and Parser
- `getline()` function is used to take raw input.
- Parser processses the input into a proccessed string, which consists of valid commands separated by the delimiter `;`.
- Each of these commands are tokenised using `strtok()` and sent ahead to `handle_pipes()` to check for piping, pre-execution.

## Piping
- The parser is used to count the total number of commands and thus, the total number of pipes.
- In case the total number of commands is 1 (ie: number of pipes are 0), the command is sent ahead for dispatch. Otherwise, it is sent ahead to handle piping.
- Piping is done using a 2-dimensional array `int file_desc[command_count-1][0]`.
- If the first command is being run, then then it's `STDOUT_FILENO` is set to `file_desc[0][1]`.
- For every subsequent command till the last command, if the index of the command is `i`, then `STDOUT_FILENO` is set to `file_desc[i][1]` and `STDIN_FILENO` is set to `file_desc[i-1][0]`.
- For the last command,`STDIN_FILENO` is set to `file_desc[i-1][0]` and `STDOUT_FILENO` is reset.
- After necessary piping step, each of the command is executed.
- After all the commands are executed, both `STDOUT_FILENO` and `STDIN_FILENO` are reset.

## Execution and Redirection
- The command is tokenised and stored `char *command` and `char *args[]`, where `command` contains the command to be executed and `args[]` contain all the arguments.
- During tokenisation, redirection is identified, if any. Necessary flags are set and the location of input and output files are stored.
- If there is no redirection, both `command` and `args` are sent to the dispatcher, which dispatches it to corresponding built-in function.
- If there is redirection, then the `STDIN_FILENO` and `STDOUT_FILENO` are set accordingly to the input and output files respectively, and then `command` and `args` are sent to the dispatcher.
- Both `STDOUT_FILENO` and `STDIN_FILENO` are reset, once the command is executed.

## Dispatcher 
- The dispatcher dispatches a command to corresponding built-in function.
- In case ths command is not built-in, it is dispatched to `void other_command(char *command, char *args[])`.
- From here, the command is sent to `void exec_back(char *args[])` or `void exec_fore(char *args[])` depending whether the last argument is `&` or not.

## `ls` Command
- In case multiple arguments are given, it prints the argument before printing the output corresponding to it 
- Both `-a` and `-l` flags have been implemented.
- For `-l` flag:
  - `total` is calculated by taking the sum of blocks given by `((file_stat.st_blocks * 512 + 1023) / 1024)` over all the files.
  - In case one of the file in output is a symbolic link, the curresponding real path is also displayed in the form `symbolic_link -> real_path`.
  - Time is of the format **MMM DD YYYY** if the timestamp is 6 months (15778476 seconds) old or is a future time, and **MMM DD HH:MM** format otherwise.

## `cd` Command
- `cd -` is implemented by storing previous location in global variable `PREV_LOC` everytime the directory is changed.

## `pinfo` Command
- Status and virtual memory are obtained from `/proc/<pid>/stat`.
- Executable path is obtained from link at `/proc/<pid>/exe`.

## `history`
- The history is stored in `~/.hash_history`.
- It is loaded before input is taken and rewritten once input is recieved.
- In case the current command is same as the last command in history, then the current command is not stored in history.

## Foreground Processes
- `pid` is generated by `fork()`.
- The commands are executed through `execvp()`.
- `waitpid()` is used to wait till termination.

## Background Processes
- Similar to Foreground Processes except `waitpid()` is not used.
- `signal()` is used to watch for the exit signal sent by the process and print the corresponding exit message.
- `setpgid()` is used to set the process group for background execution.

## Storing Processes
- Information regarding the processes are stored in multiple one dimensional arrays.
- The usage of the arrays is similar to that of map.
- These inlude: 
  * Map from pid to process name (`char *PROC_NAME[]`)
  * Map from pid to job number (`int JOB_NUM[]`)
  * Map from job number to pid (`pid_t JOB_PID[]`)
  * Map from pid to job foreground/background status (`int FORE_BACK[]`)

## `jobs`
- The flags are parsed from the input, for corresponding action.
- All background processes are collected and stored in an array, which is sorted according to alphabetical order of process name, using `qsort()`.
- Necessary information is obtained from `/proc/pid/stat` and printed accordingly.

## `sig`
- Input is checked, to ensure validity.
- Signal is send accordingly, using the `kill()` command.

## `fg`
- Input is checked, to ensure validity.
- Input and output signals (`SIGTTIN` and `SIGTTOU`) are set to ignore for the shell.
- The process of group of the background process is set to same as that of the shell, and `SIGCONT` is sent to it.
- In case an `SIGTSTP` is recieved, the process is pushed to background again.
- Process group of the process is set again, and the input and output signals (`SIGTTIN` and `SIGTTOU`) are reset to default.

## `bg`
- Input is checked, to ensure validity.
- `SIGCONT` is sent to the process.

## `replay`
- Parsing is done to ensure that input is valid.
- Command execute once at time `t=0`.
- `clock_t start` stores the start time of command execution.
- `clock_t keep` stores the time since last command execution.
- Command executes at every interval value in the given period.

## Signal Handling
- Apart from the implementation of error-message by handling `SIGCHLD`, both `SIGINT` (Ctrl+C) and `SIGTSTP` (Ctrl+Z) has been handled.
- Signal handling involves some necessary cleaning functions, which clean the process maps accordingly.
- Pressing Ctrl+Z or Ctrl+C on prompt will result in printing of a newline and printing of a fresh prompt for aesthetic purposes.
- `PROMPT_FL` flag is used to prevent double printing of prompt.

## `EOF` Handling
- On recieving `EOF` (Ctrl+D), the shell's loop is broken, using the return value of `-1` from `getline()` command.
- `killall()` function is called to kill all child process, so that the exit is clean,

# Assumptions 
- Maximum number of argument `MAX_ARG` for any command will be 100;
- Maximum input length`MAX_COMMAND` for any command will be `(int) (2 << 15)` .
- Maximum lenth of path `MAX_LOC` will be 10000.
- Maximum `pid` value `MAX_PID` will be less than `(int) (2 << 20)`.
- Maximum number of jobs in a session `MAX_JOBS` will be less than `(int) (100000)`.
- In replay, the command will execute at time 0 and then every interval in the time. 
  * In case the period is a multiple of the interval, it will not execute at the last second.
  *  For example: if the period is 2 and the inteval is 6, the command will execute at time 0, 2 and 4 seconds respectively.
  *  This implementation ensures that total runtime of the replay is equal to the specified interval.
- All child processes are killed on exit of shell.

# Running the Shell
Compile the shell from the parent directory and run the file as shown below:
```
$ make && ./hash
```
Alernatively you can run the precompiled executable provided in the repository:
```
$ ./hash
```

