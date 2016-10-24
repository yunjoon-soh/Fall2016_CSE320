#include <criterion/criterion.h>

#include <stdio.h>
#include "sfbuiltin.h"

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
	builtin_cd(NULL);

	checkCurDir(HOME);
	checkCurHist(cdir);
}

/**
* TEST BEGINS HERE
*/
Test(builtin_cd, cdDot, .init=setup) {
	char prev_cdir[PATH_MAX], next_cdir[PATH_MAX];
	getcwd(prev_cdir, PATH_MAX);

	builtin_cd(".");

	getcwd(&next_cdir[0], PATH_MAX);

	cr_assert( strcmp(&prev_cdir[0], &next_cdir[0]) == 0 );
}

Test(builtin_cd, cdDotDot, .init=setup) {
	char prev_cdir[PATH_MAX], next_cdir[PATH_MAX], *last_backslash;
	getcwd(prev_cdir, PATH_MAX);

	builtin_cd("..");

	getcwd(&next_cdir[0], PATH_MAX);

	last_backslash = strrchr(&prev_cdir[0], '/'); 
	if (last_backslash){
		*last_backslash = '\0';
	}

	cr_assert( strcmp(&prev_cdir[0], &next_cdir[0]) == 0,
		"Previous was %s, Now it is %s\n", prev_cdir, next_cdir);
}

Test(builtin_cd, cdDash, .init=setup) {
	char prev_cdir[PATH_MAX], next_cdir[PATH_MAX], prev_hist[PATH_MAX], next_hist[PATH_MAX];
	
	getcwd(prev_cdir, PATH_MAX);
	strcpy(prev_hist, cd_history);

	builtin_cd("-");

	getcwd(next_cdir, PATH_MAX);
	strcpy(next_hist, cd_history);

	info("%s %s vs %s %s\n", prev_hist, prev_cdir, next_hist, next_cdir);
	cr_assert( strcmp(prev_hist, next_cdir) == 0 );
	cr_assert( strcmp(next_hist, prev_cdir) == 0 );
}

// Test(builtin_cd, cdNULL, .init=setup) {
// 	builtin_cd(NULL);
// }