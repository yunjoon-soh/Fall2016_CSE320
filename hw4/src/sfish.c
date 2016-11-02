#include "sfish.h"

int main(int argc, char** argv, char *envp[]) {
    int childPid, childStatus;

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
        if(argv == NULL){
            debug("Nothing to parse, continue\n");
            continue;
        }

        for(int i = 0; i < argc; i++){
            debug("argc=%d, argv[%d]=%s\n", argc, i, argv[i]);
        }
        
        do {
            debug("do-while loop next_pipe=%d\n", next_pipe);
            char* prog;
            int open_fd = -1;

            if(next_pipe == 0)
                prog = argv[next_pipe];
            else
                prog = argv[next_pipe + 1];

            if( isBuiltin(prog) != SF_FALSE ){ // if not SF_FALSE, then it is true

                if( strcmp(prog, "cd") == 0 ||
                    strcmp(prog, "chpmt") == 0 || 
                    strcmp(prog, "chclr") == 0 ){ // if cd do not fork

                    // execute the built in program
                    debug("No fork next_pipe=%d\n", next_pipe);
                    last_exe.val = exeBuiltIn(argc, (argv + next_pipe));

                } else{ // fork is necessary
                    
                    // 
                    if (next_pipe == 0){
                        // find the next pipe character's location
                        next_pipe = getNextPipe(argc, argv, next_pipe);

                    } else if( strcmp(argv[next_pipe], "|") == 0 ){
                        
                    } else if( strcmp(argv[next_pipe], ">") == 0 ){
                        char* filename = argv[next_pipe + 1];
                        // open write only, create if not exists, trucate if eixsts
                        open_fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                        if(open_fd == -1){
                            perror("open(filename, O_WRONLY | O_CREAT | O_TRUNC, 644) failed");
                            return -1;
                        }

                        dup2(open_fd, 1);

                        argv[next_pipe] = 0;

                    } else if( strcmp(argv[next_pipe], "<") == 0 ){
                        char* filename = argv[next_pipe + 1];

                        // open read only
                        open_fd = open(filename, O_RDONLY);
                        if(open_fd == -1){
                            perror("open(filename, O_RDONLY) failed");
                            return -1;
                        }

                        dup2(0, open_fd);

                        argv[next_pipe] = 0;
                    }

                    // execution
                    if ( (childPid = fork()) == 0){ // if child process
                        debug("Child process: %s is built in\n", prog);

                        // execute the built in program
                        last_exe.val = exeBuiltIn(argc, (argv + next_pipe + 1)); // TOOD

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

                        if(open_fd != -1){
                            close(open_fd);
                        }
                    }
                }
            }
            else{ // not builtin program

                // check if it is a background program
                if(isBgProc(prog)){
                    //record in list of background jobs

                } else if ( (childPid = fork()) == 0 ){

                    // 
                    last_exe.val = exeCmd(argc, &prog, envp);

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
                }
            }

            // inspect the arguments starting from the next command
            next_pipe++;

        } while( (next_pipe = getNextPipe(argc, argv, next_pipe)) != -1 && next_pipe < argc);

        free(cmd);
    }

    //Don't forget to free allocated memory, and close file descriptors.
    free(cmd);
    free(promptBuf);
    //WE WILL CHECK VALGRIND!

    return EXIT_SUCCESS;
}
