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

char** cntry_code_reverter(int code, char **buf){
	(*buf)[0] = (char) ((code / 26) + 'A');
	(*buf)[1] = (char) (code % 26 + 'A');
	(*buf)[2] = '\0';

	return buf;
}

char *trimWhiteSpace(char *line){
	char *ptr = line;
	while(*ptr == ' '){
		ptr++;
	}
	return ptr;
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
		perror("Opendir");
		exit(EXIT_FAILURE);
	}

	return dir;
}

struct dirent *Readdir(DIR *dirp, struct dirent **ent){
	int tmp_errno = errno; // save errno
	*ent = readdir (dirp);
	if(ent == NULL && tmp_errno != errno){
		perror("Part1.c");
		exit(EXIT_FAILURE); // TODO : bad idea?
	}
	return *ent;
}

int Closedir(DIR **pdir){
	if(closedir(*pdir) != -1){
		return 0;
	} else {
		perror("Closedir");
		exit(EXIT_FAILURE);
	}

	return -1;
}

size_t usleep_time = 1000;
void Fread_r(struct map_res **res, FILE *fp){
	P(&mutex);
	readcnt++;
	if(readcnt == 1){
		P(&w); // if anyone is reading, hold write mutex
		debug("Hold write mutex\n");
	}
	V(&mutex);

	P(&line);
	int local_line_cnt = linecnt;
	debug("local_line_cnt=%lu\n", linecnt);
	V(&line);

	while(local_line_cnt <= 0){ // while nothing to read...
		// release write mutex and wait
		// debug("Release write mutex\n");
		V(&w);
		// debug("Sleep for %lu\n", usleep_time);
		usleep(usleep_time);

		// update the local_line_cnt
		P(&line);
		local_line_cnt = linecnt;
		V(&line);
	}

	P(&w); // now take the writing mutex

	set_struct(res, fp); // read from file

	P(&line);
	linecnt--; // update the linecnt
	V(&line);

	P(&mutex);
	readcnt--;
	if(readcnt == 0){
		V(&w); // if no one is reading anymore, release write mutex
		debug("Release write mutex\n");
	}
	V(&mutex);
}

void Fwrite_r(struct map_res *res, FILE *fp){
	P(&w);

	fprintf_struct(res, fp);
	fflush(fp);

	P(&line);
	linecnt++;
	debug("New Linecnt=%lu\n", linecnt);
	V(&line);
	
	V(&w);
}

/*For Part4*/
void Read_struct_r(struct map_res **res){
	P(&items);
	P(&mutex);

	// read from front of the buffer
	*res = buf[++start % BUF_SIZE];

	V(&mutex);
	V(&slots);
}

/*For Part4*/
void Write_struct_r(struct map_res *res){
	P(&slots); // wait for available slot
	P(&mutex); // lock to write
	
	// write to end of buffer
	write_to_buf(buf[++end % BUF_SIZE], res);
	
	V(&mutex); // allow others to write
	V(&items); // update the # of items
}

void write_to_buf(struct map_res* buf, struct map_res *res){
	debug("res->filename=%s\n", res->filename);
	buf->datum_cnt = res->datum_cnt;
	buf->tot_duration = res->tot_duration;
	buf->unique_years = res->unique_years;

	// TODO: Do I really need this?
	size_t filename_len = sizeof(char) * strlen(res->filename) + 1;
	buf->filename = (char*) malloc(filename_len);
	strncpy(buf->filename, res->filename, filename_len);

	buf->max_cntry_code = res->max_cntry_code;
	buf->max_cntry_cnt = res->max_cntry_cnt;
}

int Sem_init(sem_t *sem, int pshared, unsigned int value){
	int ret = sem_init(sem, pshared, value);
	if(ret == -1){
		perror("Sem_init()");
		exit(EXIT_FAILURE);
	}

	return -1;
}

int P(sem_t* sem){
	int ret = sem_wait(sem);

	if(ret == -1){
		perror("P");
	}

	return ret;
}

int V(sem_t* sem){
	int ret = sem_post(sem);

	if(ret == -1){
		perror("V");
	}

	return ret;
}

int Pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg){
	int ret = pthread_create(thread, attr, start_routine, arg);
	if(ret){
		error("ERROR; return code from pthread_create() is %d\n", ret);
	}

	return ret;
}

int Pthread_setname(pthread_t thread, const char *name){
	int ret = pthread_setname_np(thread, name);

	if(ret != 0){
		errno = ret;
		perror("Pthread_setname");
	}

	return ret;
}

int Pthread_join(pthread_t thread, void **retval){
	int ret = pthread_join(thread, retval);

	if(ret){
		errno = ret;
		perror("Pthread_join");
	}

	return ret;
}

int Select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout){
	int ret = select(nfds, readfds, writefds, exceptfds, timeout);

	if(ret == -1){
		perror("select");
	}

	return ret;
}

int Socketpair(int domain, int type, int protocol, int sv[2]){
	int ret = socketpair(domain, type, protocol, sv);

	if(ret == -1){
		perror("Socketpair");
	}

	return ret;
}

// int Accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen){
// 	int ret = accept(sockfd, addr, addrlen);

// 	if(ret == -1){
// 		perror("");
// 	}

// 	return ret;
// }