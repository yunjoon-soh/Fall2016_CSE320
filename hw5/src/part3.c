#include "lott.h"
#include "helper.h"
#include "list.h"
#include "map_reduce.h"

static void* map(void*);
static void* reduce(void*);
static size_t f_cnt;
static size_t per_thread;

size_t readcnt, linecnt;
sem_t mutex, w, line;
char *tmp_fname = "mapred.tmp";
FILE* tmp_f_w; /* FILE* opened with 'w' */

int part3(size_t nthreads){
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
	size_t upto = (f_cnt < nthreads)?f_cnt:nthreads; /* just in case there are more threads than files*/


	// 3. Prepare filenames
	char *fnames[f_cnt]; /* pointer for file names */

	// Note. assume no recursive sub dirs
	Opendir(base_dir, &dir);
	for(int i = 0; i < f_cnt; ){ // For every file in the dir...
		ent = Readdir (dir, &ent);

		// skip . and ..
		if( strncmp(ent->d_name, ".", 1) == 0 || strncmp(ent->d_name, "..", 2) == 0){
			continue;
		}
		
		// set the filename with data/FILENAME
		int filename_len = sizeof(ent->d_name) + base_dir_len + 1;
		fnames[i] = (char*) malloc(filename_len);
		memcpy(fnames[i], base_dir, base_dir_len); // filenmae is now ./data/
		strncat(fnames[i], ent->d_name, sizeof(ent->d_name)); // append filename

		i++; // put it here, because we don't want to increase i, even when ent->d_name is . or ..
	}
	// Now we have all the filenames on heap

	// 4. Prepare threads and semaphores
	pthread_t t[nthreads], treduce;

	Sem_init(&mutex, 0, 1); /*For locking the readcnt*/
	Sem_init(&w, 0, 1); /*For locking the writing action*/
	Sem_init(&line, 0, 1); /*For locking the linecnt*/
	readcnt = 0; /* # of readers */
	linecnt = 0; /* # of newly readable item cnts*/
	
	Fopen(tmp_fname, "w", &tmp_f_w);

	// 5. Spawn "upto" # of threads, each call map()
	char thread_name[20];
	for (int i = 0; i < upto; i++){
		// pass on the ptr to first filename
		Pthread_create(&t[i], NULL, map, &fnames[per_thread * i]);
		sprintf(thread_name, "map %d", i);
		Pthread_setname(t[i], thread_name);
	}

	// 6. Spawn "reduce" thread
	FILE* tmp_f_r;
	Fopen(tmp_fname, "r", &tmp_f_r);
	Pthread_create(&treduce, NULL, reduce, tmp_f_r);
	Pthread_setname(treduce, "reduce");

	// 7. Join threads
	for (int i = 0; i < upto; i++){
		Pthread_join(t[i], NULL); // thread write the result to the tmp file
	}
	Pthread_join(treduce, NULL);

	// 8. Clean up
	for(int i = 0; i < f_cnt; i++){
		free(fnames[i]);
	}
	Closedir (&dir);
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
	fclose(tmp_f_r);
	fclose(tmp_f_w);
	unlink(tmp_fname); // remove temp file

	return 0;
}


static void* map(void* v){
	char **filenames = (char **) v;

	struct map_res *mr = (struct map_res *) malloc(sizeof(struct map_res));
	for(int i = 0; i < per_thread; i++){
		mr->filename = filenames[i]; // reference to heap

		map_part1(mr); // same as part 1's map()

		Fwrite_r(mr, tmp_f_w); // write to temp file

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
	FILE *fp = (FILE *) v;

	// 1. Init result values
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

	struct list *cntry_based = NULL; /*Sum of cnt per country*/
	struct map_res *keep_track[f_cnt]; /* Inorder to clean up latter*/

	// 2. Iterate mr_a and calculate values
	for(int i = 0; i < f_cnt; i++){ // for every map_res...
		struct map_res *now = NULL;

		// 2-1. Read from tmp file
		Fread_r(&now, fp); 
		keep_track[i] = now;

		// 2-2. Set max, min for query A/B
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

		// 2-3. Set max, min for query C/D
		long dist_year = (long) now->year_root;
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

		// 2-4. Find max cnt per cntry and append it to final result
		struct list *c_now = now->cntry_root;
		if(c_now == NULL){
			warn("No cntry_root found!\n");
		} else {
			int max_key = -1, max_cnt = -1;
			while(c_now != NULL){
				if(c_now->value > max_cnt){
					max_key = c_now->key;
					max_cnt = c_now->value;
				} else if(c_now->value == max_cnt){
					max_key = (c_now->key < max_key)?c_now->key:max_key;
				}
				c_now = c_now->next;
			}
			add(&cntry_based, max_key, max_cnt);
		}
	}

	// 3. Find country with max count
	struct list *c_now = cntry_based;
	int cntry_code;
	if(c_now == NULL){
		warn("No cntry_root found!\n");
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
	for(int i = 0; i < f_cnt; i++){
		struct map_res *now = keep_track[i];
		free(now->filename);
		freeAll(&now->cntry_root);
		free(now);
	}

	return NULL;
}