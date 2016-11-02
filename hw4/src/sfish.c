#include "sfish.h"

int main(int argc, char** argv, char *envp[]) {
    

    //DO NOT MODIFY THIS. If you do you will get a ZERO.
    rl_catch_signals = 0;
    //This is disable readline's default signal handlers, since you are going
    //to install your own.

    preprocess();

    char *cmd;
    int len = 5 + LOGIN_NAME_MAX + HOST_NAME_MAX + PATH_MAX + 20 + 40;
    // 20 for extra, 5 for "sfish", 40 for extra escape chars
    char *promptBuf = (char*) malloc(len); 

    while((cmd = readline(getsnPrompt(promptBuf, len))) != NULL) {
        int next_pipe = 0;
        if (strcmp(cmd,"exit")==0)
            break;

        if(*cmd == '\0'){
            debug("cmd is empty.. continue\n");
            continue;
        }
        add_history(cmd);


        //All your debug print statments should be surrounded by this #ifdef
        //block. Use the debug target in the makefile to run with these enabled.
        debug("Length of command entered: %s, len=%ld @ %p\n", cmd, strlen(cmd), cmd);
        //You WILL lose points if your shell prints out garbage values.

        int argc = countElements(cmd) + 1; // + 1 for adding null argv
        char *buf[argc];
        char **argv = parseNCmd(&cmd, buf, argc);
        int pipe_fd[2] = {STDIN_FILENO, STDOUT_FILENO};

        if(argv == NULL){
            debug("Nothing to parse, continue\n");
            continue;
        }

        for(int i = 0; i < argc; i++){
            debug("argc=%d, argv[%d]=%s\n", argc, i, argv[i]);
        }
        
        do {
            debug("do-while loop next_pipe=%d\n", next_pipe);
            int open_fd = -1, prog;

            if(next_pipe == 0){
                prog = next_pipe;
            } else{
                prog = next_pipe + 1;
            }

            if( isBuiltin(argv[prog]) != SF_FALSE ){ // if not SF_FALSE, then it is true

                if( strcmp(argv[prog], "cd") == 0 ||
                    strcmp(argv[prog], "chpmt") == 0 || 
                    strcmp(argv[prog], "chclr") == 0 ){ // if cd do not fork

                    // execute the built in program
                    debug("No fork next_pipe=%d\n", next_pipe);
                    last_exe.val = exeBuiltIn(argc, (argv + next_pipe));

                } else{ // fork is necessary
                    
                    // if first time parsing this cmd line, check for pipeline
                    if (next_pipe == 0){
                        // find the next pipe character's location
                        next_pipe = getNextPipe(argc, argv, next_pipe);
                    }

                    // check the pipe
                    if( strcmp(argv[next_pipe], "|") == 0 ){
                        
                    } else if( strcmp(argv[next_pipe], ">") == 0 ){
                        char* filename = argv[next_pipe + 1];
                        debug("filename=%s\n", filename);
                        // open write only, create if not exists, trucate if eixsts
                        open_fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                        if(open_fd == -1){
                            perror("open(filename, O_WRONLY | O_CREAT | O_TRUNC, 644) failed");
                            return -1;
                        }

                        pipe_fd[WRITE_END] = open_fd;

                        argv[next_pipe] = 0;

                    } else if( strcmp(argv[next_pipe], "<") == 0 ){
                        char* filename = argv[next_pipe + 1];

                        // open read only
                        open_fd = open(filename, O_RDONLY);
                        if(open_fd == -1){
                            perror("open(filename, O_RDONLY) failed");
                            return -1;
                        }

                        pipe_fd[READ_END] = open_fd;                        

                        argv[next_pipe] = 0;
                    }

                    Fork_Builtin(pipe_fd, argc, argv, prog);
                }
            }
            else{ // not builtin program

                // if first time parsing this cmd line, check for pipeline
                if (next_pipe == 0){

                    // find the next pipe character's location
                    next_pipe = getNextPipe(argc, argv, next_pipe);
                }

                // check the pipe
                if( strcmp(argv[next_pipe], "|") == 0 ){
                    
                } else if( strcmp(argv[next_pipe], ">") == 0 ){
                    char* filename = argv[next_pipe + 1];
                    debug("filename=%s\n", filename);
                    // open write only, create if not exists, trucate if eixsts
                    open_fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if(open_fd == -1){
                        perror("open(filename, O_WRONLY | O_CREAT | O_TRUNC, 644) failed");
                        continue;
                    }

                    pipe_fd[WRITE_END] = open_fd;

                    argv[next_pipe] = 0;

                } else if( strcmp(argv[next_pipe], "<") == 0 ){
                    char* filename = argv[next_pipe + 1];
                    debug("filename=%s\n", filename);
                    // open read only
                    open_fd = open(filename, O_RDONLY);
                    if(open_fd == -1){
                        perror("open(filename, O_RDONLY) failed");
                        continue;
                    }

                    pipe_fd[READ_END] = open_fd;

                    argv[next_pipe] = 0;
                }

                Fork_Cmd(pipe_fd, argc, argv, envp, prog);
                
            }
        } while( (next_pipe = getNextPipe(argc, argv, next_pipe + 1)) != -1 && next_pipe < argc);

        free(cmd);
    }

    //Don't forget to free allocated memory, and close file descriptors.
    free(cmd);
    free(promptBuf);
    //WE WILL CHECK VALGRIND!

    return EXIT_SUCCESS;
}

