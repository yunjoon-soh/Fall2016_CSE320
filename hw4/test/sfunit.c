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
Test(sfish_helper_countElements, counting_test){
	int count;

	count = countElements("");
	cr_assert(count == 0, "Empty space is not counted as 0, instead %d", count);

	count = countElements(" ");
	cr_assert(count == 0, "Empty space is not counted as 0");

	count = countElements("  ");
	cr_assert(count == 0, "Empty space is not counted as 0");

	count = countElements("abc");
	cr_assert(count == 1);

	count = countElements(" abc");
	cr_assert(count == 1);

	count = countElements("abc ");
	cr_assert(count == 1);

	count = countElements(" abc ");
	cr_assert(count == 1);

	count = countElements("abc def");
	cr_assert(count == 2);

	count = countElements(" abc def");
	cr_assert(count == 2);

	count = countElements(" abc def ");
	cr_assert(count == 2);

	count = countElements(" abc      def ");
	cr_assert(count == 2);

	count = countElements(" abc  f    def ");
	cr_assert(count == 3);
}

Test(sfish_helper_countElements, counting_with_pipeline_test_gt){
	int count;

	count = countElements(">");
	cr_assert(count == 1);

	count = countElements("abc >");
	cr_assert(count == 2);

	count = countElements("abc > ");
	cr_assert(count == 2);

	count = countElements(" abc > ");
	cr_assert(count == 2);

	count = countElements("   abc   >   ");
	cr_assert(count == 2);

	count = countElements("   a   bc   >   ");
	cr_assert(count == 3);

	count = countElements("abc > def");
	cr_assert(count == 3);

	count = countElements("                 abc    >   def   ");
	cr_assert(count == 3);

	count = countElements(">abc ");
	cr_assert(count == 2);

	count = countElements(" a>bc ");
	cr_assert(count == 3);

	count = countElements("abc def>");
	cr_assert(count == 3);

	count = countElements(" abc d ef>");
	cr_assert(count == 4);

	count = countElements(" >a b> >c d> e>>f ");
	cr_assert(count == 12);

	count = countElements(" a>b>c      d>e>f ");
	cr_assert(count == 10);

	count = countElements(" a>bc>  f>    d>e>f ");
	cr_assert(count == 11);
}

Test(sfish_helper_countElements, counting_with_pipeline_test_lt){
	int count;

	count = countElements("<");
	cr_assert(count == 1);

	count = countElements("abc <");
	cr_assert(count == 2);

	count = countElements("abc < ");
	cr_assert(count == 2);

	count = countElements(" abc < ");
	cr_assert(count == 2);

	count = countElements("   abc   <   ");
	cr_assert(count == 2);

	count = countElements("   a   bc   <   ");
	cr_assert(count == 3);

	count = countElements("abc < def");
	cr_assert(count == 3);

	count = countElements("                 abc    <   def   ");
	cr_assert(count == 3);

	count = countElements("<abc ");
	cr_assert(count == 2);

	count = countElements(" a<bc ");
	cr_assert(count == 3);

	count = countElements("abc def<");
	cr_assert(count == 3);

	count = countElements(" abc d ef<");
	cr_assert(count == 4);

	count = countElements(" <a b< <c d< e<<f ");
	cr_assert(count == 12);

	count = countElements(" a<b<c      d<e<f ");
	cr_assert(count == 10);

	count = countElements(" a<bc<  f<    d<e<f ");
	cr_assert(count == 11);
}

Test(sfish_helper_countElements, counting_with_pipeline_test_pipe){
	int count;

	count = countElements("|");
	cr_assert(count == 1);

	count = countElements("abc |");
	cr_assert(count == 2);

	count = countElements("abc | ");
	cr_assert(count == 2);

	count = countElements(" abc | ");
	cr_assert(count == 2);

	count = countElements("   abc   |   ");
	cr_assert(count == 2);

	count = countElements("   a   bc   |   ");
	cr_assert(count == 3);

	count = countElements("abc | def");
	cr_assert(count == 3);

	count = countElements("                 abc    |   def   ");
	cr_assert(count == 3);

	count = countElements("|abc ");
	cr_assert(count == 2);

	count = countElements(" a|bc ");
	cr_assert(count == 3);

	count = countElements("abc def|");
	cr_assert(count == 3);

	count = countElements(" abc d ef|");
	cr_assert(count == 4);

	count = countElements(" |a b| |c d| e||f ");
	cr_assert(count == 12);

	count = countElements(" a|b|c      d|e|f ");
	cr_assert(count == 10);

	count = countElements(" a|bc|  f|    d|e|f ");
	cr_assert(count == 11);
}

// sfish_helper
Test(sfish_helper_parseCmd, cmd_is_empty, .init=setup){
	char cmd[] = "";
	int count = countElements(cmd) + 1;
	cr_assert(count == 0 + 1);

	char* buf[count];
	char** ret = parseNCmd(cmd, buf, count);

	cr_assert(ret == NULL);
}

// one_param
Test(sfish_helper_parseCmd, cmd_has_one_param, .init=setup){
	char cmd[] = "cd";
	int count = countElements(cmd) + 1;
	cr_assert(count == 1 + 1);

	char* buf[count];
	char** ret = parseNCmd(cmd, buf, count);
	cr_assert(ret != NULL);
	cr_assert( strcmp(buf[0], "cd") == 0, "expected buf[%i]=%s actual=%s", 0, "cd", buf[0]);
	cr_assert(buf[1] == NULL);
}

Test(sfish_helper_parseCmd, cmd_has_one_param2, .init=setup){
	char cmd[] = "  cd";
	int count = countElements(cmd) + 1;
	cr_assert(count == 1 + 1);

	char* buf[count];
	char** ret = parseNCmd(cmd, buf, count);
	cr_assert(ret != NULL);
	cr_assert( strcmp(buf[0], "cd") == 0, "expected buf[%i]=%s actual=%s", 0, "cd", buf[0]);
	cr_assert(buf[1] == NULL);
}

Test(sfish_helper_parseCmd, cmd_has_one_param3, .init=setup){
	char cmd[] = "cd  ";
	int count = countElements(cmd) + 1;
	cr_assert(count == 1 + 1);

	char* buf[count];
	char** ret = parseNCmd(cmd, buf, count);
	cr_assert(ret != NULL);
	cr_assert( strcmp(buf[0], "cd") == 0, "expected buf[%i]=\"%s\" actual=\"%s\"", 0, "cd", buf[0]);
	cr_assert(buf[1] == NULL);
}

Test(sfish_helper_parseCmd, cmd_has_one_param4, .init=setup){
	char cmd[] = "  cd  ";
	int count = countElements(cmd) + 1;
	cr_assert(count == 1 + 1);

	char* buf[count];
	char** ret = parseNCmd(cmd, buf, count);
	cr_assert(ret != NULL);
	cr_assert( strcmp(buf[0], "cd") == 0, "expected buf[%i]=%s actual=%s", 0, "cd", buf[0]);
	cr_assert(buf[1] == NULL);
}

// two_param
Test(sfish_helper_parseCmd, cmd_has_two_param, .init=setup){
	char cmd[] = "cd ./bin/sfish";
	int count = countElements(cmd) + 1;
	cr_assert(count == 2 + 1);

	char* buf[count];
	char** ret = parseNCmd(cmd, buf, count);
	cr_assert(ret != NULL);

	cr_assert( strcmp(buf[0], "cd") == 0, "expected buf[%i]=%s actual=%s", 0, "cd", buf[0]);
	cr_assert( strcmp(buf[1], "./bin/sfish") == 0, "expected buf[%i]=%s actual=%s", 1, "./bin/sfish", buf[1]);
	cr_assert(buf[2] == NULL);
}

