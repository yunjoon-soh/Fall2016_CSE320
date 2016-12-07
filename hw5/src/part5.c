#include "lott.h"
#include "helper.h"
#include "list.h"
#include "map_reduce.h"
#include <stdint.h>

static void* map(void*);
static void* reduce(void*);
void* map_part1_5(void* v);

#define READ 0
#define WRITE 1

static size_t f_cnt;
static size_t per_thread;
int maxfd, cnt;
fd_set read_set, ready_set;
sem_t mutex;

int fd[FD_SETSIZE][2];

struct thread_info{
	char **fname;
	int i;
};

int part5(size_t nthreads){
	printf(
		"Part: %s\n"
		"Query: %s\n",
		PART_STRINGS[current_part], QUERY_STRINGS[current_query]);

	DIR *dir = NULL; /*DIR pointer to data/ folder*/
	struct dirent *ent; /*variable for reading the data/ folder*/
	f_cnt = 0; /*# of files in data/ folder */
	const char *base_dir = "./data/";
	int base_dir_len = 8; // 7 chars + 1 null term

	// 1. Count the number of files
	Opendir(base_dir, &dir);
	while ((ent = Readdir (dir, &ent)) != NULL) {
		if( strncmp(ent->d_name, ".", 1) == 0 || strncmp(ent->d_name, "..", 2) == 0){// skip . and ..
			continue;
		}
		f_cnt++;
	}
	Closedir (&dir);

	// assign per thread
	per_thread = f_cnt/nthreads;
	// if the remainder is not 0, +1 for per thread; invalid files will be dummy files and ignored inside threads
	per_thread = (f_cnt % nthreads == 0)?per_thread:per_thread+1;
	// take the min of two values, if there were smaller number of files than the nthreads, we don't need to generate all the threads.
	// e.g., passing 10 as nthreads when there are only 5 files in ./data/

	// 3. Prepare filenames
	char *fnames[per_thread * nthreads];

	// Note. assume no recursive sub dirs
	Opendir(base_dir, &dir);
	for(int i = 0; i < f_cnt; ){ // For every file in the dir...
		ent = Readdir (dir, &ent);

		if( strncmp(ent->d_name, ".", 1) == 0 || strncmp(ent->d_name, "..", 2) == 0){// skip . and ..
			continue;
		}
		
		// set the filename with data/FILENAME
		int filename_len = sizeof(ent->d_name) + base_dir_len + 1;
		fnames[i] = (char*) malloc(filename_len);
		memcpy(fnames[i], base_dir, base_dir_len); // filenmae is now ./data/
		strncat(fnames[i], ent->d_name, sizeof(ent->d_name)); // append filename

		i++; // put it here, because we don't want to increase i, even when ent->d_name is . or ..
	}

	// 3-2. Fill in with dummy filenames
	for(int i = f_cnt; i < per_thread * nthreads; i++){		
		fnames[i] = (char*) malloc(1);
		fnames[i][0] = '\0';
	}

	// Now we have all the filenames on heap, ref to it on stack
	pthread_t t[nthreads];
	pthread_t treduce;

	debug("Initialize semaphores\n");	
	Sem_init(&mutex, 0, 1); /*For locking the access to maxfd and cnt*/
	
	FD_ZERO(&read_set);
	maxfd = -1;
	cnt = 0;

	for(int i = 0; i < nthreads; i++){
		// create pair of sockets
		Socketpair(AF_UNIX, SOCK_STREAM, 0, fd[i]);

		// add READ fd to read_set and update maxfd
		FD_SET(fd[i][READ], &read_set);
		
		P(&mutex);
		maxfd = (maxfd < fd[i][READ])?fd[i][READ]:maxfd;
		cnt++;
		V(&mutex);
		
		FD_SET(fd[i][READ], &read_set);
	}

	struct thread_info tinfo[nthreads];
	char thread_name[20];
	for(int i = 0; i < nthreads; i++){
		tinfo[i].fname = (char**) malloc(per_thread * sizeof(char*));
		tinfo[i].i = i;

		for(int j = 0; j < per_thread; j++){
			tinfo[i].fname[j] = fnames[i * per_thread + j];
		}

		Pthread_create(&t[i], NULL, map, (void *) &tinfo[i]);
		sprintf(thread_name, "map %3d", i);
		Pthread_setname(t[i], thread_name);
	}

	Pthread_create(&treduce, NULL, reduce, (void*) (intptr_t)nthreads);
	Pthread_setname(treduce, "reduce");

	for(int i = 0; i < nthreads; i++){
		Pthread_join(t[i], NULL);
	}
	Pthread_join(treduce, NULL);

	for(int i = 0; i < nthreads; i++){
		close(fd[i][READ]);
		close(fd[i][WRITE]);
	}

	return 0;
}

