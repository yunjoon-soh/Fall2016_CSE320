#include <criterion/criterion.h>

#include <stdio.h>
#include "sfbuiltin.h"
#include "sfish_helper.h"

#define HOME "/home/dan"
void checkCurHist(char* cmp);
void checkCurDir(char* cmp);

void checkCurHist(char* cmp) {
	char hist[PATH_MAX];
	strcpy(hist, cd_history);
	cr_assert( strcmp(hist, cmp) == 0, "checkCurHist (exp:%s real:%s)\n", cmp, hist);
}

void checkCurDir(char* cmp) {
	char cdir[PATH_MAX];
	getcwd(cdir, PATH_MAX);
	cr_assert( strcmp(cdir, cmp) == 0, "checkCurDir (exp:%s real:%s)\n", cmp, cdir);
}

void setup(){
	preprocess();

	char cdir[PATH_MAX];
	getcwd(cdir, PATH_MAX);

	builtin_cd(NULL); // same as typing "cd", i.e., home dir

	checkCurDir(HOME);
	checkCurHist(cdir);
}

/**
* TEST BEGINS HERE
*/

// // sfish_helper
// Test(sfish_helper_parseCmd, cmd_is_empty, .init=setup){
// 	char cmd[] = "";
// 	int count = countSpaces(cmd) + 1;
// 	cr_assert(count == 1);

// 	char* buf[count];
// 	parseNCmd(cmd, buf, count);

// 	cr_assert( strcmp(buf[0], "") == 0, "expected buf[%i]=%s actual=%s", 0, "", buf[0]);
// }

// Test(sfish_helper_parseCmd, cmd_has_one_param, .init=setup){
// 	char cmd[] = "cd";
// 	int count = countSpaces(cmd) + 1;
// 	cr_assert(count == 1);

// 	char* buf[count];
// 	parseNCmd(cmd, buf, count);

// 	cr_assert( strcmp(buf[0], "cd") == 0, "expected buf[%i]=%s actual=%s", 0, "cd", buf[0]);
// }

// Test(sfish_helper_parseCmd, cmd_has_two_param, .init=setup){
// 	char cmd[] = "cd ./bin/sfish";
// 	int count = countSpaces(cmd) + 1;
// 	cr_assert(count == 2);

// 	char* buf[count];
// 	parseNCmd(cmd, buf, count);

// 	cr_assert( strcmp(buf[0], "cd") == 0, "expected buf[%i]=%s actual=%s", 0, "cd", buf[0]);
// 	cr_assert( strcmp(buf[1], "./bin/sfish") == 0, "expected buf[%i]=%s actual=%s", 1, "./bin/sfish", buf[1]);
// }


// // builtin_cd

// Test(builtin_cd, cdDot, .init=setup) {
// 	char prev_cdir[PATH_MAX], next_cdir[PATH_MAX];
// 	getcwd(prev_cdir, PATH_MAX);

// 	builtin_cd(".");

// 	getcwd(&next_cdir[0], PATH_MAX);

// 	cr_assert( strcmp(&prev_cdir[0], &next_cdir[0]) == 0 );
// }

// Test(builtin_cd, cdDotDot, .init=setup) {
// 	char prev_cdir[PATH_MAX], next_cdir[PATH_MAX], *last_backslash;
// 	getcwd(prev_cdir, PATH_MAX);

// 	builtin_cd("..");

// 	getcwd(&next_cdir[0], PATH_MAX);

// 	last_backslash = strrchr(&prev_cdir[0], '/'); 
// 	if (last_backslash){
// 		*last_backslash = '\0';
// 	}

// 	cr_assert( strcmp(&prev_cdir[0], &next_cdir[0]) == 0,
// 		"Previous was %s, Now it is %s\n", prev_cdir, next_cdir);
// }

// Test(builtin_cd, cdDash, .init=setup) {
// 	char prev_cdir[PATH_MAX], next_cdir[PATH_MAX], prev_hist[PATH_MAX], next_hist[PATH_MAX];
	
// 	getcwd(prev_cdir, PATH_MAX);
// 	strcpy(prev_hist, cd_history);

// 	builtin_cd("-");

// 	getcwd(next_cdir, PATH_MAX);
// 	strcpy(next_hist, cd_history);

// 	cr_assert( strcmp(prev_hist, next_cdir) == 0 );
// 	cr_assert( strcmp(next_hist, prev_cdir) == 0 );
// }

// Test(builtin_cd, cdFolder_Not_Exist, .init=setup) {
// 	int ret;
// 	char prev_cdir[PATH_MAX], next_cdir[PATH_MAX], prev_hist[PATH_MAX], next_hist[PATH_MAX];
	
// 	getcwd(prev_cdir, PATH_MAX);
// 	strcpy(prev_hist, cd_history);

