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
void parse_command(char **args);
void display_help();

char user_name[64];
char last_path[BUFFER];
char current_dir[BUFFER];
char formatted_current_dir[BUFFER];
int arg;

int main()
{
    register uid_t user_id;
    register struct passwd *user;
    char command_line[BUFFER];
    char *tmp_str;
    char args_s[20][BUFFER];
    char *args[20];
    char breaking_char[] = " ";
    int quote_count, i;

    getcwd(current_dir, sizeof(current_dir));
    format_home_path();
    user_id = geteuid();
    user = getpwuid(user_id);
    strcpy(user_name, user->pw_name);

    while(true){
        arg = 0;
        getcwd(current_dir, sizeof(current_dir));

        printf("[%s%s%s:%s%s%s]\n", CYN_CLR, user_name, REG_CLR, GRN_CLR, current_dir, REG_CLR);
        format_home_path();
        printf("[%s%s%s:%s%s%s]", CYN_CLR, user_name, REG_CLR, GRN_CLR, formatted_current_dir, REG_CLR);
        printf("$ ");

        fgets(command_line, sizeof(command_line), stdin);
        args[0] = strtok(command_line, breaking_char);

        while(true){
            quote_count = 0;
            if(args[arg] == NULL) break;
/*
            for(i = 0; i < strlen(args[arg]); i++){
                if(args[arg][i] == '\'')  quote_count++;
            }
            if(quote_count % 2 != 0){

                tmp_str = strtok(NULL, "\'");
                if(tmp_str == NULL) break;
                strcat(args[arg], tmp_str);
                strcat(args[arg], "'");

                arg++;
                args[arg] = strtok(NULL, " ");

                if(args[arg] == NULL) break;

                if((int)args[arg][0] == 47){
                    strcat(args[arg-1], args[arg]);
                    arg--;
                }
            }
*/

            arg++;
            args[arg] = strtok(NULL, breaking_char);
        }

        parse_command(args);


        strcpy(last_path, current_dir);
    }

    return 0;
}

void parse_command(char **args){
    int i;
    for(i = 0; i < arg; i++){
        if((int)args[i][0] == 10){
            arg--;
            break;
        }
    }
    for(i = 0; i < strlen(args[arg-1]); i++){
        if((int)args[arg-1][i] == 32 || (int)args[arg-1][i] == 10){
            args[arg-1][i] = '\0';
        }
    }

    if(strcmp(args[0], "cd") == 0){
        if(arg == 1) change_dir("~");
        else if(change_dir(args[1]) != 0) printf("Directory doesn't exist!\n");
    }
    else if(strcmp(args[0], "exit") == 0){
        exit(0);
    }
    else if(strcmp(args[0], "help") == 0){
        display_help();
    }
}

void display_help(){
    printf("*** MicroShell SOP ***\n");
    printf("Author: Mateusz Drewniak\n\n");
    printf("--- Features: ---\n");
    printf("* `cd` command - changes the working directory\n");
    printf("* `help` command - displays this help message\n");
    printf("* `exit` command - exits the programme\n");
    printf("* other commands - try typing in other commands known from regular terminals,\n");
    printf("  this programme will attempt to find them using the PATH environment variable\n");
    printf("  and execute them for you\n");
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