Test(sfish_helper_parseCmd, cmd_has_two_param2, .init=setup){
	char cmd[] = "  cd ./bin/sfish";
	int count = countElements(cmd) + 1;
	cr_assert(count == 2 + 1);

	char* buf[count];
	char** ret = parseNCmd(cmd, buf, count);
	cr_assert(ret != NULL);

	cr_assert( strcmp(buf[0], "cd") == 0, "expected buf[%i]=%s actual=%s", 0, "cd", buf[0]);
	cr_assert( strcmp(buf[1], "./bin/sfish") == 0, "expected buf[%i]=%s actual=%s", 1, "./bin/sfish", buf[1]);
	cr_assert(buf[2] == NULL);
}

Test(sfish_helper_parseCmd, cmd_has_two_param3, .init=setup){
	char cmd[] = "cd ./bin/sfish  ";
	int count = countElements(cmd) + 1;
	cr_assert(count == 2 + 1);

	char* buf[count];
	char** ret = parseNCmd(cmd, buf, count);
	cr_assert(ret != NULL);

	cr_assert( strcmp(buf[0], "cd") == 0, "expected buf[%i]=%s actual=%s", 0, "cd", buf[0]);
	cr_assert( strcmp(buf[1], "./bin/sfish") == 0, "expected buf[%i]=%s actual=%s", 1, "./bin/sfish", buf[1]);
	cr_assert(buf[2] == NULL);
}

Test(sfish_helper_parseCmd, cmd_has_two_param4, .init=setup){
	char cmd[] = "   cd     ./bin/sfish     ";
	int count = countElements(cmd) + 1;
	cr_assert(count == 2 + 1);

	char* buf[count];
	char** ret = parseNCmd(cmd, buf, count);
	cr_assert(ret != NULL);

	cr_assert( strcmp(buf[0], "cd") == 0, "expected buf[%i]=%s actual=%s", 0, "cd", buf[0]);
	cr_assert( strcmp(buf[1], "./bin/sfish") == 0, "expected buf[%i]=%s actual=%s", 1, "./bin/sfish", buf[1]);
	cr_assert(buf[2] == NULL);
}

// three_param
Test(sfish_helper_parseCmd, cmd_has_three_param, .init=setup){
	char cmd[] = "cd ./bin/sfish outfile";
	int count = countElements(cmd) + 1;
	cr_assert(count == 3 + 1);

	char* buf[count];
	char** ret = parseNCmd(cmd, buf, count);
	cr_assert(ret != NULL);

	cr_assert( strcmp(buf[0], "cd") == 0, "expected buf[%i]=%s actual=%s", 0, "cd", buf[0]);
	cr_assert( strcmp(buf[1], "./bin/sfish") == 0, "expected buf[%i]=%s actual=%s", 1, "./bin/sfish", buf[1]);
	cr_assert( strcmp(buf[2], "outfile") == 0, "expected buf[%i]=%s actual=%s", 2, "outfile", buf[2]);
	cr_assert(buf[3] == NULL);
}

// pipeline(gt)
Test(sfish_helper_parseCmd, cmd_has_pipeline_gt, .init=setup){
	char cmd[] = "cd ./bin/sfish > outfile";
	int count = countElements(cmd) + 1;
	cr_assert(count == 4 + 1);

	char* buf[count];
	char** ret = parseNCmd(cmd, buf, count);
	cr_assert(ret != NULL);

	cr_assert( strcmp(buf[0], "cd") == 0, "expected buf[%i]=%s actual=%s", 0, "cd", buf[0]);
	cr_assert( strcmp(buf[1], "./bin/sfish") == 0, "expected buf[%i]=%s actual=%s", 1, "./bin/sfish", buf[1]);
	cr_assert( strcmp(buf[2], ">") == 0, "expected buf[%i]=\"%s\" actual=\"%s\"", 2, ">", buf[2]);
	cr_assert( strcmp(buf[3], "outfile") == 0, "expected buf[%i]=%s actual=%s", 3, "outfile", buf[3]);
	cr_assert(buf[4] == NULL);
}

Test(sfish_helper_parseCmd, cmd_has_pipeline2_gt, .init=setup){
	char cmd[] = "cd ./bin/sfish> outfile";
	int count = countElements(cmd) + 1;
	cr_assert(count == 4 + 1);

	char* buf[count];
	char** ret = parseNCmd(cmd, buf, count);
	cr_assert(ret != NULL);

	cr_assert( strcmp(buf[0], "cd") == 0, "expected buf[%i]=%s actual=%s", 0, "cd", buf[0]);
	cr_assert( strcmp(buf[1], "./bin/sfish") == 0, "expected buf[%i]=%s actual=%s", 1, "./bin/sfish", buf[1]);
	cr_assert( strcmp(buf[2], ">") == 0, "expected buf[%i]=\"%s\" actual=\"%s\"", 2, ">", buf[2]);
	cr_assert( strcmp(buf[3], "outfile") == 0, "expected buf[%i]=%s actual=%s", 3, "outfile", buf[3]);
	cr_assert(buf[4] == NULL);
}

Test(sfish_helper_parseCmd, cmd_has_pipeline3_gt, .init=setup){
	char cmd[] = "cd ./bin/sfish >outfile";
	int count = countElements(cmd) + 1;
	cr_assert(count == 4 + 1);

	char* buf[count];
	char** ret = parseNCmd(cmd, buf, count);
	cr_assert(ret != NULL);

	cr_assert( strcmp(buf[0], "cd") == 0, "expected buf[%i]=%s actual=%s", 0, "cd", buf[0]);
	cr_assert( strcmp(buf[1], "./bin/sfish") == 0, "expected buf[%i]=%s actual=%s", 1, "./bin/sfish", buf[1]);
	cr_assert( strcmp(buf[2], ">") == 0, "expected buf[%i]=\"%s\" actual=\"%s\"", 2, ">", buf[2]);
	cr_assert( strcmp(buf[3], "outfile") == 0, "expected buf[%i]=%s actual=%s", 3, "outfile", buf[3]);
	cr_assert(buf[4] == NULL);
}

Test(sfish_helper_parseCmd, cmd_has_pipeline4_gt, .init=setup){
	char cmd[] = "cd ./bin/sfish>outfile";
	int count = countElements(cmd) + 1;
	cr_assert(count == 4 + 1);

	char* buf[count];
	char** ret = parseNCmd(cmd, buf, count);
	cr_assert(ret != NULL);

	cr_assert( strcmp(buf[0], "cd") == 0, "expected buf[%i]=%s actual=%s", 0, "cd", buf[0]);
	cr_assert( strcmp(buf[1], "./bin/sfish") == 0, "expected buf[%i]=%s actual=%s", 1, "./bin/sfish", buf[1]);
	cr_assert( strcmp(buf[2], ">") == 0, "expected buf[%i]=\"%s\" actual=\"%s\"", 2, ">", buf[2]);
	cr_assert( strcmp(buf[3], "outfile") == 0, "expected buf[%i]=%s actual=%s", 3, "outfile", buf[3]);
	cr_assert(buf[4] == NULL);
}