int Fork_Builtin(int pipe_fd[2], int argc, char** argv, int prog){
    int childPid, childStatus;

    // execution
    if ( (childPid = fork()) == 0){ // if child process
        debug("Child process: %s is built in\n", argv[prog]);

        dup2(pipe_fd[READ_END], STDIN_FILENO);
        dup2(pipe_fd[WRITE_END], STDOUT_FILENO);

        // execute the built in program
        last_exe.val = exeBuiltIn(argc, (argv + prog));

        // deal with file descriptors
        if(pipe_fd[READ_END] != -1){
            dup2(pipe_fd[READ_END], STDIN_FILENO);
        }

        if(pipe_fd[WRITE_END] != -1){
            dup2(pipe_fd[WRITE_END], STDOUT_FILENO);    
        }

        close(pipe_fd[READ_END]);
        close(pipe_fd[WRITE_END]);

        // exit the child process
        exit(last_exe.val);
    }
    else {
        // wait for child to finish
        pid_t wpid = wait(&childStatus);

        // print out the status code
        if(WIFEXITED(childStatus)){
            last_exe.val = WEXITSTATUS(childStatus);
            debug("Child %d terminated with exit code %d\n", wpid, last_exe.val);
        } else{
            last_exe.val = WEXITSTATUS(childStatus);
            debug("Child %d terminated abnormally: %d\n", wpid, last_exe.val);
        }        

        return last_exe.val;
    }
}

int Fork_Cmd(int pipe_fd[2], int argc, char** argv, char* envp[], int prog){
    int childPid, childStatus;

    // check if it is a background program
    if(isBgProc(argv[prog])){
        //record in list of background jobs
        return last_exe.val;

    } else if ( (childPid = fork()) == 0 ){
        debug("Child process: %s is program\n", argv[prog]);

        // set fds
        dup2(pipe_fd[READ_END], STDIN_FILENO);
        dup2(pipe_fd[WRITE_END], STDOUT_FILENO);

        // execute passed in program
        last_exe.val = exeCmd(argc, (argv + prog), envp);

        // revoke fds
        if(pipe_fd[READ_END] != -1){
            dup2(pipe_fd[READ_END], STDIN_FILENO);
        }

        if(pipe_fd[WRITE_END] != -1){
            dup2(pipe_fd[WRITE_END], STDOUT_FILENO);    
        }

        close(pipe_fd[READ_END]);
        close(pipe_fd[WRITE_END]);

        // exit the child process
        exit(last_exe.val);

    } else {
        // wait for child to finish
        pid_t wpid = wait(&childStatus);

        // print out the status code
        if(WIFEXITED(childStatus)){
            last_exe.val = WEXITSTATUS(childStatus);

int Fork_Builtin(int pipe_fd[2], int argc, char** argv, int prog){
    int childPid, childStatus;

    // execution
    if ( (childPid = fork()) == 0){ // if child process
        debug("Child process: %s is built in\n", argv[prog]);

        dup2(pipe_fd[READ_END], STDIN_FILENO);
        dup2(pipe_fd[WRITE_END], STDOUT_FILENO);

        // execute the built in program
        last_exe.val = exeBuiltIn(argc, (argv + prog));

        // deal with file descriptors
        if(pipe_fd[READ_END] != -1){
            dup2(pipe_fd[READ_END], STDIN_FILENO);
        }

        if(pipe_fd[WRITE_END] != -1){
            dup2(pipe_fd[WRITE_END], STDOUT_FILENO);    
        }

        close(pipe_fd[READ_END]);
        close(pipe_fd[WRITE_END]);

        // exit the child process
        exit(last_exe.val);
    }
    else {
        // wait for child to finish
        pid_t wpid = wait(&childStatus);

        // print out the status code
        if(WIFEXITED(childStatus)){
            last_exe.val = WEXITSTATUS(childStatus);
            debug("Child %d terminated with exit code %d\n", wpid, last_exe.val);
        } else{
            last_exe.val = WEXITSTATUS(childStatus);
            debug("Child %d terminated abnormally: %d\n", wpid, last_exe.val);
        }        

        return last_exe.val;
    }
}

int Fork_Cmd(int pipe_fd[2], int argc, char** argv, char* envp[], int prog){
    int childPid, childStatus;

    // check if it is a background program
    if(isBgProc(argv[prog])){
        //record in list of background jobs
        return last_exe.val;

    } else if ( (childPid = fork()) == 0 ){
        debug("Child process: %s is program\n", argv[prog]);

        // set fds
        dup2(pipe_fd[READ_END], STDIN_FILENO);
        dup2(pipe_fd[WRITE_END], STDOUT_FILENO);

        // execute passed in program
        last_exe.val = exeCmd(argc, (argv + prog), envp);

        // revoke fds
        if(pipe_fd[READ_END] != -1){
            dup2(pipe_fd[READ_END], STDIN_FILENO);
        }

        if(pipe_fd[WRITE_END] != -1){
            dup2(pipe_fd[WRITE_END], STDOUT_FILENO);    
        }

        close(pipe_fd[READ_END]);
        close(pipe_fd[WRITE_END]);

        // exit the child process
        exit(last_exe.val);

    } else {
        // wait for child to finish
        pid_t wpid = wait(&childStatus);

        // print out the status code
        if(WIFEXITED(childStatus)){
            last_exe.val = WEXITSTATUS(childStatus);
            debug("Child %d terminated with exit code %d\n", wpid, last_exe.val);
        } else{
            last_exe.val = WEXITSTATUS(childStatus);
            debug("Child %d terminated abnormally: %d\n", wpid, last_exe.val);
        }

        return last_exe.val;
    }
}
            debug("Child %d terminated with exit code %d\n", wpid, last_exe.val);
        } else{
            last_exe.val = WEXITSTATUS(childStatus);
            debug("Child %d terminated abnormally: %d\n", wpid, last_exe.val);
        }

        return last_exe.val;
    }
}