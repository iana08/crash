#include "history.h"
#include "tokenizer.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#define TOKENBUF 4200

struct history_entry *list_of_commands[HIST_MAX]= {NULL};
int world_history = 0;
//This functin should print history entries
void print_history() {
    /* This function should print history entries */

    int i=world_history;
	while(list_of_commands[i]!=NULL && i<HIST_MAX)
	{
		printf("%ld %s",list_of_commands[i]->cmd_id, list_of_commands[i]->command);
		i++;
	}
	i=0;
	while(list_of_commands[i]!=NULL && i<world_history)
	{
		printf("%ld %s",list_of_commands[i]->cmd_id, list_of_commands[i]->command);
		i++;
	}
}

// This function should write history 
void write_history(char * cmd, int cmd_id)
{
	if(list_of_commands[world_history] != NULL){
		free(list_of_commands[world_history] -> command);
		free(list_of_commands[world_history]);
	} 

    struct history_entry * cmd_write = (struct history_entry *)calloc(1 , sizeof(struct history_entry));
   	cmd_write -> command = strdup(cmd);
	cmd_write->cmd_id = cmd_id;	
	list_of_commands[world_history] = cmd_write;

	++world_history;
	if(world_history > HIST_MAX-1)
		world_history = 0;
}

//This should get the command given a number, !, or a command
char * get_cmd(char * cmd)
{
	int val;
	val = atoi(cmd);
	if(val!=0 || strcmp(cmd,"0\n") == 0)
	{
		int temp_wh = world_history;
		temp_wh--;

		if(temp_wh<0)
		{
			temp_wh = HIST_MAX-1;
		}
		temp_wh++;

		while(temp_wh>=0)
		{
			if(list_of_commands[temp_wh]!= NULL)
			{
				if(val == list_of_commands[temp_wh]->cmd_id)
				{
					return list_of_commands[temp_wh]->command;
				}
			}
			temp_wh--;
		}

		temp_wh= HIST_MAX-1;
		while(temp_wh>=world_history)
		{
			if(list_of_commands[temp_wh]!= NULL)
			{
				if(val == list_of_commands[temp_wh]->cmd_id)
				{
					return list_of_commands[temp_wh]->command;
				}
			}
			temp_wh--;
		}
	}else if(strcmp(cmd,"!\n") == 0)
	{
		return list_of_commands[world_history-1]->command;
	}else
	{
		char lazy_ptr;
		cmd = next_token(&cmd,"\n",&lazy_ptr);
		if (cmd == NULL)
		{
			return cmd;
		}

		int temp_wh = world_history;
		temp_wh--;

		if(temp_wh<0)
		{
			temp_wh = HIST_MAX-1;
		}

		while(temp_wh>=0)
		{
			if(list_of_commands[temp_wh]!= NULL)
			{
				char *line;
				line = strdup(list_of_commands[temp_wh]->command);
		        char *next_tok = line;
		        char *curr_tok;
		        while ((curr_tok = next_token(&next_tok, " \t\r\n",&lazy_ptr)) != NULL) 
		        {
		        	char* cmdptr=strstr(curr_tok,cmd); 
		       		if((strcmp(curr_tok,cmd)==0 || cmdptr!=NULL) && strlen(cmdptr) == strlen(curr_tok))
					{
						free(line);
						return list_of_commands[temp_wh]->command;
					}
		        }
			}
			temp_wh--;
		}

		temp_wh= HIST_MAX-1;
		while(temp_wh>=world_history)
		{
			if(list_of_commands[temp_wh]!= NULL)
			{
				char *line;
				line = strdup(list_of_commands[temp_wh]->command);
		        char *next_tok = line;
		        char *curr_tok;
		        while ((curr_tok = next_token(&next_tok, " \t\r\n", &lazy_ptr)) != NULL) 
		        {
		        	char* cmdptr=strstr(curr_tok,cmd); 
		        	if((strcmp(curr_tok,cmd)==0 || cmdptr!=NULL) && strlen(cmdptr) == strlen(curr_tok))
					{
						free(line);
						return list_of_commands[temp_wh]->command;
					}
		        }
			}
			temp_wh--;
		}

	}
	return cmd;
}

//Gets the run time for the command and puts it in the struct
void timed_history(double time)
{
	int temp = world_history;
	--temp;
	if(list_of_commands[temp] != NULL)
	{
		list_of_commands[temp]->run_time = time;
	}
	
}

//prints the entries with the history command
void print_timed_history()
{
	int i=world_history;
	while(list_of_commands[i]!=NULL && i<HIST_MAX)
	{
		printf("%ld : %fs %s",list_of_commands[i]->cmd_id, list_of_commands[i]->run_time, list_of_commands[i]->command);
		i++;
	}
	i=0;
	while(list_of_commands[i]!=NULL && i<world_history)
	{
		printf("%ld : %fs %s",list_of_commands[i]->cmd_id, list_of_commands[i]->run_time, list_of_commands[i]->command);
		i++;
	}
}

//Now nothing is here,... Nothing is to remain
void clear_history()
{
	int i;
	for(i=0;i<HIST_MAX;i++)
	{
		if(list_of_commands[i]!=NULL)
		{
			free(list_of_commands[i] -> command);
			free(list_of_commands[i]);
		}
	}
}