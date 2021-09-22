#ifndef UTILS_H
#define UTILS_H

void clear();                                                           // clears the screen
void prompt();                                                          // prints the prompt string
void relative_path(char *path);                                         // given a path, generates path relative to the starting location
int await_input();                                                      // takes input and sends it for further processing
int take_input(char *output_string);                                    // logic for taking input
void call_command(char *raw_string);                                    // logic of processing and calling commands
int process_raw_string(char *raw_string, char *processed_string);       // processing the raw string to remove tabs, extra space, trailing space, newline, ampersand and empty commands
void exec_command(char *command_string);                                // executes each tokenised command
void dispatch(char *command, char *args[]);                             // dispatches the function calls for the commands
void other_command(char *command, char *args[]);                        // handles commands that are not inbuilt
void get_history();                                                     // gets content from history file (~/.hash_history)
void add_history(char *command);                                        // writes content to history file (~/.hash_history)
void handle_redirection(char *command, char *args[],\
                         int i_flag, int o_flag,\
                          char* i_file, char* o_file);

#endif