// 	ret = builtin_cd("./asdklfjaklwejflkqwkelrjiosjvklds");
// 	cr_assert(ret == SF_FAIL, "Expected to fail, but return was %d\n", ret);

// 	getcwd(next_cdir, PATH_MAX);
// 	strcpy(next_hist, cd_history);

// 	cr_assert( strcmp(prev_hist, next_hist) == 0 );
// 	cr_assert( strcmp(prev_cdir, next_cdir) == 0 );
// }

// Test(builtin_cd, cdFolder_Exist, .init=setup) {
// 	int ret;
// 	char prev_cdir[PATH_MAX], next_cdir[PATH_MAX], prev_hist[PATH_MAX], next_hist[PATH_MAX];
	
// 	getcwd(prev_cdir, PATH_MAX);
// 	strcpy(prev_hist, cd_history);

// 	printf("%s\n", prev_cdir);
// 	ret = builtin_cd("./ysoh/hw4/test");
// 	cr_assert(ret == SF_SUCCESS, "Expected to sucess, but return was %d\n", ret);

// 	getcwd(next_cdir, PATH_MAX);
// 	strcpy(next_hist, cd_history);

// 	cr_assert( strcmp(next_hist, prev_cdir) == 0 );
// 	cr_assert( strcmp(next_cdir, "/home/dan/ysoh/hw4/test") == 0);
// }

// Test(builtin_cd, cdNULL, .init=setup) {
// 	int ret;
// 	char prev_cdir[PATH_MAX], next_cdir[PATH_MAX], prev_hist[PATH_MAX], next_hist[PATH_MAX];
// 	char *existing_folder = "/home/dan/ysoh/hw4/test";
	
// 	// change current dir to variable existing_folder
// 	getcwd(prev_cdir, PATH_MAX);
// 	strcpy(prev_hist, cd_history);

// 	ret = builtin_cd(existing_folder);
// 	cr_assert(ret == SF_SUCCESS, "Expected to sucess, but return was %d\n", ret);

// 	getcwd(next_cdir, PATH_MAX);
// 	strcpy(next_hist, cd_history);

// 	cr_assert( strcmp(next_hist, prev_cdir) == 0 );
// 	cr_assert( strcmp(next_cdir, "/home/dan/ysoh/hw4/test") == 0);

// 	// "cd", i.e., change to HOME dir
// 	getcwd(prev_cdir, PATH_MAX);
// 	strcpy(prev_hist, cd_history);

// 	ret = builtin_cd(NULL);
// 	cr_assert(ret == SF_SUCCESS, "Expected to sucess, but return was %d\n", ret);

// 	getcwd(next_cdir, PATH_MAX);
// 	strcpy(next_hist, cd_history);

// 	checkCurDir(HOME);
// 	checkCurHist(existing_folder);
// }

// builtin_chpmt
Test(builtin_chpmt, chpmt_valid, .init=setup) {
	int argc;
	char *buf[3], *pmt, **argv;
	char cmd0[] = "chpmt user 0";
	char cmd1[] = "chpmt user 1";
	char cmd2[] = "chpmt machine 0";
	char cmd3[] = "chpmt machine 1";

	int len = 5 + LOGIN_NAME_MAX + HOST_NAME_MAX + PATH_MAX + 20; // 20 for extra, 5 for "sfish"
    char *promptBuf = (char*) malloc(len); 

    // assert default
	pmt = getsnPrompt(promptBuf, len);
	cr_assert( strcmp(pmt, "sfish-dan@dan-ubuntu:[~]> ") == 0);

	// turn off user
	argc = 3;
	argv = parseNCmd(cmd0, buf, argc);

	builtin_chpmt(argc, argv);

	pmt = getsnPrompt(promptBuf, len);
	cr_assert( strcmp(pmt, "sfish-dan-ubuntu:[~]> ") == 0);

	// turn off machine
	argc = 3;
	argv = parseNCmd(cmd2, buf, argc);

	builtin_chpmt(argc, argv);

	pmt = getsnPrompt(promptBuf, len);
	cr_assert( strcmp(pmt, "sfish:[~]> ") == 0);

	// turn on user
	argc = 3;
	argv = parseNCmd(cmd1, buf, argc);

	builtin_chpmt(argc, argv);

	pmt = getsnPrompt(promptBuf, len);
	cr_assert( strcmp(pmt, "sfish-dan:[~]> ") == 0);

	// turn on machine
	argc = 3;
	argv = parseNCmd(cmd3, buf, argc);

	builtin_chpmt(argc, argv);

	pmt = getsnPrompt(promptBuf, len);
	cr_assert( strcmp(pmt, "sfish-dan@dan-ubuntu:[~]> ") == 0);

	free(promptBuf);
}