Test(sfish_helper_parseCmd, cmd_has_two_pipeline_gt, .init=setup){
	char cmd[] = "cd ./bin/sfish>outfile > asdf";
	int count = countElements(cmd) + 1;
	cr_assert(count == 6 + 1);

	char* buf[count];
	char** ret = parseNCmd(cmd, buf, count);
	cr_assert(ret != NULL);

	cr_assert( strcmp(buf[0], "cd") == 0, "expected buf[%i]=%s actual=%s", 0, "cd", buf[0]);
	cr_assert( strcmp(buf[1], "./bin/sfish") == 0, "expected buf[%i]=%s actual=%s", 1, "./bin/sfish", buf[1]);
	cr_assert( strcmp(buf[2], ">") == 0, "expected buf[%i]=\"%s\" actual=\"%s\"", 2, ">", buf[2]);
	cr_assert( strcmp(buf[3], "outfile") == 0, "expected buf[%i]=%s actual=%s", 3, "outfile", buf[3]);
	cr_assert( strcmp(buf[4], ">") == 0, "expected buf[%i]=%s actual=%s", 4, ">", buf[4]);
	cr_assert( strcmp(buf[5], "asdf") == 0, "expected buf[%i]=%s actual=%s", 5, "asdf", buf[5]);
	cr_assert(buf[6] == NULL);
}

Test(sfish_helper_parseCmd, cmd_has_two_pipeline2_gt, .init=setup){
	char cmd[] = "cd ./bin/sfish>outfile> asdf";
	int count = countElements(cmd) + 1;
	cr_assert(count == 6 + 1);

	char* buf[count];
	char** ret = parseNCmd(cmd, buf, count);
	cr_assert(ret != NULL);

	cr_assert( strcmp(buf[0], "cd") == 0, "expected buf[%i]=%s actual=%s", 0, "cd", buf[0]);
	cr_assert( strcmp(buf[1], "./bin/sfish") == 0, "expected buf[%i]=%s actual=%s", 1, "./bin/sfish", buf[1]);
	cr_assert( strcmp(buf[2], ">") == 0, "expected buf[%i]=\"%s\" actual=\"%s\"", 2, ">", buf[2]);
	cr_assert( strcmp(buf[3], "outfile") == 0, "expected buf[%i]=%s actual=%s", 3, "outfile", buf[3]);
	cr_assert( strcmp(buf[4], ">") == 0, "expected buf[%i]=%s actual=%s", 4, ">", buf[4]);
	cr_assert( strcmp(buf[5], "asdf") == 0, "expected buf[%i]=%s actual=%s", 5, "asdf", buf[5]);
	cr_assert(buf[6] == NULL);
}

Test(sfish_helper_parseCmd, cmd_has_two_pipeline3_gt, .init=setup){
	char cmd[] = "cd ./bin/sfish>outfile >asdf";
	int count = countElements(cmd) + 1;
	cr_assert(count == 6 + 1);

	char* buf[count];
	char** ret = parseNCmd(cmd, buf, count);
	cr_assert(ret != NULL);

	cr_assert( strcmp(buf[0], "cd") == 0, "expected buf[%i]=%s actual=%s", 0, "cd", buf[0]);
	cr_assert( strcmp(buf[1], "./bin/sfish") == 0, "expected buf[%i]=%s actual=%s", 1, "./bin/sfish", buf[1]);
	cr_assert( strcmp(buf[2], ">") == 0, "expected buf[%i]=\"%s\" actual=\"%s\"", 2, ">", buf[2]);
	cr_assert( strcmp(buf[3], "outfile") == 0, "expected buf[%i]=%s actual=%s", 3, "outfile", buf[3]);
	cr_assert( strcmp(buf[4], ">") == 0, "expected buf[%i]=%s actual=%s", 4, ">", buf[4]);
	cr_assert( strcmp(buf[5], "asdf") == 0, "expected buf[%i]=%s actual=%s", 5, "asdf", buf[5]);
	cr_assert(buf[6] == NULL);
}

Test(sfish_helper_parseCmd, cmd_has_two_pipeline4_gt, .init=setup){
	char cmd[] = "cd ./bin/sfish>outfile>asdf";
	int count = countElements(cmd) + 1;
	cr_assert(count == 6 + 1);

	char* buf[count];
	char** ret = parseNCmd(cmd, buf, count);
	cr_assert(ret != NULL);

	cr_assert( strcmp(buf[0], "cd") == 0, "expected buf[%i]=%s actual=%s", 0, "cd", buf[0]);
	cr_assert( strcmp(buf[1], "./bin/sfish") == 0, "expected buf[%i]=%s actual=%s", 1, "./bin/sfish", buf[1]);
	cr_assert( strcmp(buf[2], ">") == 0, "expected buf[%i]=\"%s\" actual=\"%s\"", 2, ">", buf[2]);
	cr_assert( strcmp(buf[3], "outfile") == 0, "expected buf[%i]=%s actual=%s", 3, "outfile", buf[3]);
	cr_assert( strcmp(buf[4], ">") == 0, "expected buf[%i]=%s actual=%s", 4, ">", buf[4]);
	cr_assert( strcmp(buf[5], "asdf") == 0, "expected buf[%i]=%s actual=%s", 5, "asdf", buf[5]);
	cr_assert(buf[6] == NULL);
}

Test(sfish_helper_parseCmd, cmd_has_two_pipeline5_gt, .init=setup){
	char cmd[] = "cd ./bin/sfish>outfile>asdf";
	int count = countElements(cmd) + 1;
	cr_assert(count == 6 + 1);

	char* buf[count];
	char** ret = parseNCmd(cmd, buf, count);
	cr_assert(ret != NULL);

	cr_assert( strcmp(buf[0], "cd") == 0, "expected buf[%i]=%s actual=%s", 0, "cd", buf[0]);
	cr_assert( strcmp(buf[1], "./bin/sfish") == 0, "expected buf[%i]=%s actual=%s", 1, "./bin/sfish", buf[1]);
	cr_assert( strcmp(buf[2], ">") == 0, "expected buf[%i]=\"%s\" actual=\"%s\"", 2, ">", buf[2]);
	cr_assert( strcmp(buf[3], "outfile") == 0, "expected buf[%i]=%s actual=%s", 3, "outfile", buf[3]);
	cr_assert( strcmp(buf[4], ">") == 0, "expected buf[%i]=%s actual=%s", 4, ">", buf[4]);
	cr_assert( strcmp(buf[5], "asdf") == 0, "expected buf[%i]=%s actual=%s", 5, "asdf", buf[5]);
	cr_assert(buf[6] == NULL);
}

Test(sfish_helper_parseCmd, cmd_has_two_pipeline_neighboring_gt, .init=setup){
	char cmd[] = "cd ./bin/sfish >> asdf";
	int count = countElements(cmd) + 1;
	cr_assert(count == 5 + 1);

	char* buf[count];
	char** ret = parseNCmd(cmd, buf, count);
	cr_assert(ret != NULL);

	cr_assert( strcmp(buf[0], "cd") == 0, "expected buf[%i]=%s actual=%s", 0, "cd", buf[0]);
	cr_assert( strcmp(buf[1], "./bin/sfish") == 0, "expected buf[%i]=%s actual=%s", 1, "./bin/sfish", buf[1]);
	cr_assert( strcmp(buf[2], ">") == 0, "expected buf[%i]=\"%s\" actual=\"%s\"", 2, ">", buf[2]);
	cr_assert( strcmp(buf[3], ">") == 0, "expected buf[%i]=%s actual=%s", 3, "outfile", buf[3]);
	cr_assert( strcmp(buf[4], "asdf") == 0, "expected buf[%i]=%s actual=%s", 4, "asdf", buf[4]);
	cr_assert(buf[5] == NULL);
}

