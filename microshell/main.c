#include <stdbool.h>
#include <dirent.h>
#include <errno.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUFFER 512
#define CYN_CLR "\e[0;36m"
#define GRN_CLR "\e[0;32m"
#define REG_CLR "\e[0;0m"

int change_dir(char *target_path);
void del_from_str(char *base_string, char *target_string, char first_del_char);
void format_home_path();
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
    register uid_t user_id;
    register struct passwd *user;
    int i, history_breaker, tmp_id;

    getcwd(current_dir, sizeof(current_dir));
    format_home_path();
    user_id = geteuid();
    user = getpwuid(user_id);
    strcpy(user_name, user->pw_name);
    strcpy(history, "");

    while(true){
        arg = 0;
        getcwd(current_dir, sizeof(current_dir));

        printf("[%s%s%s:%s%s%s]\n", CYN_CLR, user_name, REG_CLR, GRN_CLR, current_dir, REG_CLR);
        format_home_path();
        printf("[%s%s%s:%s%s%s]", CYN_CLR, user_name, REG_CLR, GRN_CLR, formatted_current_dir, REG_CLR);
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
        else if(command_line[i] == '\''){
            quotes = true;
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

    if(quotes == true) printf("Improper syntax!\n");
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
        printf(history);
    }
    else if(strcmp(args_s[0], "echo") == 0){
        echo();
    }
    else{
        printf("Unknown command!\n");
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
    char *tmp_path;
    char current_dir_copy[BUFFER];
    char home_path[BUFFER];

    strcpy(home_path, "/home/");
    strcat(home_path, user_name);

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
        tmp_path = strtok(current_dir_copy, "/");

        if(strcmp(tmp_path, "home") == 0){
            tmp_path = strtok(NULL, "/");

            if(strcmp(tmp_path, user_name) == 0){
                tmp_path = strtok(NULL, "");
                strcpy(formatted_current_dir, "~/");
                strcat(formatted_current_dir, tmp_path);
            }
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
    int ret_val = 0;

    if(strcmp(target_path, "~") == 0){
        strcpy(parsed_path, "/home/");
        strcat(parsed_path, user_name);
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