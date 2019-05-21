#include <fcntl.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "debug.h"
#include "history.h"
#include "timer.h"
#include "tokenizer.h"
#include "leetify.h"
#include "expansion.h"

void print_prompt()                                                                                         ;
void get_prompt()                                                                                           ;
void replace_homedir()                                                                                      ;

void cd_the_professor()                                                                                     ;
bool delete_this()                                                                                          ;
void updatecwd()                                                                                            ;

#define MAXBUF 128
#define TOKENBUF 4200
#define MAXJOB 10
unsigned int i = 0                                                                                          ;
unsigned int cmd_id = 0                                                                                     ;
char home_dir[50] = "/home/";
char hostname[MAXBUF];
char * username; 
char cwd[MAXBUF]; 
char temp_cwd[MAXBUF];
char temp_dir[MAXBUF] = "~";
char parent_cwd[MAXBUF];
bool done = true;
bool block = false;
int * envret;
bool history_bool = true;
bool redirect_bool = false;
bool pipe_bool = false;
bool stdout_file = false;
int jobs_id = 0                                                                                                                       ;
int cmd_tmp =0                                                                                                                       ;
unsigned int cmd_ind = 0                                                                                                                       ;
bool hide_time = false                                                                                                                       ;

struct jobs                                                                                                 {
    char * command                                                                                                                                                                                                                 ;
    unsigned int child_pid                                                                                  ;
    };

struct jobs *jobs_list[MAXJOB] = {NULL}                                                                     ;

// my signal handler for exit
void sigusr1_handler(int signo)                                                                              {
    done = false                                                                                                                       ;
    // free(envret);
    exit(0)                                                                                                     ;
}
//My signal handler for when my child quits
void sigchild_handler(int signo)                                                                                {
    int status;
    unsigned int pid = waitpid(-1, &status, WNOHANG)                                                                                                                       ;
    int i                                                                                                                       ; 

    for(i=0;i<MAXJOB;i++)
        {
        if(jobs_list[i] != NULL)
        {
            if(jobs_list[i]->child_pid == pid)
            {
                if(jobs_list[i+1]!=NULL){
                    free(jobs_list[i] -> command)                                                                                                                       ;
                    free(jobs_list[i])                                                                                                                       ;
                    jobs_list[i] = jobs_list[i+1]                                                                                                                       ;
                    jobs_list[i+1] = (NULL)                                                                                                                       ;
                }
                else{
                    free(jobs_list[i])                                                                                                                       ;
                    jobs_list[i] = (NULL)                                                                                                                       ;
                }
            }
        }
    }
}

// when the user wants to quit but not the shell
void sigint_handler(int signo)                                                                              {
    printf("\n");
    print_prompt();
}

// I wonder what this does
void print_jobs_J_O_B_S()
{
    int i;
    for(i=0;i<MAXJOB;i++)
    {
        if(jobs_list[i]!=NULL)
        {
            printf(" HE_HE %s",jobs_list[i]->command);
            fflush(stdout);
        }
    }
}

void print_prompt(void)
{
    if(isatty(STDIN_FILENO)){
            printf("\n--[%d|%s@%s:%s]--$ ",i,username,hostname,cwd);
            i++;
            fflush(stdout);
    }
}

void get_prompt(void)
{
    username = getlogin();
    gethostname(hostname, sizeof(hostname));
    strcat(home_dir,username);
    getcwd(temp_cwd,sizeof(temp_cwd));
    strcpy(parent_cwd, temp_cwd);
    if(strstr(temp_cwd, home_dir) != NULL)
    {
        replace_homedir();
    }
    else
    {
        strcpy(cwd,temp_cwd);
    }
}

void replace_homedir(void)
{
    int j;
    int i =0;
    i++;
    for(j = strlen(home_dir); j<MAXBUF;j++)
    {
        temp_dir[i] = temp_cwd[j];
        i++;
    }
    strcpy(cwd,temp_dir);
}

// changes the current directory
void cd_the_professor(char * tokens[])
{
    int i = 0;
    int j = 0;
    while(tokens[i] != NULL)
    {
        j++;
        if((strcmp(tokens[i], "cd") == 0) && tokens[j] != NULL)
        {
            if (chdir(tokens[j]) != 0)  
                perror("chdir() to /usr failed"); 
        }else if(strcmp(tokens[i], "cd") == 0 && tokens[j] == NULL)
        {
            if (chdir(home_dir) != 0)  
                perror("chdir() to /usr failed"); 
        }        
        i++;
    }
}

