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

void* map_part1(void* v){
	struct map_res *res = (struct map_res *) v;
	res->year_root = NULL;
	res->cntry_root = NULL;
	res->tot_duration = 0;
	res->datum_cnt = 0;
	res->unique_years = 0;
	res->max_cntry_code = 0;
	res->max_cntry_cnt = 0;

	if(strncmp(res->filename, "\0", 1) == 0){ // dummy input
		return v;
	}

	// read per line
	FILE *fp = NULL;
	Fopen(res->filename, "r", &fp);

	size_t len = 4096;
	char *line = (char*) malloc(4096 * sizeof(char));   
	ssize_t read;

	while ((read = getline(&line, &len, fp)) != -1) {
		int cnt = 0;
		char *ptr = line;

		while(*ptr != '\0') if(*ptr++ == ',')   cnt++; // count number of comma

		char *buf[cnt];
		splitByComma(line, buf, cnt);

		// country root
		add(&res->cntry_root, cntry_code_converter(buf[3]), 1);
		
		struct tm lt;
		time_t t_val = (time_t) strtol(buf[0], NULL, 10);
		localtime_r( &t_val, &lt);

		// year root
		add(&res->year_root, lt.tm_year + 1900, 1);

		res->tot_duration += atoi(buf[2]);
		res->datum_cnt += 1;
	}

	struct list *head = res->year_root;
	while(head != NULL){
		res->unique_years++;
		head = head->next;
	}

	head = res->cntry_root;
	while(head != NULL){
		if(res->max_cntry_cnt < head->value){ // if value is larger
			res->max_cntry_cnt = head->value;
			res->max_cntry_code = head->key;
		} else if(res->max_cntry_cnt == head->value){ // tie break
			res->max_cntry_code = (res->max_cntry_code < head->key)?res->max_cntry_code:head->key;			
		}
		head = head->next;
	}

	// clean up
	fclose(fp);
	free(line);

	return (void*)res;
}

/*
* Wrapper
*/
FILE **Fopen(const char *path, const char *mode, FILE **fp){
	*fp = fopen(path, mode);
	if(*fp == NULL){
		perror("Fopen");
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

size_t usleep_time = 1000, hasWLock;
void Fread_r(struct map_res **res, FILE *fp){
	// save the linecnt
	P(&line);
	int local_line_cnt = linecnt;
	V(&line);

	while(local_line_cnt <= 0){ // while nothing to read...
		
		if(hasWLock == 1){
			hasWLock = 0;
			V(&w); // release write mutex and wait
		}

		usleep(usleep_time);

		// update the local_line_cnt
		P(&line);
		local_line_cnt = linecnt;
		V(&line);
	}

	if(hasWLock == 0){
		hasWLock = 1;
		P(&w); // if anyone is reading, hold write mutex
	}

	set_struct(res, fp); // read from file

	P(&line);
	linecnt--; // update the linecnt
	V(&line);
}

void Fwrite_r(struct map_res *res, FILE *fp){
	P(&w);

	fprintf_struct(res, fp);
	fflush(fp);

	P(&line);
	linecnt++;
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
	buf->datum_cnt = res->datum_cnt;
	buf->tot_duration = res->tot_duration;
	buf->unique_years = res->unique_years;
	buf->filename = res->filename;
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