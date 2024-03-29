#include "sfish.h"

volatile sig_atomic_t bg;
int cmd_cnt = 0;

int main(int argc, char** argv, char *envp[]) {
	//DO NOT MODIFY THIS. If you do you will get a ZERO.
	rl_catch_signals = 0;
	//This is disable readline's default signal handlers, since you are going
	//to install your own.
	
	rl_command_func_t ctrl_b, ctrl_g, ctrl_h, ctrl_p;
	rl_bind_keyseq ("\\C-b", ctrl_b);
	rl_bind_keyseq ("\\C-g", ctrl_g);
	rl_bind_keyseq ("\\C-h", ctrl_h);
	rl_bind_keyseq ("\\C-p", ctrl_p);

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

		struct exe_info ei_array[argc - 1]; // at most argc -1 number of exe
		for(int i = 0; i < argc - 1; i++){
			ei_array[i].prog_index = -1;
			ei_array[i].pipe_fd[READ_END] = STDIN_FILENO;
			ei_array[i].pipe_fd[WRITE_END] = STDOUT_FILENO;
			ei_array[i].stderr_pipe = STDERR_FILENO;
		}
		int ei_cnt = 0;

		if(argv == NULL){
			debug("Nothing to parse, continue\n");
			continue;
		}

		for(int i = 0; i < argc; i++){
			debug("argc=%d, argv[%d]=%s\n", argc, i, argv[i]);
		}
		

		int open_fd = -1, prog = -1;
		do {
			debug("do-while loop next_pipe=%d\n", next_pipe);
			if(next_pipe == 0){
				ei_array[ei_cnt].prog_index = 0;
				prog = 0;
			}

			if( strcmp(argv[next_pipe], "|") == 0 ){
				if(next_pipe == 0){
					fprintf(stderr, "Error: invalid command: started with |\n");
					break;
				}

				// create pipe_fd
				pipe(pipe_fd);

				ei_array[ei_cnt].pipe_fd[WRITE_END] = pipe_fd[WRITE_END];
				ei_array[ei_cnt].prog_index = prog;

				ei_array[ei_cnt + 1].pipe_fd[READ_END] = pipe_fd[READ_END];
				ei_array[ei_cnt + 1].prog_index = next_pipe + 1;

				argv[next_pipe] = 0;

				// increase the exe counter
				ei_cnt++;

				// get next exe's location
				prog = next_pipe + 1;

			} else if( strcmp(argv[next_pipe], ">") == 0 ){
				if(next_pipe == 0){
					fprintf(stderr, "Error: invalid command: started with >\n");
					break;
				}

				char* filename = argv[next_pipe + 1];
				debug("filename=%s\n", filename);

				// open write only, create if not exists, trucate if eixsts
				open_fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
				if(open_fd == -1){
					perror("open(filename, O_WRONLY | O_CREAT | O_TRUNC, 644) failed");
					continue;
				}

				char *ptr = argv[next_pipe - 1];
				if( *(ptr) != '2'){
					ei_array[ei_cnt].pipe_fd[WRITE_END] = open_fd;
					ei_array[ei_cnt].stderr_pipe = open_fd;

					argv[next_pipe] = 0;
				} else {
					debug("READ_END set to stderr\n");

					ei_array[ei_cnt].stderr_pipe = open_fd;
				}

			} else if( strcmp(argv[next_pipe], "<") == 0 ){
				if(next_pipe == 0){
					fprintf(stderr, "Error: invalid command: started with <\n");
					break;
				}

				char* filename = argv[next_pipe + 1];
				debug("filename=%s\n", filename);

				// open read only
				open_fd = open(filename, O_RDONLY);
				if(open_fd == -1){
					perror("open(filename, O_RDONLY) failed");
					continue;
				}

				ei_array[ei_cnt].pipe_fd[READ_END] = open_fd;

				argv[next_pipe] = 0;
			}
		} while( (next_pipe = getNextPipe(argc, argv, next_pipe + 1)) != -1 && next_pipe < argc);

		for(int i = 0; i <= ei_cnt; i++){
			int prog = ei_array[i].prog_index;
			// debug("pipe[0]=%d, pipe[1]=%d, prog=%d\n", ei_array[i].pipe_fd[0], ei_array[i].pipe_fd[1], prog);

			if( strcmp(argv[prog], "cd") == 0 ||
				strcmp(argv[prog], "chpmt") == 0 || 
				strcmp(argv[prog], "chclr") == 0 ||
				strcmp(argv[prog], "fg") == 0 || 
				strcmp(argv[prog], "bg") == 0 || 
				strcmp(argv[prog], "kill") == 0 || 
				strcmp(argv[prog], "disown") == 0){ // if cd do not fork

				// execute the built in program
				last_exe.val = exeBuiltIn(argc, (argv + prog));

			} else { // forking is necessary
				Fork(argc, argv, envp, &ei_array[i]);
			}
		}

		free(cmd);
	}

	//Don't forget to free allocated memory, and close file descriptors.
	free(cmd);
	free(promptBuf);
	//WE WILL CHECK VALGRIND!

	return EXIT_SUCCESS;
}

