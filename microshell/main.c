#include <stdbool.h>
#include <dirent.h>
#include <errno.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUFFER 512
#define CYN_CLR "\e[1;36m"
#define GRN_CLR "\e[1;32m"
#define YLW_CLR "\e[1;33m"
#define BLU_CLR "\e[1;34m"
#define REG_CLR "\e[0;0m"

int change_dir(char *target_path);
void del_from_str(char *base_string, char *target_string, char first_del_char);
void format_home_path();
void execute_command();
void parse_command();
void display_help();
void echo();

char user_name[64];
char last_path[BUFFER];
char current_dir[BUFFER];
char formatted_current_dir[BUFFER];
char command_line[BUFFER];
char args_s[20][BUFFER];
char history[BUFFER * 2];
int arg;

int main()
{
    char tmp_str[1024];
    struct utsname local_name;
    register uid_t user_id;
    register struct passwd *user;
    int i, history_breaker, tmp_id;

    getcwd(current_dir, sizeof(current_dir));
    format_home_path();
    user_id = geteuid();
    uname(&local_name);
    user = getpwuid(user_id);
    strcpy(user_name, user->pw_name);
    strcpy(history, "");
    printf("%sWelcome to %s***%sMatiShell%s***%s\n", CYN_CLR, GRN_CLR, YLW_CLR, GRN_CLR, REG_CLR);

    while(true){
        arg = 0;
        getcwd(current_dir, sizeof(current_dir));

        format_home_path();
        printf("%s%s@%s%s:%s%s%s", CYN_CLR, user_name, local_name.nodename, REG_CLR, GRN_CLR, formatted_current_dir, REG_CLR);
        printf("$ ");

        fgets(command_line, sizeof(command_line), stdin);
        if(strlen(history) > 3 * BUFFER / 2){
            tmp_id = 0;
            for(i = 0; i < strlen(history); i++)
                if(history[i] == '\n' && i > BUFFER / 4) break;
            history_breaker = i + 1;

            for(i = history_breaker; i <= strlen(history); i++){
                tmp_str[tmp_id] = history[i];
                tmp_id++;
            }

            strcpy(history, tmp_str);
            strcpy(tmp_str, "");
        }
        strcat(history, command_line);
        parse_command();


        strcpy(last_path, current_dir);
    }

    return 0;
}

void parse_command(){
    int i = 0;
    int curr_arg = 0;
    int curr_char = 0;
    bool word = false;
    bool quotes = false;
    bool double_quotes = false;

    while(i < strlen(command_line)){
        if(quotes){
            if(command_line[i] == '\''){
                quotes = false;
            }
            else{
                args_s[curr_arg][curr_char] = command_line[i];
                curr_char++;
            }
        }
        else if(double_quotes){
            if(command_line[i] == '\"'){
                double_quotes = false;
            }
            else{
                args_s[curr_arg][curr_char] = command_line[i];
                curr_char++;
            }
        }
        else if(command_line[i] == '\''){
            quotes = true;
            word = true;
        }
        else if(command_line[i] == '\"'){
            double_quotes = true;
            word = true;
        }
        else if(command_line[i] == ' ' || command_line[i] == '\n'){
            if(word){
                word = false;
                args_s[curr_arg][curr_char] = '\0';
                curr_arg++;
                curr_char = 0;
            }
        }
        else{
            if(!word) word = true;
            args_s[curr_arg][curr_char] = command_line[i];
            curr_char++;
        }

        i++;
    }

    for(i = 0; i < curr_arg; i++){
        if((int)args_s[i][0] == 10){
            curr_arg--;
            break;
        }
    }
    for(i = 0; i < strlen(args_s[curr_arg]); i++){
        if((int)args_s[curr_arg][i] == 32 || (int)args_s[curr_arg][i] == 10){
            args_s[curr_arg][i] = '\0';
        }
    }
    arg = curr_arg;
/*
    for(i = 0; i < curr_arg; i++){
        printf("%s \n", args_s[i]);
        for (int j = 0; j < strlen(args_s[i]); j++)
        {
            printf("%d ", args_s[i][j]);
        }
        printf("\n");
    }

    printf("%d \n", curr_arg);
*/

    if(quotes || double_quotes) printf("Improper syntax!\n");
    else if(strcmp(args_s[0], "cd") == 0){
        if(arg == 1) change_dir("~");
        else if(arg > 2) printf("Too many arguments!\n");
        else if(change_dir(args_s[1]) != 0) printf("Directory doesn't exist!\n");
    }
    else if(strcmp(args_s[0], "exit") == 0){
        exit(0);
    }
    else if(strcmp(args_s[0], "help") == 0){
        display_help();
    }
    else if(strcmp(args_s[0], "history") == 0){
        printf("%s", history);
    }
    else if(strcmp(args_s[0], "echo") == 0){
        echo();
    }
    else{
        execute_command();
        /* printf("Unknown command!\n"); */
    }
}

