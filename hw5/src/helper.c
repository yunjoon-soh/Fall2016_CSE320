// helper.c
#include "helper.h"

/*
* Functions
*/

/* Split a line by comma, store each one of them into the buffer*/
/*
Edge cases: 
1)cnt = 0
2)size of buf > cnt 
3)size of buf < cnt
4) |',' in line| < cnt 
5) |',' in line| > cnt
6) empty line
7) negative cnt
*/
char **splitByComma(char *line, char *buf[], size_t cnt){
	char *ptr = line;

	int idx = 0;
	buf[idx++] = ptr;

	while(*ptr != '\0' && idx <= cnt){
		// e.g., no comma: idx = 1, cnt = 0
		// e.g., one comma: idx = 1, cnt = 1
		//       if ',' found for the first time, idx becomes 2 and exit loop

		if(*ptr == ','){
			*ptr++ = '\0'; // convert comma to null term and move on to next
			buf[idx++] = ptr; // next char is new head of the buf
		}
		else {
			ptr += 1; // move on to next char
		}
	}

	return buf;
}

/* cntry_code_converter(..)
Encode two character into int value, 
i.e., treat them as base-26 values and convert to decimal
*/
int cntry_code_converter(char code[2]){
	int toRet = 0;

	if(code[0] >= 'a' && code[0] <= 'z'){
		toRet += (code[0] - 'a') * 26;
	} else if(code[0] >= 'A' && code[0] <= 'Z'){
		toRet += (code[0] - 'A') * 26;
	} else {
		return -1;
	}

	if(code[1] >= 'a' && code[1] <= 'z'){
		toRet += (code[1] - 'a');
	} else if(code[1] >= 'A' && code[1] <= 'Z'){
		toRet += (code[1] - 'A');
	} else {
		return -1;
	}

	return toRet;
}

/*
* Wrapper
*/
FILE **Fopen(const char *path, const char *mode, FILE **fp){
	*fp = fopen(path, mode);
	if(*fp == NULL){
		perror(path);
		exit(EXIT_FAILURE);
	}

	return fp;
}

DIR **Opendir(const char *name, DIR **dir){
	if((*dir = opendir (name)) != NULL){
		return dir;
	} else {
		perror("");
		exit(EXIT_FAILURE);
	}

	return dir;
}

int Closedir(DIR **pdir){
	if(closedir(*pdir) != -1){
		return 0;
	} else {
		perror("");
		exit(EXIT_FAILURE);
	}

	return -1;
}

int Pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg){
	int ret = pthread_create(thread, attr, start_routine, arg);
	if(ret){
		error("ERROR; return code from pthread_create() is %d\n", ret);
	}

	return ret;
}

int Pthread_join(pthread_t thread, void **retval){
	int ret = pthread_join(thread, retval);

	if(ret){
		errno = ret;
		perror("");
	}

	return ret;
}