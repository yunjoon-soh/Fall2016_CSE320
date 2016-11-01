#include "sfish.h"

int main(int argc, char** argv, char *envp[]) {
    int childPid, childStatus;
    char *strstr_ret;
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
        char **argv = parseNCmd(cmd, buf, argc);
        if(argv == NULL){
            debug("Nothing to parse, continue\n");
            continue;
        }
        debug("Parse result: argc=%d, argv=%s\n", argc, argv[0]);
        for(int i = 0; i <= argc; i++){
            debug("argv[%d]=%s\n", i, argv[i]);
        }

        if( (last_exe.val = exeBuiltIn(argc, argv)) == SF_SUCCESS ){ 
            // if successful, then it is true
            debug("%s is built in\n", cmd);
        }
        else{
            if ( (childPid = fork()) == 0 ){
                pipelineCheck(argc, argv);
                exeCmd(argc, argv, envp);
            }
            else {
                pid_t wpid = wait(&childStatus);
                if(WIFEXITED(childStatus)){
                    last_exe.val = WEXITSTATUS(childStatus);
                    debug("Child %d terminated with exit code %d\n", wpid, last_exe.val);
                } else{
                    last_exe.val = WEXITSTATUS(childStatus);
                    debug("Child %d terminated abnormally: %d\n", wpid, lasst_exe.val);
                }
            }
        //  } else {
        //      if (isBgProc(cmd)){
        //          //record in list of background jobs
        //      } else {
        //          waitpid (childPid, &childStatus, WNOHANG);
        //      }
        //  }
        }
        free(cmd);
    }

    //Don't forget to free allocated memory, and close file descriptors.
    free(cmd);
    free(promptBuf);
    //WE WILL CHECK VALGRIND!

    return EXIT_SUCCESS;
}