void execute_command(){
    int i, fork_val;

    char **arg_copy;
    arg_copy = malloc((arg + 1) * sizeof(char *));
    for(i = 0; i <= arg; i++)
        arg_copy[i] = malloc(BUFFER * sizeof(char));


    for(i = 0; i < arg; i++)
        arg_copy[i] = args_s[i];
    arg_copy[arg] = NULL;

    fork_val = fork();

    if(fork_val == -1){
        printf("Unable to create a child process!\n");
    }
    else if(fork_val == 0){     /* Child process */
        if(execvp(arg_copy[0], arg_copy) < 0)
            printf("Unknown command!\n");
        exit(0);
    }
    else{                       /* Parent process */
        wait(NULL);
    }


}

void display_help(){
    printf("*** MicroShell SOP ***\n");
    printf("Author: Mateusz Drewniak\n\n");
    printf("--- Features: ---\n");
    printf("* `cd` command - changes the working directory\n");
    printf("* `help` command - displays this help message\n");
    printf("* `exit` command - exits the programme\n");
    printf("* `history` command - prints this message and automatically deletes old entries\n");
    printf("* `echo` command - prints all the passed arguments\n");
    printf("* other commands - try typing in other commands known from regular terminals,\n");
    printf("  this programme will attempt to find them using the PATH environment variable\n");
    printf("  and execute them for you\n");
}

void echo(){
    int i;
    for(i = 1; i < arg; i++)
        printf("%s ", args_s[i]);
    printf("\n");
}

void format_home_path(){
    int i, j, new_path_length = 0;
    char *tmp_path;
    char *tmp_str;
    char current_dir_copy[BUFFER];
    char home_path[BUFFER];
    bool home_dir = false;

    tmp_str = getenv("HOME");
    strcpy(home_path, tmp_str);

    strcpy(current_dir_copy, current_dir);
    strcpy(formatted_current_dir, current_dir);

    if(strcmp(current_dir, home_path) == 0){
        strcpy(formatted_current_dir, "~");
    }
    else if(strcmp(current_dir, "/") == 0){
        strcpy(formatted_current_dir, "/");
    }
    else if(strcmp(current_dir, "/home") == 0){
        strcpy(formatted_current_dir, "/home");
    }
    else{
        for(i = 0; i < strlen(current_dir); i++){
            current_dir_copy[i] = '\0';
            if(strcmp(current_dir_copy, home_path) == 0){
                home_dir = true;
                break;
            }
            current_dir_copy[i] = current_dir[i];
        }

        if(home_dir){
            new_path_length = strlen(current_dir) - strlen(current_dir_copy) + 1;
            strcpy(current_dir_copy, "~");
            for(j = 1; j < new_path_length; j++){
                current_dir_copy[j] = current_dir[i];
                i++;
            }
            current_dir_copy[j] = '\0';
            strcpy(formatted_current_dir, current_dir_copy);
        }
    }
}

void del_from_str(char *target_string, char *base_string, char first_del_char){
    int i;
    for(i = strlen(base_string) - 1; i >= 0; i--){
        if(base_string[i] == first_del_char){
            strncpy(target_string, base_string, i + 1);
            target_string[i+1] = '\0';
            break;
        }
    }
}

int change_dir(char *target_path){
    char *parsed_path = (char *)malloc(BUFFER * sizeof(char));
    char *tmp_str;
    int ret_val = 0;

    if(strcmp(target_path, "~") == 0){
        tmp_str = getenv("HOME");
        strcpy(parsed_path, tmp_str);
    }
    else if(strcmp(target_path, "..") == 0){
        del_from_str(parsed_path, current_dir, '/');
    }
    else if(strcmp(target_path, "-") == 0){
        strcpy(parsed_path, last_path);
    }
    else{
        strcpy(parsed_path, target_path);
    }

    ret_val = chdir(parsed_path);
    free(parsed_path);

    return ret_val;
}