Test(sfish_helper_parseCmd, cmd_has_two_pipeline_neighboring2_gt, .init=setup){
	char cmd[] = "cd ./bin/sfish>> asdf";
	int count = countElements(cmd) + 1;
	cr_assert(count == 5 + 1);

	char* buf[count];
	char** ret = parseNCmd(cmd, buf, count);
	cr_assert(ret != NULL);

	cr_assert( strcmp(buf[0], "cd") == 0, "expected buf[%i]=%s actual=%s", 0, "cd", buf[0]);
	cr_assert( strcmp(buf[1], "./bin/sfish") == 0, "expected buf[%i]=%s actual=%s", 1, "./bin/sfish", buf[1]);
	cr_assert( strcmp(buf[2], ">") == 0, "expected buf[%i]=\"%s\" actual=\"%s\"", 2, ">", buf[2]);
	cr_assert( strcmp(buf[3], ">") == 0, "expected buf[%i]=%s actual=%s", 3, "outfile", buf[3]);
	cr_assert( strcmp(buf[4], "asdf") == 0, "expected buf[%i]=%s actual=%s", 4, "asdf", buf[4]);
	cr_assert(buf[5] == NULL);
}

Test(sfish_helper_parseCmd, cmd_has_two_pipeline_neighboring3_gt, .init=setup){
	char cmd[] = "cd ./bin/sfish >>asdf";
	int count = countElements(cmd) + 1;
	cr_assert(count == 5 + 1);

	char* buf[count];
	char** ret = parseNCmd(cmd, buf, count);
	cr_assert(ret != NULL);

	cr_assert( strcmp(buf[0], "cd") == 0, "expected buf[%i]=%s actual=%s", 0, "cd", buf[0]);
	cr_assert( strcmp(buf[1], "./bin/sfish") == 0, "expected buf[%i]=%s actual=%s", 1, "./bin/sfish", buf[1]);
	cr_assert( strcmp(buf[2], ">") == 0, "expected buf[%i]=\"%s\" actual=\"%s\"", 2, ">", buf[2]);
	cr_assert( strcmp(buf[3], ">") == 0, "expected buf[%i]=%s actual=%s", 3, "outfile", buf[3]);
	cr_assert( strcmp(buf[4], "asdf") == 0, "expected buf[%i]=%s actual=%s", 4, "asdf", buf[4]);
	cr_assert(buf[5] == NULL);
}

Test(sfish_helper_parseCmd, cmd_has_two_pipeline_neighboring4_gt, .init=setup){
	char cmd[] = "cd ./bin/sfish>>asdf";
	int count = countElements(cmd) + 1;
	cr_assert(count == 5 + 1);

	char* buf[count];
	char** ret = parseNCmd(cmd, buf, count);
	cr_assert(ret != NULL);

	cr_assert( strcmp(buf[0], "cd") == 0, "expected buf[%i]=%s actual=%s", 0, "cd", buf[0]);
	cr_assert( strcmp(buf[1], "./bin/sfish") == 0, "expected buf[%i]=%s actual=%s", 1, "./bin/sfish", buf[1]);
	cr_assert( strcmp(buf[2], ">") == 0, "expected buf[%i]=\"%s\" actual=\"%s\"", 2, ">", buf[2]);
	cr_assert( strcmp(buf[3], ">") == 0, "expected buf[%i]=%s actual=%s", 3, "outfile", buf[3]);
	cr_assert( strcmp(buf[4], "asdf") == 0, "expected buf[%i]=%s actual=%s", 4, "asdf", buf[4]);
	cr_assert(buf[5] == NULL);
}

// pipeline(JS_PIPE)
Test(sfish_helper_parseCmd, cmd_has_pipeline, .init=setup){
	char cmd[] = "cd ./bin/sfish | outfile";
	int count = countElements(cmd) + 1;
	cr_assert(count == 4 + 1);

	char* buf[count];
	char** ret = parseNCmd(cmd, buf, count);
	cr_assert(ret != NULL);

	cr_assert( strcmp(buf[0], "cd") == 0, "expected buf[%i]=%s actual=%s", 0, "cd", buf[0]);
	cr_assert( strcmp(buf[1], "./bin/sfish") == 0, "expected buf[%i]=%s actual=%s", 1, "./bin/sfish", buf[1]);
	cr_assert( strcmp(buf[2], "|") == 0, "expected buf[%i]=\"%s\" actual=\"%s\"", 2, "|", buf[2]);
	cr_assert( strcmp(buf[3], "outfile") == 0, "expected buf[%i]=%s actual=%s", 3, "outfile", buf[3]);
	cr_assert(buf[4] == NULL);
}

Test(sfish_helper_parseCmd, cmd_has_pipeline2, .init=setup){
	char cmd[] = "cd ./bin/sfish| outfile";
	int count = countElements(cmd) + 1;
	cr_assert(count == 4 + 1);

	char* buf[count];
	char** ret = parseNCmd(cmd, buf, count);
	cr_assert(ret != NULL);

	cr_assert( strcmp(buf[0], "cd") == 0, "expected buf[%i]=%s actual=%s", 0, "cd", buf[0]);
	cr_assert( strcmp(buf[1], "./bin/sfish") == 0, "expected buf[%i]=%s actual=%s", 1, "./bin/sfish", buf[1]);
	cr_assert( strcmp(buf[2], "|") == 0, "expected buf[%i]=\"%s\" actual=\"%s\"", 2, "|", buf[2]);
	cr_assert( strcmp(buf[3], "outfile") == 0, "expected buf[%i]=%s actual=%s", 3, "outfile", buf[3]);
	cr_assert(buf[4] == NULL);
}

Test(sfish_helper_parseCmd, cmd_has_pipeline3, .init=setup){
	char cmd[] = "cd ./bin/sfish |outfile";
	int count = countElements(cmd) + 1;
	cr_assert(count == 4 + 1);

	char* buf[count];
	char** ret = parseNCmd(cmd, buf, count);
	cr_assert(ret != NULL);

	cr_assert( strcmp(buf[0], "cd") == 0, "expected buf[%i]=%s actual=%s", 0, "cd", buf[0]);
	cr_assert( strcmp(buf[1], "./bin/sfish") == 0, "expected buf[%i]=%s actual=%s", 1, "./bin/sfish", buf[1]);
	cr_assert( strcmp(buf[2], "|") == 0, "expected buf[%i]=\"%s\" actual=\"%s\"", 2, "|", buf[2]);
	cr_assert( strcmp(buf[3], "outfile") == 0, "expected buf[%i]=%s actual=%s", 3, "outfile", buf[3]);
	cr_assert(buf[4] == NULL);
}

Test(sfish_helper_parseCmd, cmd_has_pipeline4, .init=setup){
	char cmd[] = "cd ./bin/sfish|outfile";
	int count = countElements(cmd) + 1;
	cr_assert(count == 4 + 1);

	char* buf[count];
	char** ret = parseNCmd(cmd, buf, count);
	cr_assert(ret != NULL);

	cr_assert( strcmp(buf[0], "cd") == 0, "expected buf[%i]=%s actual=%s", 0, "cd", buf[0]);
	cr_assert( strcmp(buf[1], "./bin/sfish") == 0, "expected buf[%i]=%s actual=%s", 1, "./bin/sfish", buf[1]);
	cr_assert( strcmp(buf[2], "|") == 0, "expected buf[%i]=\"%s\" actual=\"%s\"", 2, "|", buf[2]);
	cr_assert( strcmp(buf[3], "outfile") == 0, "expected buf[%i]=%s actual=%s", 3, "outfile", buf[3]);
	cr_assert(buf[4] == NULL);
}

