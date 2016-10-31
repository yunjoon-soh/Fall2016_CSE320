#include "sfish.h"

int main(int argc, char** argv) {
    // int childPid, childStatus;
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

        debug("Trim leading and ending white spaces\n");


        //All your debug print statments should be surrounded by this #ifdef
        //block. Use the debug target in the makefile to run with these enabled.
        debug("Length of command entered: %s, len=%ld @ %p\n", cmd, strlen(cmd), cmd);
        //You WILL lose points if your shell prints out garbage values.

        int argc = countElements(cmd);
        char *buf[argc];
        char **argv = parseNCmd(cmd, buf, argc);
        debug("Parse result: argc=%d, argv=%s\n", argc, argv[0]);
        for(int i = 0; i < argc; i++){
            debug("argv[%d]=%s\n", i, argv[i]);
        }
        
        if( (last_exe.val = exeBuiltIn(argc, argv)) == SF_SUCCESS ){ // if successful, then it is true
            debug("%s is built in\n", cmd);
        }
        // else{
        //  childPid = fork();
        //  if (childPid == 0){
        //      exeCmd(cmds);
        //  } else {
        //      if (isBgProc(cmd)){
        //          //record in list of background jobs
        //      } else {
        //          waitpid (childPid, &childStatus, WNOHANG);
        //      }
        //  }
        // }
        free(cmd);
    }

    //Don't forget to free allocated memory, and close file descriptors.
    free(cmd);
    free(promptBuf);
    //WE WILL CHECK VALGRIND!

    return EXIT_SUCCESS;
}