static void* map(void* v){

	struct thread_info *tinfo = (struct thread_info*) v;
	debug("map(tinfo->i=%d, tinfo->fname=%s)\n", tinfo->i, tinfo->fname[0]);

	int i = tinfo->i;
	char **filenames = &tinfo->fname[0];

	struct map_res *mr = (struct map_res *) malloc(sizeof(struct map_res));

	for(int j = 0; j < per_thread; j++){
		mr->filename = filenames[j]; // reference to heap

		if(strcmp(mr->filename, "") == 0)
			continue;

		debug("thread[%2d]:file[%3d]: %s\n", i, j, mr->filename);
		map_part1(mr); // same as part 1's map()

		// write data to the fd
		Write_struct(fd[i][WRITE], mr);
		
		// clean up
		if(mr->cntry_root != NULL)
			freeAll(&mr->cntry_root);
		if(mr->year_root != NULL)
			freeAll(&mr->year_root);
	}

	Write_struct(fd[i][WRITE], NULL); // write EOF mark

	free(mr);	
	
	// clean up
	debug("Write Done! Read with fd=%d\n", (int)fd[i][READ]);

	return NULL;
}

static void* reduce(void* v){
	if(v == NULL)
		return NULL;

	unsigned int nthreads = (uintptr_t) v;
	// initialize values of results
	char *res[5];
	res[0] = "";
	res[1] = "";
	res[2] = "";
	res[3] = "";
	res[4] = "";
	double res_value[5];
	// Max/Min average duration of all of the websites
	res_value[0] = 0; // max by default is 0
	res_value[1] = 2147483647L; // min by default largest possible value

	// Max/Min average between all sites
	res_value[2] = 0; 
	res_value[3] = 2147483647L;

	// Find country with the most users
	res_value[4] = 0;

	struct list *cntry_based = NULL;

	struct map_res *now = (struct map_res *) malloc(sizeof(struct map_res));
	while(1){
		// select the fdd that is ready to be read
		ready_set = read_set;
		
		// debug("Before calling Select maxfd=%d\n", maxfd);
		// int ret = Select(maxfd + 1, &ready_set, NULL, NULL, NULL);
		// debug("Select returned %d\n", ret);
		Select(maxfd + 1, &ready_set, NULL, NULL, NULL);

		for(int i = 0; i < nthreads; i++){ // for every thread
			if(FD_ISSET(fd[i][READ], &ready_set)) { // check if it has something readable

				// read from the buffer
				struct map_res* read_ret = Read_struct(fd[i][READ], &now);

				if(read_ret == NULL){ // if EOF mark is read
					FD_CLR(fd[i][READ], &read_set);
					P(&mutex);
					cnt--;
					V(&mutex);
					continue;
				}

				double AB = (double)now->tot_duration / (double)now->datum_cnt;
				if(AB > res_value[0]){ // max
					res_value[0] = AB;
					res[0] = now->filename;
				} else if( (AB - res_value[0]) < EPSILON && (AB - res_value[0]) > -1 * EPSILON ){
					res[0] = (strcmp(res[0], now->filename) < 0)?res[0]:now->filename;
				}

				if(AB < res_value[1]){ // min
					res_value[1] = AB;
					res[1] = now->filename;
				} else if( (AB - res_value[1]) < EPSILON && (AB - res_value[1]) > -1 * EPSILON ){
					res[1] = (strcmp(res[1], now->filename) < 0)?res[1]:now->filename;
				}

				long dist_year = (long) now->unique_years;
				if(dist_year != 0){
					double CD = (double)now->datum_cnt/(double)dist_year;
					if(CD > res_value[2]){ // max
						res_value[2] = CD;
						res[2] = now->filename;
					} else if((CD - res_value[2]) < EPSILON && (CD - res_value[2]) > -1 * EPSILON ){
						res[2] = (strcmp(res[2], now->filename) < 0)?res[2]:now->filename;
					}

					if(CD < res_value[3]){ // min
						res_value[3] = CD;
						res[3] = now->filename;
					} else if((CD - res_value[3]) < EPSILON && (CD - res_value[3]) > -1 * EPSILON ){
						res[3] = (strcmp(res[3], now->filename) < 0)?res[3]:now->filename;
					}
				}

				add(&cntry_based, now->max_cntry_code, now->max_cntry_cnt);
			}
		}
		

		P(&mutex);
		int tmp_cnt = cnt;
		V(&mutex);

		if(tmp_cnt == 0){
			break;
		}
	}

	struct list *c_now = cntry_based;
	int cntry_code;
	if(c_now == NULL){
		// warn("No cntry_root found!\n");
	} else {
		while(c_now != NULL){
			if(res_value[4] < c_now->value){
				cntry_code = c_now->key;
				res_value[4] = c_now->value;
			} else if(res_value[4] == c_now->value){
				cntry_code = (cntry_code > c_now->key)?c_now->key:cntry_code;
			}
			c_now = c_now->next;
		}
	}

	char *buf;
	#ifdef DEBUG
		// if DEBUG is defined, print the whole result
		buf = (char*) malloc(sizeof(char) * 2 + 1); // + 1 for null term
		for(int i =0; i < 4; i++){
			printf("Result: %.5f, %s\n", res_value[i], res[i]);
		}
		printf("Result: %.5f, %s\n", res_value[4], *(cntry_code_reverter(cntry_code, &buf)));
		free(buf);
	#endif

	// 4. Print out the final result according to the query.
	if(current_query != 4){
		printf("Result: %.5f, %s\n", res_value[current_query], res[current_query]);
	} else if(current_query == 4){
		buf = (char*) malloc(sizeof(char) * 2 + 1); // + 1 for null term
		printf("Result: %.5f, %s\n", res_value[4], *(cntry_code_reverter(cntry_code, &buf)));
		free(buf);
	}

	// 5. Clean up
	freeAll(&cntry_based);

	return NULL;
}