Test(sfish_helper_parseCmd, cmd_has_two_pipeline, .init=setup){
	char cmd[] = "cd ./bin/sfish|outfile | asdf";
	int count = countElements(cmd) + 1;
	cr_assert(count == 6 + 1);

	char* buf[count];
	char** ret = parseNCmd(cmd, buf, count);
	cr_assert(ret != NULL);

	cr_assert( strcmp(buf[0], "cd") == 0, "expected buf[%i]=%s actual=%s", 0, "cd", buf[0]);
	cr_assert( strcmp(buf[1], "./bin/sfish") == 0, "expected buf[%i]=%s actual=%s", 1, "./bin/sfish", buf[1]);
	cr_assert( strcmp(buf[2], "|") == 0, "expected buf[%i]=\"%s\" actual=\"%s\"", 2, "|", buf[2]);
	cr_assert( strcmp(buf[3], "outfile") == 0, "expected buf[%i]=%s actual=%s", 3, "outfile", buf[3]);
	cr_assert( strcmp(buf[4], "|") == 0, "expected buf[%i]=%s actual=%s", 4, "|", buf[4]);
	cr_assert( strcmp(buf[5], "asdf") == 0, "expected buf[%i]=%s actual=%s", 5, "asdf", buf[5]);
	cr_assert(buf[6] == NULL);
}

Test(sfish_helper_parseCmd, cmd_has_two_pipeline2, .init=setup){
	char cmd[] = "cd ./bin/sfish|outfile| asdf";
	int count = countElements(cmd) + 1;
	cr_assert(count == 6 + 1);

	char* buf[count];
	char** ret = parseNCmd(cmd, buf, count);
	cr_assert(ret != NULL);

	cr_assert( strcmp(buf[0], "cd") == 0, "expected buf[%i]=%s actual=%s", 0, "cd", buf[0]);
	cr_assert( strcmp(buf[1], "./bin/sfish") == 0, "expected buf[%i]=%s actual=%s", 1, "./bin/sfish", buf[1]);
	cr_assert( strcmp(buf[2], "|") == 0, "expected buf[%i]=\"%s\" actual=\"%s\"", 2, "|", buf[2]);
	cr_assert( strcmp(buf[3], "outfile") == 0, "expected buf[%i]=%s actual=%s", 3, "outfile", buf[3]);
	cr_assert( strcmp(buf[4], "|") == 0, "expected buf[%i]=%s actual=%s", 4, "|", buf[4]);
	cr_assert( strcmp(buf[5], "asdf") == 0, "expected buf[%i]=%s actual=%s", 5, "asdf", buf[5]);
	cr_assert(buf[6] == NULL);
}

Test(sfish_helper_parseCmd, cmd_has_two_pipeline3, .init=setup){
	char cmd[] = "cd ./bin/sfish|outfile |asdf";
	int count = countElements(cmd) + 1;
	cr_assert(count == 6 + 1);

	char* buf[count];
	char** ret = parseNCmd(cmd, buf, count);
	cr_assert(ret != NULL);

	cr_assert( strcmp(buf[0], "cd") == 0, "expected buf[%i]=%s actual=%s", 0, "cd", buf[0]);
	cr_assert( strcmp(buf[1], "./bin/sfish") == 0, "expected buf[%i]=%s actual=%s", 1, "./bin/sfish", buf[1]);
	cr_assert( strcmp(buf[2], "|") == 0, "expected buf[%i]=\"%s\" actual=\"%s\"", 2, "|", buf[2]);
	cr_assert( strcmp(buf[3], "outfile") == 0, "expected buf[%i]=%s actual=%s", 3, "outfile", buf[3]);
	cr_assert( strcmp(buf[4], "|") == 0, "expected buf[%i]=%s actual=%s", 4, "|", buf[4]);
	cr_assert( strcmp(buf[5], "asdf") == 0, "expected buf[%i]=%s actual=%s", 5, "asdf", buf[5]);
	cr_assert(buf[6] == NULL);
}

Test(sfish_helper_parseCmd, cmd_has_two_pipeline4, .init=setup){
	char cmd[] = "cd ./bin/sfish|outfile|asdf";
	int count = countElements(cmd) + 1;
	cr_assert(count == 6 + 1);

	char* buf[count];
	char** ret = parseNCmd(cmd, buf, count);
	cr_assert(ret != NULL);

	cr_assert( strcmp(buf[0], "cd") == 0, "expected buf[%i]=%s actual=%s", 0, "cd", buf[0]);
	cr_assert( strcmp(buf[1], "./bin/sfish") == 0, "expected buf[%i]=%s actual=%s", 1, "./bin/sfish", buf[1]);
	cr_assert( strcmp(buf[2], "|") == 0, "expected buf[%i]=\"%s\" actual=\"%s\"", 2, "|", buf[2]);
	cr_assert( strcmp(buf[3], "outfile") == 0, "expected buf[%i]=%s actual=%s", 3, "outfile", buf[3]);
	cr_assert( strcmp(buf[4], "|") == 0, "expected buf[%i]=%s actual=%s", 4, "|", buf[4]);
	cr_assert( strcmp(buf[5], "asdf") == 0, "expected buf[%i]=%s actual=%s", 5, "asdf", buf[5]);
	cr_assert(buf[6] == NULL);
}

Test(sfish_helper_parseCmd, cmd_has_two_pipeline5, .init=setup){
	char cmd[] = "cd ./bin/sfish|outfile|asdf";
	int count = countElements(cmd) + 1;
	cr_assert(count == 6 + 1);

	char* buf[count];
	char** ret = parseNCmd(cmd, buf, count);
	cr_assert(ret != NULL);

	cr_assert( strcmp(buf[0], "cd") == 0, "expected buf[%i]=%s actual=%s", 0, "cd", buf[0]);
	cr_assert( strcmp(buf[1], "./bin/sfish") == 0, "expected buf[%i]=%s actual=%s", 1, "./bin/sfish", buf[1]);
	cr_assert( strcmp(buf[2], "|") == 0, "expected buf[%i]=\"%s\" actual=\"%s\"", 2, "|", buf[2]);
	cr_assert( strcmp(buf[3], "outfile") == 0, "expected buf[%i]=%s actual=%s", 3, "outfile", buf[3]);
	cr_assert( strcmp(buf[4], "|") == 0, "expected buf[%i]=%s actual=%s", 4, "|", buf[4]);
	cr_assert( strcmp(buf[5], "asdf") == 0, "expected buf[%i]=%s actual=%s", 5, "asdf", buf[5]);
	cr_assert(buf[6] == NULL);
}

Test(sfish_helper_parseCmd, cmd_has_two_pipeline_neighboring, .init=setup){
	char cmd[] = "cd ./bin/sfish || asdf";
	int count = countElements(cmd) + 1;
	cr_assert(count == 5 + 1);

	char* buf[count];
	char** ret = parseNCmd(cmd, buf, count);
	cr_assert(ret != NULL);

	cr_assert( strcmp(buf[0], "cd") == 0, "expected buf[%i]=%s actual=%s", 0, "cd", buf[0]);
	cr_assert( strcmp(buf[1], "./bin/sfish") == 0, "expected buf[%i]=%s actual=%s", 1, "./bin/sfish", buf[1]);
	cr_assert( strcmp(buf[2], "|") == 0, "expected buf[%i]=\"%s\" actual=\"%s\"", 2, "|", buf[2]);
	cr_assert( strcmp(buf[3], "|") == 0, "expected buf[%i]=%s actual=%s", 3, "outfile", buf[3]);
	cr_assert( strcmp(buf[4], "asdf") == 0, "expected buf[%i]=%s actual=%s", 4, "asdf", buf[4]);
	cr_assert(buf[5] == NULL);
}