int Fork(int argc, char** argv, char *envp[], struct exe_info* ei){
	if( isBuiltin(argv[ei->prog_index]) ){
		return Fork_Builtin(ei->pipe_fd, ei->stderr_pipe, argc, argv, ei->prog_index);
	} else {
		return Fork_Cmd(ei->pipe_fd, ei->stderr_pipe, argc, argv, envp, ei->prog_index);
	}
}

int Fork_Builtin(int pipe_fd[2], int stderr_no, int argc, char** argv, int prog){
	int childPid, childStatus;

	// execution
	if ( (childPid = fork()) == 0){ // if child process
		debug("Child process: %s(pid=%d) is built in\n", argv[prog], getpid());

		// set signal handlers //TODO: do this?
		// debug("Setting the child's signal handler\n");
		// SetSigHandler();

		// set fds
		SetFd(pipe_fd, stderr_no);

		// execute the built in program
		last_exe.val = exeBuiltIn(argc, (argv + prog));

		CloseFd(pipe_fd, stderr_no);
		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);

		// exit the child process
		exit(last_exe.val);

	} else {
		// wait for child to finish
		pid_t wpid = wait(&childStatus);

		// close fds
		CloseFd(pipe_fd, stderr_no);

		// print out the status code
		HandleExit(wpid, childStatus);

		cmd_cnt++;

		return last_exe.val;
	}
}


int Fork_Cmd(int pipe_fd[2], int stderr_no, int argc, char** argv, char* envp[], int prog){
	int childPid = -1;
	int childStatus = 0;

	// check if it is a background program
	if(isBgProc(argv + prog)){
		//record in list of background jobs
		debug("background process\n");
		
		if( (childPid = fork()) == 0){ // child code
			// set the pgid of the child process
			setpgid(0,0);

			debug("Child process: %s(pid=%d, pgid=%d) is program\n", argv[prog], getpid(), getpgid(getpid()));

			debug("Setting the child's signal handler\n");
			SetSigHandler();

			// set fds
			debug("pipe_fd[READ_END]=%d, pipe_fd[WRITE_END]=%d\n", pipe_fd[READ_END], pipe_fd[WRITE_END]);
			SetFd(pipe_fd, stderr_no);

			// execute passed in program
			last_exe.val = exeCmd(argc, (argv + prog), envp);

			fprintf(stderr, "Returned from exeCmd\n");

			CloseFd(pipe_fd, stderr_no);
			close(STDIN_FILENO);
			close(STDOUT_FILENO);
			close(STDERR_FILENO);

			// exit the child process
			exit(last_exe.val);

		} else { // parent code
			struct job* now = createJob(childPid, RUNNING, (argv + prog));
			now->inJob = 1; // this is background job, so should be added to the Job list

			addJob(now);

			cmd_cnt++;

			// reaping handled by sigchld
		}

		return last_exe.val;
	} else if ( (childPid = fork()) == 0 ){
		// set the pgid of the child process
		setpgid(0,0);

		debug("Child process: %s(pid=%d, pgid=%d) is program\n", argv[prog], getpid(), getpgid(getpid()));

		debug("Setting the child's signal handler\n");
		SetSigHandler();

		// set fds
		debug("pipe_fd[READ_END]=%d, pipe_fd[WRITE_END]=%d\n", pipe_fd[READ_END], pipe_fd[WRITE_END]);
		SetFd(pipe_fd, stderr_no);

		// execute passed in program
		last_exe.val = exeCmd(argc, (argv + prog), envp);

		fprintf(stderr, "Returned from exeCmd\n");
		CloseFd(pipe_fd, stderr_no);
		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);

		// exit the child process
		exit(last_exe.val);

	} else {
		struct job* now = createJob(childPid, RUNNING, (argv + prog));

		addJob(now);

		cmd_cnt++;

		fg = now;
		debug("fg=%d\n", fg->pid);

		// wait for child to finish
		pid_t wpid = waitpid(childPid, &childStatus, WUNTRACED);

		if(WIFEXITED(childStatus)){
			removeJob(childPid, JOB_FALSE);

			// close fds
			CloseFd(pipe_fd, stderr_no);

			// print out the status code
			HandleExit(wpid, childStatus);
		}

		return last_exe.val;
	}
}