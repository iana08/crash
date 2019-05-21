#ifndef _HISTORY_H_
#define _HISTORY_H_

#define HIST_MAX 100

struct history_entry {
    unsigned long cmd_id;
    double run_time;
    char * command;
};

void print_history();
void write_history();
void clear_history();
char * get_cmd();
void timed_history();
void print_timed_history();


#endif