Test(sfish_helper_parseCmd, cmd_has_two_pipeline_neighboring2, .init=setup){
	char cmd[] = "cd ./bin/sfish|| asdf";
	int count = countElements(cmd) + 1;
	cr_assert(count == 5 + 1);

	char* buf[count];
	char** ret = parseNCmd(cmd, buf, count);
	cr_assert(ret != NULL);

	cr_assert( strcmp(buf[0], "cd") == 0, "expected buf[%i]=%s actual=%s", 0, "cd", buf[0]);
	cr_assert( strcmp(buf[1], "./bin/sfish") == 0, "expected buf[%i]=%s actual=%s", 1, "./bin/sfish", buf[1]);
	cr_assert( strcmp(buf[2], "|") == 0, "expected buf[%i]=\"%s\" actual=\"%s\"", 2, "|", buf[2]);
	cr_assert( strcmp(buf[3], "|") == 0, "expected buf[%i]=%s actual=%s", 3, "outfile", buf[3]);
	cr_assert( strcmp(buf[4], "asdf") == 0, "expected buf[%i]=%s actual=%s", 4, "asdf", buf[4]);
	cr_assert(buf[5] == NULL);
}

Test(sfish_helper_parseCmd, cmd_has_two_pipeline_neighboring3, .init=setup){
	char cmd[] = "cd ./bin/sfish ||asdf";
	int count = countElements(cmd) + 1;
	cr_assert(count == 5 + 1);

	char* buf[count];
	char** ret = parseNCmd(cmd, buf, count);
	cr_assert(ret != NULL);

	cr_assert( strcmp(buf[0], "cd") == 0, "expected buf[%i]=%s actual=%s", 0, "cd", buf[0]);
	cr_assert( strcmp(buf[1], "./bin/sfish") == 0, "expected buf[%i]=%s actual=%s", 1, "./bin/sfish", buf[1]);
	cr_assert( strcmp(buf[2], "|") == 0, "expected buf[%i]=\"%s\" actual=\"%s\"", 2, "|", buf[2]);
	cr_assert( strcmp(buf[3], "|") == 0, "expected buf[%i]=%s actual=%s", 3, "outfile", buf[3]);
	cr_assert( strcmp(buf[4], "asdf") == 0, "expected buf[%i]=%s actual=%s", 4, "asdf", buf[4]);
	cr_assert(buf[5] == NULL);
}

Test(sfish_helper_parseCmd, cmd_has_two_pipeline_neighboring4, .init=setup){
	char cmd[] = "cd ./bin/sfish||asdf";
	int count = countElements(cmd) + 1;
	cr_assert(count == 5 + 1);

	char* buf[count];
	char** ret = parseNCmd(cmd, buf, count);
	cr_assert(ret != NULL);

	cr_assert( strcmp(buf[0], "cd") == 0, "expected buf[%i]=%s actual=%s", 0, "cd", buf[0]);
	cr_assert( strcmp(buf[1], "./bin/sfish") == 0, "expected buf[%i]=%s actual=%s", 1, "./bin/sfish", buf[1]);
	cr_assert( strcmp(buf[2], "|") == 0, "expected buf[%i]=\"%s\" actual=\"%s\"", 2, "|", buf[2]);
	cr_assert( strcmp(buf[3], "|") == 0, "expected buf[%i]=%s actual=%s", 3, "outfile", buf[3]);
	cr_assert( strcmp(buf[4], "asdf") == 0, "expected buf[%i]=%s actual=%s", 4, "asdf", buf[4]);
	cr_assert(buf[5] == NULL);
}

// pipeline(lt)
Test(sfish_helper_parseCmd, cmd_has_pipeline_lt, .init=setup){
	char cmd[] = "cd ./bin/sfish | outfile";
	int count = countElements(cmd) + 1;
	cr_assert(count == 4 + 1);

	char* buf[count];
	char** ret = parseNCmd(cmd, buf, count);
	cr_assert(ret != NULL);

	cr_assert( strcmp(buf[0], "cd") == 0, "expected buf[%i]=%s actual=%s", 0, "cd", buf[0]);
	cr_assert( strcmp(buf[1], "./bin/sfish") == 0, "expected buf[%i]=%s actual=%s", 1, "./bin/sfish", buf[1]);
	cr_assert( strcmp(buf[2], "|") == 0, "expected buf[%i]=\"%s\" actual=\"%s\"", 2, "|", buf[2]);
	cr_assert( strcmp(buf[3], "outfile") == 0, "expected buf[%i]=%s actual=%s", 3, "outfile", buf[3]);
	cr_assert(buf[4] == NULL);
}

Test(sfish_helper_parseCmd, cmd_has_pipeline2_lt, .init=setup){
	char cmd[] = "cd ./bin/sfish| outfile";
	int count = countElements(cmd) + 1;
	cr_assert(count == 4 + 1);

	char* buf[count];
	char** ret = parseNCmd(cmd, buf, count);
	cr_assert(ret != NULL);

	cr_assert( strcmp(buf[0], "cd") == 0, "expected buf[%i]=%s actual=%s", 0, "cd", buf[0]);
	cr_assert( strcmp(buf[1], "./bin/sfish") == 0, "expected buf[%i]=%s actual=%s", 1, "./bin/sfish", buf[1]);
	cr_assert( strcmp(buf[2], "|") == 0, "expected buf[%i]=\"%s\" actual=\"%s\"", 2, "|", buf[2]);
	cr_assert( strcmp(buf[3], "outfile") == 0, "expected buf[%i]=%s actual=%s", 3, "outfile", buf[3]);
	cr_assert(buf[4] == NULL);
}

Test(sfish_helper_parseCmd, cmd_has_pipeline3_lt, .init=setup){
	char cmd[] = "cd ./bin/sfish |outfile";
	int count = countElements(cmd) + 1;
	cr_assert(count == 4 + 1);

	char* buf[count];
	char** ret = parseNCmd(cmd, buf, count);
	cr_assert(ret != NULL);

	cr_assert( strcmp(buf[0], "cd") == 0, "expected buf[%i]=%s actual=%s", 0, "cd", buf[0]);
	cr_assert( strcmp(buf[1], "./bin/sfish") == 0, "expected buf[%i]=%s actual=%s", 1, "./bin/sfish", buf[1]);
	cr_assert( strcmp(buf[2], "|") == 0, "expected buf[%i]=\"%s\" actual=\"%s\"", 2, "|", buf[2]);
	cr_assert( strcmp(buf[3], "outfile") == 0, "expected buf[%i]=%s actual=%s", 3, "outfile", buf[3]);
	cr_assert(buf[4] == NULL);
}

Test(sfish_helper_parseCmd, cmd_has_pipeline4_lt, .init=setup){
	char cmd[] = "cd ./bin/sfish|outfile";
	int count = countElements(cmd) + 1;
	cr_assert(count == 4 + 1);

	char* buf[count];
	char** ret = parseNCmd(cmd, buf, count);
	cr_assert(ret != NULL);

	cr_assert( strcmp(buf[0], "cd") == 0, "expected buf[%i]=%s actual=%s", 0, "cd", buf[0]);
	cr_assert( strcmp(buf[1], "./bin/sfish") == 0, "expected buf[%i]=%s actual=%s", 1, "./bin/sfish", buf[1]);
	cr_assert( strcmp(buf[2], "|") == 0, "expected buf[%i]=\"%s\" actual=\"%s\"", 2, "|", buf[2]);
	cr_assert( strcmp(buf[3], "outfile") == 0, "expected buf[%i]=%s actual=%s", 3, "outfile", buf[3]);
	cr_assert(buf[4] == NULL);
}