// gets rids of comments
bool delete_this(char * this)
{
    char *next_tok2 = this;
    char *curr_tok2;
    char *pp;
    char lazy_ptr;
    while ((curr_tok2 = next_token(&next_tok2, "", &lazy_ptr)) != NULL) {
        pp = strstr(curr_tok2,"#");
        if(pp!=NULL)
        {
            return false;
        }
    }
    return true;
}

// updates the current working directory
void updatecwd(void)
{
    getcwd(temp_cwd,sizeof(temp_cwd));
    char* ptr = strstr(temp_cwd, home_dir);
    if(ptr != NULL && strlen(ptr) == strlen(temp_cwd))
    {
        replace_homedir();
    }else
    {
        strcpy(cwd, temp_cwd);
    }
}

// MY MAIN SQUEEZE
int main(void) {
    signal(SIGTERM, sigusr1_handler);
    signal(SIGINT, sigint_handler);
    signal(SIGCHLD, sigchild_handler);
    get_prompt();
    int pipe_num = 0;
    while (true)
    {
        hide_time = false;
        redirect_bool = false;
        block = false;
        history_bool =true;
        pipe_bool = false;
        print_prompt();
        
        char *line = NULL;
        size_t line_sz = 0;
        int ret; 
        ret = getline(&line,&line_sz,stdin); 

        char *temp_cmd = strstr(line,"!");


        char *new_temp_cmd = strstr(line, "&");
        char lazy_ptr;
        new_temp_cmd = next_token(&new_temp_cmd,"\n", &lazy_ptr);

        if(ret == EOF)
        {
            fclose(stdin);
            free(line);
            clear_history();
            exit(0);
        }
        else if(temp_cmd!=NULL && strlen(temp_cmd) == (strlen(line)))
        {
            temp_cmd++;
            temp_cmd = strdup(temp_cmd);
            free(line);
            strcpy(line,get_cmd(temp_cmd));
            free(temp_cmd);
            history_bool = false;
        }
        else if(new_temp_cmd!=NULL && strlen(new_temp_cmd) == 1){
            line = next_token(&line, "&", &lazy_ptr);
            line = strcat(line, "\n");
            block = true;
        }
        char *comd = strdup(line);

        write_history(comd,cmd_id);
        cmd_id++;

        LOG("Got line: --> %s", line);

        char *tokens[TOKENBUF];
        int i = 0;
        char *next_tok = line;
        char *curr_tok;
        char quoue = 0;
        while ((curr_tok = next_token(&next_tok, " \t\r\n", &quoue)) != NULL) 
        {
            if(strcmp(curr_tok,"exit") == 0)
            {
                free(comd);
                free(line);
                clear_history();
                raise(SIGTERM);
                fclose(stdin);
                break;
            }else if(!delete_this(curr_tok))
            {
                break;
            }else
            {
                if(i>TOKENBUF)
                {
                    perror("Too much data for the buffer\n");
                    break;
                }
                if(strcmp("|",curr_tok)==0)
                {
                    pipe_num++;
                    pipe_bool = true;
                }
                if(strcmp(">",curr_tok)==0){
                    redirect_bool = true;
                    pipe_bool = true;
                }
                if(strcmp("-t",curr_tok) == 0)
                {
                    hide_time = true;
                }
                tokens[i] = curr_tok;
                i++;
            }
        }
        tokens[i] = (char *) NULL;

        struct command_line cmds[pipe_num+1];

        int end = 0;
        if(strlen(&quoue) != 0)
        {
            while(tokens[end] != NULL)
            {
                int tem = strcspn(tokens[end],&quoue);
                int ine;
                if(tem<strlen(tokens[end]))
                {
                    memmove(&tokens[end][tem],&tokens[end][tem+1],strlen(tokens[end]) - tem+1);
                }

                for(ine=tem;ine<strlen(tokens[end]);ine++)
                {
                    if(strcmp(&tokens[end][ine],&quoue) == 0)
                    {
                        memmove(&tokens[end][ine],&tokens[end][ine+1],strlen(tokens[end]) - ine+1);
                    }
                }
                end++;
            }
        }
        int index = 0;
        while(tokens[index] != NULL)
        {
            char* doll_ptr = strstr(tokens[index],"$");
            while(doll_ptr != NULL)
            {
                tokens[index] = expand_var(tokens[index]);
                doll_ptr = strstr(tokens[index],"$");
            }
            index++;
            
        }

        if(tokens[0] != NULL && (pipe_bool || redirect_bool))
        {
            char* cmd_arr[50] = {0};
            int end;
            bool found = false;
            cmd_tmp =0;
            int tok_tmp=0;
            int tmp = 0;

            for(end=0 ; end<i ; end++)
            {
                if(strcmp(tokens[end], "|") == 0 || strcmp(tokens[end], ">") == 0)
                {
                    cmd_arr[tmp]= (char *) NULL;
                    found = true;
                    cmds[cmd_tmp].tokens = calloc(tmp+1, sizeof(char*));
                    tok_tmp = 0;
                    while(cmd_arr[tok_tmp] != NULL)
                    {
                        cmds[cmd_tmp].tokens[tok_tmp] = cmd_arr[tok_tmp];
                        tok_tmp++;
                    }
                    cmds[cmd_tmp].tokens[tok_tmp] = (char*) NULL;

                    if(redirect_bool && strcmp(tokens[end], ">") == 0)
                    {
                        cmds[cmd_tmp].stdout_pipe = false;
                        ++end;
                        stdout_file = true;
                        cmds[cmd_tmp].stdout_file = tokens[end];
                    }else
                    {
                        cmds[cmd_tmp].stdout_pipe = true;
                        cmds[cmd_tmp].stdout_file = NULL;
                    }

                    memset(cmd_arr,0,tmp*(sizeof(cmd_arr[0])));// for valgrind

                    cmd_tmp++;
                    tmp = 0;
                }else{
                    cmd_arr[tmp]=tokens[end];
                    tmp++;
                }
            }
            if(found)
            {
                cmd_arr[tmp]= (char *) NULL;
                cmds[cmd_tmp].tokens = calloc(tmp+1, sizeof(char*));
                int tok_tmp=0;
                while(cmd_arr[tok_tmp]!=NULL)
                {
                    cmds[cmd_tmp].tokens[tok_tmp] = cmd_arr[tok_tmp];
                    tok_tmp++;
                }
                cmds[cmd_tmp].tokens[tok_tmp] = (char*) NULL;

                cmds[cmd_tmp].stdout_pipe = false;
                cmds[cmd_tmp].stdout_file = NULL;
            }
        }

        if(tokens[0] != NULL)
        {
            if(strcmp(tokens[0], "cd") == 0)
            {
                double start = get_time();
                cd_the_professor(tokens);
                updatecwd();
                double end = get_time();
                timed_history(end-start);
            }
            else if(strcmp(tokens[0], "setenv") == 0)
            {
                if(tokens[2] != NULL && tokens[1] != NULL)
                {
                    double start = get_time();
                    setenv(tokens[1],tokens[2],0);
                    double end = get_time();
                    timed_history(end-start);
                }
            }
            else if(strcmp(tokens[0], "history") == 0)
            {
                if(hide_time)
                {
                    printf("fsd\n");
                    print_timed_history();
                }else{
                    double start = get_time();
                    print_history();
                    double end = get_time();
                    timed_history(end-start);
                    continue;  
                }
                
            }
            else if(strcmp(tokens[0], "jobs") == 0)
            {
                double start = get_time();
                print_jobs_J_O_B_S();
                double end = get_time();
                timed_history(end-start);
            }

            double start = get_time();
            pid_t pid = fork();
            if(pid == 0)
            {
                fclose(stdin);

                if(pipe_bool || redirect_bool)
                {
                    double start = get_time();
                    execute_pipeline(cmds);
                    double end = get_time();
                    timed_history(end-start);

                }else
                {
                    
                    int ret = execvp(tokens[0], tokens);
                    if(ret == -1)
                    {
                        perror("Nothing is going to happen.\n");
                        exit(0);
                    }
                }

                
            }else if(pid == -1)
            {
                perror("fork");
            }
            else
            {
                //Parent
                int status;
                if(!block)
                {
                    waitpid(pid, &status, 0);
                    double end = get_time();
                    timed_history(end-start);
                }else
                {
                    struct jobs * job;
                    job = (struct jobs *) calloc(1, sizeof(struct jobs));
                    job -> command = strdup(comd);
                    job -> child_pid = pid;
                    jobs_list[jobs_id] = job;
                    jobs_id++;

                    if(jobs_id>MAXJOB)
                    {
                        jobs_id = 0;
                    }
                }
                
                i++;
                LOG("Child exited. Status %d\n", status);
            }
        }

        if(pipe_bool || redirect_bool)
        {
            int kilo;
            for(kilo = 0; kilo < cmd_tmp+1 ; kilo++)
            {
                if(cmds[kilo].tokens != NULL)
                {
                    int ban = 0;
                    while(cmds[kilo].tokens != NULL && cmds[kilo].tokens[ban] != NULL)
                    {
                        if(cmds[kilo].tokens != NULL)
                        {
                           free(cmds[kilo].tokens); 
                        }
                        cmds[kilo].tokens = NULL;
                        ban++;
                    }
                }
            }
        }
        free(comd);
        free(line);
    }
    return 0;
}