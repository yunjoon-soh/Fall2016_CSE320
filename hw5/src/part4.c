#include "lott.h"
#include "helper.h"
#include "list.h"
#include "map_reduce.h"

static void* map(void*);
static void* reduce(void*);
static size_t f_cnt;
static size_t per_thread, nt;

void* map_part1_4(void* v);
size_t start, end; /* Used in Read_struct_r */
struct map_res *buf[BUF_SIZE];
sem_t mutex, slots, items;

int part4(size_t nthreads){
	nt = nthreads;
	printf(
		"Part: %s\n"
		"Query: %s\n",
		PART_STRINGS[current_part], QUERY_STRINGS[current_query]);

	DIR *dir = NULL; /*DIR pointer to data/ folder*/
	struct dirent *ent; /*variable for reading the data/ folder*/
	f_cnt = 0; /*# of files in data/ folder */
	const char *base_dir = "./data/";
	const int base_dir_len = 8; // 7 chars + 1 null term

	// 1. Count the number of files
	Opendir(base_dir, &dir);
	while ((ent = Readdir (dir, &ent)) != NULL) {
		if( strncmp(ent->d_name, ".", 1) == 0 || strncmp(ent->d_name, "..", 2) == 0){// skip . and ..
			continue;
		}
		f_cnt++;
	}
	Closedir (&dir);

	// 2. Count the number of files per thread
	per_thread = f_cnt/nthreads;
	// if the remainder is not 0, +1 for per thread; invalid files will be dummy files and ignored inside threads
	per_thread = (f_cnt % nthreads == 0)?per_thread:per_thread+1;
	// take the min of two values, if there were smaller number of files than the nthreads, we don't need to generate all the threads.
	// e.g., passing 10 as nthreads when there are only 5 files in ./data/
	size_t upto = (f_cnt < nthreads)?f_cnt:nthreads;

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
	// Now we have all the filenames on heap

	// 4. Prepare shared buffer
	// Initialize the buffers
	for(int i = 0; i < BUF_SIZE; i++){
		buf[i] = (struct map_res *) malloc(sizeof(struct map_res));
		buf[i]->year_root = NULL;
		buf[i]->cntry_root = NULL;
	}

	// 5. Prepare threads and semaphores
	pthread_t t[nthreads], treduce;

	Sem_init(&mutex, 0, 1); /*For locking the access*/
	Sem_init(&slots, 0, BUF_SIZE); /*For counting # of free slots in buf*/
	Sem_init(&items, 0, 0); /*For counting # of items in buf*/

	// 6. Spawn "upto" # of threads, each call map()
	char thread_name[20];
	for (int i = 0; i < upto; i++){ // 
		// debug("Create thread %d: %lu\n", i, per_thread * i);
		// pass on the ptr to first filename
		Pthread_create(&t[i], NULL, map, &fnames[per_thread * i]);
		sprintf(thread_name, "map %d", i);
		Pthread_setname(t[i], thread_name);
	}

	// 7. Spawn "reduce" thread
	Pthread_create(&treduce, NULL, reduce, NULL);
	Pthread_setname(treduce, "reduce");

	// 8. Join threads
	for (int i = 0; i < upto; i++){
		Pthread_join(t[i], NULL); // thread write the result to the tmp file
	}
	Pthread_join(treduce, NULL);

	// 9. Clean up
	for(int i = 0; i < BUF_SIZE; i++){
		free(buf[i]);
	}
	for(int i = 0; i < per_thread * nthreads; i++){
		free(fnames[i]);
	}
	Closedir (&dir);
	// close(STDIN_FILENO);
	// close(STDOUT_FILENO);
	// close(STDERR_FILENO);

	return 0;
}

static void* map(void* v){
	char **filenames = (char **) v;

	struct map_res *mr = (struct map_res *) malloc(sizeof(struct map_res));
	for(int i = 0; i < per_thread; i++){
		mr->filename = filenames[i]; // reference to heap

		if(strcmp(mr->filename, "") == 0)
			continue;

		map_part1(mr); // same as part 1's map()

		Write_struct_r(mr); // write to buf only when possible
		debug("mr[%3d]: %s\n", i, mr->filename);

		// clean up
		if(mr->cntry_root != NULL)
			freeAll(&mr->cntry_root);
		if(mr->year_root != NULL)
			freeAll(&mr->year_root);
	}
	free(mr);

	return NULL;
}

static void* reduce(void* v){
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

	struct map_res *now = NULL;
	for(int i = 0; i < f_cnt; i++){

		// read from the buffer
		Read_struct_r(&now);

		// 2-1. Set max, min for query A/B
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

		// 2-2. Set max, min for query C/D
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

		// 2-3. Find max cnt per cntry and append it to final result
		add(&cntry_based, now->max_cntry_code, now->max_cntry_cnt);
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
			printf("Result: %.5f, %s\n", res_value[i], res[i] + 7);
		}
		printf("Result: %d, %s\n", (int)res_value[4], *(cntry_code_reverter(cntry_code, &buf)));
		free(buf);
	#endif

	// 4. Print out the final result according to the query.
	if(current_query != 4){
		printf("Result: %.5f, %s\n", res_value[current_query], res[current_query] + 7);
	} else if(current_query == 4){
		buf = (char*) malloc(sizeof(char) * 2 + 1); // + 1 for null term
		printf("Result: %d, %s\n", (int)res_value[4], *(cntry_code_reverter(cntry_code, &buf)));
		free(buf);
	}

	// 5. Clean up
	freeAll(&cntry_based);

	return NULL;
}
