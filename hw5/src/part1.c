#include "lott.h"
#include "helper.h"
#include "list.h"
#include "map_reduce.h"

static void* map(void*);
static void* reduce(void*);
static size_t f_cnt; /* # of files in data/ */

int part1(){
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

	// 2. Spawn f_cnt # of threads
	struct map_res *mr_a[f_cnt]; /* array of map_res */
	pthread_t t[f_cnt]; /* threads */
	char thread_name[20];

	// Note. assume no recursive sub dirs
	Opendir(base_dir, &dir);
	for(int i = 0; i < f_cnt; ){ // For every file in the dir...
		ent = Readdir (dir, &ent);

		// skip . and .. files
		if( strncmp(ent->d_name, ".", 1) == 0 || strncmp(ent->d_name, "..", 2) == 0){
			continue;
		}

		// prepare map_res struct
		struct map_res *mr = (struct map_res *) malloc(sizeof(struct map_res));
		mr_a[i] = mr;
		
		// set the filename with data/FILENAME
		int filename_len = sizeof(ent->d_name) + base_dir_len;
		mr->filename = (char*) malloc(filename_len);
		memcpy(mr->filename, base_dir, base_dir_len);
		strncat(mr->filename, ent->d_name, sizeof(ent->d_name));

		// create thread
		Pthread_create(&t[i], NULL, map, mr);
		sprintf(thread_name, "map %d", i);
		Pthread_setname(t[i], thread_name);
		i++; // put it here, because we don't want to increase i, even when ent->d_name is . or ..
	}

	// 3. Joining threads
	for (int i = 0; i< f_cnt; i++){
		Pthread_join(t[i], (void **) &mr_a[i]);
	}

	// 4. After join finishes, call reduce
	reduce(mr_a);

	// 5. Clean up
	for (int i = 0; i< f_cnt; i++){
		free(mr_a[i]->filename);
		freeAll(&(mr_a[i]->year_root));
		freeAll(&(mr_a[i]->cntry_root));

		free(mr_a[i]);
	}
	Closedir (&dir);
	// close(STDIN_FILENO);
	// close(STDOUT_FILENO);
	// close(STDERR_FILENO);

	return 0;
}

static void* map(void* v){
	return map_part1(v);
}

static void* reduce(void* v){
	struct map_res **mr_a = (struct map_res**) v;

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

	// 2. Iterate mr_a and calculate values
	for(int i = 0; i < f_cnt; i++){ // for every map_res...
		struct map_res *now = mr_a[i];

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

	// 3. Find cntry with max cnt
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