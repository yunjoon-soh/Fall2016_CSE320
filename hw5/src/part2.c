#include "lott.h"
#include "helper.h"
#include "list.h"
#include "map_reduce.h"

static void* map(void*);
static void* reduce(void*);
static size_t f_cnt;
static size_t per_thread;

void* map_part1(void* v);

int part2(size_t nthreads){
	printf(
		"Part: %s\n"
		"Query: %s\n",
		PART_STRINGS[current_part], QUERY_STRINGS[current_query]);

	DIR *dir = NULL;
	struct dirent *ent;
	f_cnt = 0;

	char *base_dir = "./data/";
	int base_dir_len = 8;
	Opendir(base_dir, &dir);
	while ((ent = readdir (dir)) != NULL) {
		if( strncmp(ent->d_name, ".", 1) == 0 || strncmp(ent->d_name, "..", 2) == 0){
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

	struct map_res *mr_a[per_thread * nthreads];
	pthread_t t[nthreads];

	/* print all the files and directories within directory */
	// assume no recursive sub dirs
	Opendir(base_dir, &dir);
	for(int i = 0; i < f_cnt; ){
		int tmp_errno = errno;
		ent = readdir (dir);
		if(ent == NULL && tmp_errno == errno){
			warn("Number of files differ: Not all of them are found!");
			break;
		} else if(ent == NULL && tmp_errno != errno){
			perror("Part1.c");
			exit(EXIT_FAILURE); // TODO : bad idea?
		}

		if( strncmp(ent->d_name, ".", 1) == 0 || strncmp(ent->d_name, "..", 2) == 0){
			continue;
		}

		debug("%3d: \"%s\"\n", i, ent->d_name);

		struct map_res *mr = (struct map_res *) malloc(sizeof(struct map_res));
		mr_a[i] = mr;
		
		int filename_len = sizeof(ent->d_name) + base_dir_len;
		mr->filename = (char*) malloc(filename_len);
		memcpy(mr->filename, base_dir, base_dir_len);
		strncat(mr->filename, ent->d_name, sizeof(ent->d_name));

		i++;
	}

	for(int i = f_cnt; i < per_thread * nthreads; i++){
		struct map_res *mr = (struct map_res *) malloc(sizeof(struct map_res));
		mr_a[i] = mr;
		
		mr->filename = (char*) malloc(1);
		mr->filename[0] = '\0';
	}

	debug("Before creating threads\n");

	for (int i = 0; i < upto; i++){
		debug("Create thread %d: %lu\n", i, per_thread * i);
		struct map_res **mr = &mr_a[per_thread * i];
		Pthread_create(&t[i], NULL, map, mr);
	}
		
	// after spawning all of the children, start joining them
	for (int i = 0; i < upto; i++){
		debug("Joining %d\n", i);
		struct map_res **mr = &mr_a[per_thread * i];
		Pthread_join(t[i], (void **) &mr);
	}

	reduce(mr_a);
	// reduce(NULL);

	for (int i = 0; i< per_thread * nthreads; i++){
		print_map_res(mr_a[i]);
		free(mr_a[i]->filename);
		freeAll(&(mr_a[i]->year_root));
		freeAll(&(mr_a[i]->cntry_root));

		free(mr_a[i]);
	}

	Closedir (&dir);

	return 0;
}

static void* map(void* v){
	struct map_res **mr_a = (struct map_res **) v;

	for(int i = 0; i < per_thread; i++){
		debug("mr_a[%d]=%p\n", i, mr_a[i]);
		print_map_res(mr_a[i]);
		map_part1(mr_a[i]);
	}

	return mr_a;
}

void* map_part1(void* v){
	// Init map_res
	struct map_res *res = (struct map_res *) v;
	res->year_root = NULL;
	res->cntry_root = NULL;
	res->tot_duration = 0;
	res->datum_cnt = 0;

	// fopen the file
	FILE *fp = NULL;
	Fopen(res->filename, "r", &fp);

	// read per line
	size_t len = 4096;
	ssize_t read;
	char *line = (char*) malloc(4096 * sizeof(char));	

	while ((read = getline(&line, &len, fp)) != -1) {
		// debug("Retrieved line of length %zu : %s\n", read, line);
		
		// 1. count number of comma, to prepare for invalid input
		int cnt = 0; /*# of comma*/
		char *ptr = line;
		while(*ptr != '\0')	if(*ptr++ == ',') cnt++;

		if(cnt != 3){
			error("Unexpected number of comma(=%d), skip %s\n", cnt, res->filename);
			break;
		}

		// 2. split by comma
		char *buf[cnt];
		splitByComma(line, buf, cnt);

		// 3. Add year value
		struct tm lt;
		time_t t_val = (time_t) strtol(buf[0], NULL, 10);
		localtime_r( &t_val, &lt);
		add(&res->year_root, lt.tm_year + 1900, 1);

		// 4. Add country value
		add(&res->cntry_root, cntry_code_converter(buf[3]), 1);
		
		// 5. Sum up the total duration
		res->tot_duration += atoi(buf[2]);

		// 6. Add up the user cnt;
		res->datum_cnt += 1;
	}

	fclose(fp);
	free(line);

	return (void*)res;
}


static void* reduce(void* v){
	struct map_res **mr_a = (struct map_res**) v;

	struct map_res *res[5];
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
	for(int i = 0; i < f_cnt; i++){ // for every map_res...
		struct map_res *now = mr_a[i];
		debug("Reduce check for result %3d: %s\n", i, now->filename);
		print_map_res(now);

		// 1. Set max, min for query A/B
		double AB = (double)now->tot_duration / (double)now->datum_cnt;
		if(AB > res_value[0]){ // max
			res_value[0] = AB;
			res[0] = now;
		} else if( (AB - res_value[0]) < EPSILON && (AB - res_value[0]) > -1 * EPSILON ){
			res[0] = (strcmp(res[0]->filename, now->filename) < 0)?res[0]:now;
		}

		if(AB < res_value[1]){ // min
			res_value[1] = AB;
			res[1] = now;
		} else if( (AB - res_value[1]) < EPSILON && (AB - res_value[1]) > -1 * EPSILON ){
			res[1] = (strcmp(res[1]->filename, now->filename) < 0)?res[1]:now;
		}

		struct list *y_now = now->year_root;
		if(y_now == NULL){
			warn("No year_root found!\n");
		} else {
			// count number of distinct years
			size_t dist_year = count_list(y_now); 

			// 2. Set max, min for query C/D
			double CD = (double)now->datum_cnt/(double)dist_year;
			if(CD > res_value[2]){ // max
				res_value[2] = CD;
				res[2] = now;
			} else if((CD - res_value[2]) < EPSILON && (CD - res_value[2]) > -1 * EPSILON ){
				res[2] = (strcmp(res[2]->filename, now->filename) < 0)?res[2]:now;
			}

			if(CD < res_value[3]){ // min
				res_value[3] = CD;
				res[3] = now;
			} else if((CD - res_value[3]) < EPSILON && (CD - res_value[3]) > -1 * EPSILON ){
				res[3] = (strcmp(res[3]->filename, now->filename) < 0)?res[3]:now;
			}
		}

		// 3. Add up the cnt per cntry
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

	// 4. Find cntry with max cnt
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

	// 5. Clean up
	freeAll(&cntry_based);

	// 6. Print out the debugging result
	for(int i =0; i < 4; i++){
		printf("Result: %.5f, %s\n", res_value[i], res[i]->filename);
	}

	char *buf = (char*) malloc(sizeof(char) * 2 + 1); // + 1 for null term
	printf("Result: %.5f, %s\n", res_value[4], *(cntry_code_reverter(cntry_code, &buf)));
	free(buf);

	// 7. Print out the actual query result
	/* Where result is variables you define */
	// printf("Part: %s\nQuery: %s\nResult: %.5f, %s\n",
	// 	PART_STRINGS[current_part], QUERY_STRINGS[current_query], 
	// 	res_value[current_query], res[current_query]->filename);

	return NULL;
}