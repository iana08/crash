#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>

struct command_line {
    char **tokens;
    bool stdout_pipe;
    char *stdout_file;
};

void execute_pipeline();