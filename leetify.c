#include "leetify.h"
//use pointer arth. to go to the next
void execute_pipeline(struct command_line *cmds)
{
    if(!cmds->stdout_pipe)
    {
        if(cmds->stdout_file!=NULL)
        {
                int open_flags = O_RDWR | O_CREAT | O_TRUNC | O_APPEND | O_ASYNC
                | O_CLOEXEC;
                /*
                 * These are the file permissions we see when doing an `ls -l`: we can
                 * restrict access to the file. 0644 is octal notation for allowing the user
                 * to read and write the file, while everyone else can only read it.
                 */
                // before it was 0664
                int open_perms = 0666;
                int fd = open(cmds->stdout_file, open_flags, open_perms);
                if (fd == -1) {
                    perror("open");
                    return;
                }

                if (dup2(fd, STDOUT_FILENO) == -1) {
                    perror("dup2");
                    return;
                }
        }
        execvp(cmds->tokens[0],cmds->tokens);
        return;
        
    }else{
    //recursive
    /**
     * TODO: design a recursive algorithm that sets up a pipeline piece by
     * piece. Imagine you have three commands in a pipeline:
     *
     *  - cat
     *  - tr
     *  - sed
     *
     * This will result in three calls to execute_pipeline, one for each
     * command. Use 'stdout_pipe' to determine when you've reached the last
     * command, and 'stdout_file' to decide whether the final result gets
     * written to a file or the terminal.
     */

    /**
     * (1)
     * Here's our base case for our recursive execute_pipeline function. If
     * there are no more commands to run (stdout_pipe is false), then we just
     * need to execvp the last command. If redirection is enabled (stdout_file
     * is not NULL), then we also send stdout to the specified file.
     *
     * if there are no more commands:
     *     if stdout_file is not null:
     *         open stdout_file
     *         dup2 stdout to the file
     *
     *     execvp the command
     *     return
     */

    /**
     * (2)
     * If we aren't at the last command, then we need to set up a pipe for the
     * current command's output to go into. For example, let's say our command
     * is `cat file.txt`. We will create a pipe and have the stdout of the
     * command directed to the pipe. Before doing the recursive call, we'll set
     * up the stdin of the next process to come from the pipe, and
     * execute_pipeline will run whatever command comes next (for instance,
     * `tr '[:upper:]' '[:lower:]'`).
     *
     * create a new pipe
     * fork a new process
     * if pid is the child:
     *     dup2 stdout to pipe[1]
     *     close pipe[0]
     *     execvp the command
     * if pid is the parent:
     *     dup2 stdin to pipe[0]
     *     close pipe[1]
     *     call execute_pipeline on the next command
     */
    //recurise
        int fd[2];
        if (pipe(fd) == -1) {
            perror("pipe");
            return;
        }
        pid_t pid = fork();
        if(pid == 0)
        {
            dup2(fd[1], STDOUT_FILENO);
            close(fd[0]);
            execvp(cmds->tokens[0],cmds->tokens);
            //child
        }else if(pid ==-1){
            perror("fork\n");
        }else{
            //parent
            dup2(fd[0], STDIN_FILENO);
            close(fd[1]);
            execute_pipeline(++cmds);
        }
    }
}