Test(sfish_helper_parseCmd, cmd_has_two_pipeline_lt, .init=setup){
	char cmd[] = "cd ./bin/sfish|outfile | asdf";
	int count = countElements(cmd) + 1;
	cr_assert(count == 6 + 1);

	char* buf[count];
	char** ret = parseNCmd(cmd, buf, count);
	cr_assert(ret != NULL);

	cr_assert( strcmp(buf[0], "cd") == 0, "expected buf[%i]=%s actual=%s", 0, "cd", buf[0]);
	cr_assert( strcmp(buf[1], "./bin/sfish") == 0, "expected buf[%i]=%s actual=%s", 1, "./bin/sfish", buf[1]);
	cr_assert( strcmp(buf[2], "|") == 0, "expected buf[%i]=\"%s\" actual=\"%s\"", 2, "|", buf[2]);
	cr_assert( strcmp(buf[3], "outfile") == 0, "expected buf[%i]=%s actual=%s", 3, "outfile", buf[3]);
	cr_assert( strcmp(buf[4], "|") == 0, "expected buf[%i]=%s actual=%s", 4, "|", buf[4]);
	cr_assert( strcmp(buf[5], "asdf") == 0, "expected buf[%i]=%s actual=%s", 5, "asdf", buf[5]);
	cr_assert(buf[6] == NULL);
}

Test(sfish_helper_parseCmd, cmd_has_two_pipeline2_lt, .init=setup){
	char cmd[] = "cd ./bin/sfish|outfile| asdf";
	int count = countElements(cmd) + 1;
	cr_assert(count == 6 + 1);

	char* buf[count];
	char** ret = parseNCmd(cmd, buf, count);
	cr_assert(ret != NULL);

	cr_assert( strcmp(buf[0], "cd") == 0, "expected buf[%i]=%s actual=%s", 0, "cd", buf[0]);
	cr_assert( strcmp(buf[1], "./bin/sfish") == 0, "expected buf[%i]=%s actual=%s", 1, "./bin/sfish", buf[1]);
	cr_assert( strcmp(buf[2], "|") == 0, "expected buf[%i]=\"%s\" actual=\"%s\"", 2, "|", buf[2]);
	cr_assert( strcmp(buf[3], "outfile") == 0, "expected buf[%i]=%s actual=%s", 3, "outfile", buf[3]);
	cr_assert( strcmp(buf[4], "|") == 0, "expected buf[%i]=%s actual=%s", 4, "|", buf[4]);
	cr_assert( strcmp(buf[5], "asdf") == 0, "expected buf[%i]=%s actual=%s", 5, "asdf", buf[5]);
	cr_assert(buf[6] == NULL);
}

Test(sfish_helper_parseCmd, cmd_has_two_pipeline3_lt, .init=setup){
	char cmd[] = "cd ./bin/sfish|outfile |asdf";
	int count = countElements(cmd) + 1;
	cr_assert(count == 6 + 1);

	char* buf[count];
	char** ret = parseNCmd(cmd, buf, count);
	cr_assert(ret != NULL);

	cr_assert( strcmp(buf[0], "cd") == 0, "expected buf[%i]=%s actual=%s", 0, "cd", buf[0]);
	cr_assert( strcmp(buf[1], "./bin/sfish") == 0, "expected buf[%i]=%s actual=%s", 1, "./bin/sfish", buf[1]);
	cr_assert( strcmp(buf[2], "|") == 0, "expected buf[%i]=\"%s\" actual=\"%s\"", 2, "|", buf[2]);
	cr_assert( strcmp(buf[3], "outfile") == 0, "expected buf[%i]=%s actual=%s", 3, "outfile", buf[3]);
	cr_assert( strcmp(buf[4], "|") == 0, "expected buf[%i]=%s actual=%s", 4, "|", buf[4]);
	cr_assert( strcmp(buf[5], "asdf") == 0, "expected buf[%i]=%s actual=%s", 5, "asdf", buf[5]);
	cr_assert(buf[6] == NULL);
}

Test(sfish_helper_parseCmd, cmd_has_two_pipeline4_lt, .init=setup){
	char cmd[] = "cd ./bin/sfish|outfile|asdf";
	int count = countElements(cmd) + 1;
	cr_assert(count == 6 + 1);

	char* buf[count];
	char** ret = parseNCmd(cmd, buf, count);
	cr_assert(ret != NULL);

	cr_assert( strcmp(buf[0], "cd") == 0, "expected buf[%i]=%s actual=%s", 0, "cd", buf[0]);
	cr_assert( strcmp(buf[1], "./bin/sfish") == 0, "expected buf[%i]=%s actual=%s", 1, "./bin/sfish", buf[1]);
	cr_assert( strcmp(buf[2], "|") == 0, "expected buf[%i]=\"%s\" actual=\"%s\"", 2, "|", buf[2]);
	cr_assert( strcmp(buf[3], "outfile") == 0, "expected buf[%i]=%s actual=%s", 3, "outfile", buf[3]);
	cr_assert( strcmp(buf[4], "|") == 0, "expected buf[%i]=%s actual=%s", 4, "|", buf[4]);
	cr_assert( strcmp(buf[5], "asdf") == 0, "expected buf[%i]=%s actual=%s", 5, "asdf", buf[5]);
	cr_assert(buf[6] == NULL);
}

Test(sfish_helper_parseCmd, cmd_has_two_pipeline5_lt, .init=setup){
	char cmd[] = "cd ./bin/sfish|outfile|asdf";
	int count = countElements(cmd) + 1;
	cr_assert(count == 6 + 1);

	char* buf[count];
	char** ret = parseNCmd(cmd, buf, count);
	cr_assert(ret != NULL);

	cr_assert( strcmp(buf[0], "cd") == 0, "expected buf[%i]=%s actual=%s", 0, "cd", buf[0]);
	cr_assert( strcmp(buf[1], "./bin/sfish") == 0, "expected buf[%i]=%s actual=%s", 1, "./bin/sfish", buf[1]);
	cr_assert( strcmp(buf[2], "|") == 0, "expected buf[%i]=\"%s\" actual=\"%s\"", 2, "|", buf[2]);
	cr_assert( strcmp(buf[3], "outfile") == 0, "expected buf[%i]=%s actual=%s", 3, "outfile", buf[3]);
	cr_assert( strcmp(buf[4], "|") == 0, "expected buf[%i]=%s actual=%s", 4, "|", buf[4]);
	cr_assert( strcmp(buf[5], "asdf") == 0, "expected buf[%i]=%s actual=%s", 5, "asdf", buf[5]);
	cr_assert(buf[6] == NULL);
}

Test(sfish_helper_parseCmd, cmd_has_two_pipeline_neighboring_lt, .init=setup){
	char cmd[] = "cd ./bin/sfish || asdf";
	int count = countElements(cmd) + 1;
	cr_assert(count == 5 + 1);

	char* buf[count];
	char** ret = parseNCmd(cmd, buf, count);
	cr_assert(ret != NULL);

	cr_assert( strcmp(buf[0], "cd") == 0, "expected buf[%i]=%s actual=%s", 0, "cd", buf[0]);
	cr_assert( strcmp(buf[1], "./bin/sfish") == 0, "expected buf[%i]=%s actual=%s", 1, "./bin/sfish", buf[1]);
	cr_assert( strcmp(buf[2], "|") == 0, "expected buf[%i]=\"%s\" actual=\"%s\"", 2, "|", buf[2]);
	cr_assert( strcmp(buf[3], "|") == 0, "expected buf[%i]=%s actual=%s", 3, "outfile", buf[3]);
	cr_assert( strcmp(buf[4], "asdf") == 0, "expected buf[%i]=%s actual=%s", 4, "asdf", buf[4]);
	cr_assert(buf[5] == NULL);
}

