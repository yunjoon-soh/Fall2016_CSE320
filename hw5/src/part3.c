#include "lott.h"
#include "helper.h"
#include "list.h"
#include "map_reduce.h"

static void* map(void*);
static void* reduce(void*);
static size_t f_cnt;
static size_t per_thread;

void* map_part1_3(void* v);
size_t readcnt, linecnt;
sem_t mutex, w, line;
char *tmp_fname = "mapred.tmp";
FILE* tmp_f_w;

int part3(size_t nthreads){
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

	//assign per thread
	per_thread = f_cnt/nthreads;
	per_thread = (f_cnt % nthreads == 0)?per_thread:per_thread+1;
	size_t upto = (f_cnt < nthreads)?f_cnt:nthreads;

	debug("per_thread=%lu upto=%lu\n", per_thread, upto);

	char *fnames[f_cnt];

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
		memcpy(fnames[i], base_dir, base_dir_len);
		strncat(fnames[i], ent->d_name, sizeof(ent->d_name));

		i++; // put it here, because we don't want to increase i, even when ent->d_name is . or ..
	}

	// Now we have all the filenames on heap, ref to it on stack
	pthread_t t[nthreads];
	pthread_t treduce;

	debug("Initialize semaphores\n");
	Sem_init(&mutex, 0, 1);
	Sem_init(&w, 0, 1);
	Sem_init(&line, 0, 1);
	readcnt = 0;
	linecnt = 0;
	
	debug("Opening temp file caleed: %s\n", tmp_fname);
	Fopen(tmp_fname, "w", &tmp_f_w);

	debug("Before creating threads\n");
	for (int i = 0; i < upto; i++){
		debug("Create thread %d: %lu\n", i, per_thread * i);
		// pass on the ptr to first filename
		Pthread_create(&t[i], NULL, map, &fnames[per_thread * i]);
	}

	debug("Reduce thread starts\n");
	FILE* tmp_f_r;
	Fopen(tmp_fname, "r", &tmp_f_r);
	Pthread_create(&treduce, NULL, reduce, tmp_f_r);
		
	// after spawning all of the children, start joining them
	for (int i = 0; i < upto; i++){
		debug("Joining %d\n", i);
		// struct map_res **mr = &mr_a[per_thread * i];
		// thread write the result to the tmp file
		Pthread_join(t[i], NULL);
	}

	Pthread_join(treduce, NULL);

	// for (int i = 0; i< per_thread * nthreads; i++){
		// print_map_res(mr_a[i]);
		// free(mr_a[i]->filename);
		// freeAll(&(mr_a[i]->year_root));
		// freeAll(&(mr_a[i]->cntry_root));
		// free(mr_a[i]);
	// }
	for(int i = 0; i < f_cnt; i++){
		free(fnames[i]);
	}

	unlink(tmp_fname);
	Closedir (&dir);
	fclose(tmp_f_r);
	fclose(tmp_f_w);
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	return 0;
}


static void* map(void* v){
	char **filenames = (char **) v;
	struct map_res *mr = (struct map_res *) malloc(sizeof(struct map_res));

	for(int i = 0; i < per_thread; i++){
		mr->filename = filenames[i];
		debug("mr[%d]: %s\n", i, mr->filename);
		map_part1_3(mr);
		Fwrite_r(mr, tmp_f_w);
		if(mr->cntry_root != NULL)
			freeAll(&mr->cntry_root);
		if(mr->year_root != NULL)
			freeAll(&mr->year_root);
	}

	free(mr);

	return NULL;
}

void* map_part1_3(void* v){
	struct map_res *res = (struct map_res *) v;
	res->year_root = NULL;
	res->cntry_root = NULL;
	res->tot_duration = 0;
	res->datum_cnt = 0;

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
		// debug("Retrieved line of length %zu : %s\n", read, line);

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

	fclose(fp);
	free(line);

	return (void*)res;
}

static void* reduce(void* v){
	FILE *fp = (FILE *) v;

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
	struct map_res *keep_track[f_cnt];
	for(int i = 0; i < f_cnt; i++){
		struct map_res *now = NULL;

		Fread_r(&now, fp);
		keep_track[i] = now;

		debug("Reduce check for result %3d: %s\n", i, now->filename);
		// print_map_res(now);

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

		struct list *c_now = now->cntry_root;
		if(c_now == NULL){
			warn("No cntry_root found!\n");
		} else {
			while(c_now != NULL){
				add(&cntry_based, c_now->key, c_now->value);
				c_now = c_now->next;
			}
		}
	}

	struct list *c_now = cntry_based;
	int cntry_code;
	if(c_now == NULL){
		warn("No cntry_root found!\n");
	} else {
		while(c_now != NULL){
			if(res_value[4] < c_now->value){
				cntry_code = c_now->key;
				res_value[4] = c_now->value;
			}
			c_now = c_now->next;
		}
	}
	freeAll(&cntry_based);

	for(int i =0; i < 4; i++){
		printf("Result: %.5f, %s\n", res_value[i], res[i]);
	}

	char *buf = (char*) malloc(sizeof(char) * 2 + 1); // + 1 for null term
	printf("Result: %.5f, %s\n", res_value[4], *(cntry_code_reverter(cntry_code, &buf)));
	free(buf);

	/* Where result is variables you define */
	// printf("Part: %s\nQuery: %s\nResult: %.5f, %s\n",
	//  PART_STRINGS[current_part], QUERY_STRINGS[current_query], 
	//  res_value[current_query], res[current_query]->filename);

	// clean up
	for(int i = 0; i < f_cnt; i++){
		struct map_res *now = keep_track[i];
		free(now->filename);
		freeAll(&now->cntry_root);
		free(now);
	}

	return NULL;
}