Test(sfish_helper_parseCmd, cmd_has_two_pipeline_neighboring2_lt, .init=setup){
	char cmd[] = "cd ./bin/sfish|| asdf";
	int count = countElements(cmd) + 1;
	cr_assert(count == 5 + 1);

	char* buf[count];
	char** ret = parseNCmd(cmd, buf, count);
	cr_assert(ret != NULL);

	cr_assert( strcmp(buf[0], "cd") == 0, "expected buf[%i]=%s actual=%s", 0, "cd", buf[0]);
	cr_assert( strcmp(buf[1], "./bin/sfish") == 0, "expected buf[%i]=%s actual=%s", 1, "./bin/sfish", buf[1]);
	cr_assert( strcmp(buf[2], "|") == 0, "expected buf[%i]=\"%s\" actual=\"%s\"", 2, "|", buf[2]);
	cr_assert( strcmp(buf[3], "|") == 0, "expected buf[%i]=%s actual=%s", 3, "outfile", buf[3]);
	cr_assert( strcmp(buf[4], "asdf") == 0, "expected buf[%i]=%s actual=%s", 4, "asdf", buf[4]);
	cr_assert(buf[5] == NULL);
}

Test(sfish_helper_parseCmd, cmd_has_two_pipeline_neighboring3_lt, .init=setup){
	char cmd[] = "cd ./bin/sfish ||asdf";
	int count = countElements(cmd) + 1;
	cr_assert(count == 5 + 1);

	char* buf[count];
	char** ret = parseNCmd(cmd, buf, count);
	cr_assert(ret != NULL);

	cr_assert( strcmp(buf[0], "cd") == 0, "expected buf[%i]=%s actual=%s", 0, "cd", buf[0]);
	cr_assert( strcmp(buf[1], "./bin/sfish") == 0, "expected buf[%i]=%s actual=%s", 1, "./bin/sfish", buf[1]);
	cr_assert( strcmp(buf[2], "|") == 0, "expected buf[%i]=\"%s\" actual=\"%s\"", 2, "|", buf[2]);
	cr_assert( strcmp(buf[3], "|") == 0, "expected buf[%i]=%s actual=%s", 3, "outfile", buf[3]);
	cr_assert( strcmp(buf[4], "asdf") == 0, "expected buf[%i]=%s actual=%s", 4, "asdf", buf[4]);
	cr_assert(buf[5] == NULL);
}

Test(sfish_helper_parseCmd, cmd_has_two_pipeline_neighboring4_lt, .init=setup){
	char cmd[] = "cd ./bin/sfish||asdf";
	int count = countElements(cmd) + 1;
	cr_assert(count == 5 + 1);

	char* buf[count];
	char** ret = parseNCmd(cmd, buf, count);
	cr_assert(ret != NULL);

	cr_assert( strcmp(buf[0], "cd") == 0, "expected buf[%i]=%s actual=%s", 0, "cd", buf[0]);
	cr_assert( strcmp(buf[1], "./bin/sfish") == 0, "expected buf[%i]=%s actual=%s", 1, "./bin/sfish", buf[1]);
	cr_assert( strcmp(buf[2], "|") == 0, "expected buf[%i]=\"%s\" actual=\"%s\"", 2, "|", buf[2]);
	cr_assert( strcmp(buf[3], "|") == 0, "expected buf[%i]=%s actual=%s", 3, "outfile", buf[3]);
	cr_assert( strcmp(buf[4], "asdf") == 0, "expected buf[%i]=%s actual=%s", 4, "asdf", buf[4]);
	cr_assert(buf[5] == NULL);
}

// builtin_cd
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

	cr_assert( strcmp(prev_hist, next_cdir) == 0 );
	cr_assert( strcmp(next_hist, prev_cdir) == 0 );
}

Test(builtin_cd, cdFolder_Not_Exist, .init=setup) {
	int ret;
	char prev_cdir[PATH_MAX], next_cdir[PATH_MAX], prev_hist[PATH_MAX], next_hist[PATH_MAX];
	
	getcwd(prev_cdir, PATH_MAX);
	strcpy(prev_hist, cd_history);

	ret = builtin_cd("./asdklfjaklwejflkqwkelrjiosjvklds");
	cr_assert(ret == SF_FAIL, "Expected to fail, but return was %d\n", ret);

	getcwd(next_cdir, PATH_MAX);
	strcpy(next_hist, cd_history);

	cr_assert( strcmp(prev_hist, next_hist) == 0 );
	cr_assert( strcmp(prev_cdir, next_cdir) == 0 );
}

Test(builtin_cd, cdFolder_Exist, .init=setup) {
	int ret;
	char prev_cdir[PATH_MAX], next_cdir[PATH_MAX], prev_hist[PATH_MAX], next_hist[PATH_MAX];
	
	getcwd(prev_cdir, PATH_MAX);
	strcpy(prev_hist, cd_history);

	printf("%s\n", prev_cdir);
	ret = builtin_cd("./ysoh/hw4/test");
	cr_assert(ret == SF_SUCCESS, "Expected to sucess, but return was %d\n", ret);

	getcwd(next_cdir, PATH_MAX);
	strcpy(next_hist, cd_history);

	cr_assert( strcmp(next_hist, prev_cdir) == 0 );
	cr_assert( strcmp(next_cdir, "/home/dan/ysoh/hw4/test") == 0);
}

Test(builtin_cd, cdNULL, .init=setup) {
	int ret;
	char prev_cdir[PATH_MAX], next_cdir[PATH_MAX], prev_hist[PATH_MAX], next_hist[PATH_MAX];
	char *existing_folder = "/home/dan/ysoh/hw4/test";
	
	// change current dir to variable existing_folder
	getcwd(prev_cdir, PATH_MAX);
	strcpy(prev_hist, cd_history);

	ret = builtin_cd(existing_folder);
	cr_assert(ret == SF_SUCCESS, "Expected to sucess, but return was %d\n", ret);

	getcwd(next_cdir, PATH_MAX);
	strcpy(next_hist, cd_history);

	cr_assert( strcmp(next_hist, prev_cdir) == 0 );
	cr_assert( strcmp(next_cdir, "/home/dan/ysoh/hw4/test") == 0);

	// "cd", i.e., change to HOME dir
	getcwd(prev_cdir, PATH_MAX);
	strcpy(prev_hist, cd_history);

	ret = builtin_cd(NULL);
	cr_assert(ret == SF_SUCCESS, "Expected to sucess, but return was %d\n", ret);

	getcwd(next_cdir, PATH_MAX);
	strcpy(next_hist, cd_history);

	checkCurDir(HOME);
	checkCurHist(existing_folder);
}

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
	cr_assert( strcmp(pmt, "sfish-\x1B[0mdan\x1B[0m@\x1B[0mdan-ubuntu\x1B[0m:[~]> ") == 0, 
		"Expected:\"%s\" vs Real:\"%s\"", 
		"sfish-\x1B[0mdan\x1B[0m@\x1B[0mdan-ubuntu\x1B[0m:[~]> ", pmt);

	// turn off user
	argc = 3;
	argv = parseNCmd(cmd0, buf, argc);

	builtin_chpmt(argc, argv);

	pmt = getsnPrompt(promptBuf, len);
	cr_assert( strcmp(pmt, "sfish-\x1B[0mdan-ubuntu\x1B[0m:[~]> ") == 0, 
		"Actual: %s\n", pmt);

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
	cr_assert( strcmp(pmt, "sfish-\x1B[0mdan\x1B[0m:[~]> ") == 0);

	// turn on machine
	argc = 3;
	argv = parseNCmd(cmd3, buf, argc);

	builtin_chpmt(argc, argv);

	pmt = getsnPrompt(promptBuf, len);
	cr_assert( strcmp(pmt, "sfish-\x1B[0mdan\x1B[0m@\x1B[0mdan-ubuntu\x1B[0m:[~]> ") == 0);

	free(promptBuf);
}