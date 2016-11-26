#include "lott.h"
#include "helper.h"
#include "map_reduce.h"
#include "list.h"

static void* map(void*);
static void* reduce(void*);
static int f_cnt;

int part1(){

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

	struct map_res *mr_a[f_cnt];
	pthread_t t[f_cnt];

	/* print all the files and directories within directory */
	// assume no recursive sub dirs
	Opendir(base_dir, &dir);
	for(int i = 0; i < f_cnt;){
		int tmp_errno = errno;
		ent = readdir (dir);
		if(ent == NULL && tmp_errno == errno){
			warn("Number of files differ: Not all of them are found!");
			break;
		} else if(ent == NULL && tmp_errno != errno){
			perror("Part1.c");
			exit(EXIT_FAILURE); // TODO : bad idea?
		}
		debug("%3d: \"%s\"\n", i, ent->d_name);

		if( strncmp(ent->d_name, ".", 1) == 0 || strncmp(ent->d_name, "..", 2) == 0){
			continue;
		}

		struct map_res *mr = (struct map_res *) malloc(sizeof(struct map_res));
		mr_a[i] = mr;
		mr->datum_cnt = 0;
		
		int filename_len = sizeof(ent->d_name) + base_dir_len;
		mr->filename = (char*) malloc(filename_len);
		memcpy(mr->filename, base_dir, base_dir_len);
		strncat(mr->filename, ent->d_name, sizeof(ent->d_name));

		mr->tot_duration = 0;
		mr->year_root = NULL;
		mr->cntry_root = NULL;

		Pthread_create(&t[i], NULL, map, mr);
		i++;
	}

	// after spawning all of the children, start joining them
	for (int i = 0; i< f_cnt; i++){
		debug("Joining %d\n", i);
		Pthread_join(t[i], (void **) &mr_a[i]);
		print_map_res(mr_a[i]);
	}

	reduce(mr_a);
	for (int i = 0; i< f_cnt; i++){
		free(mr_a[i]->filename);
		free(mr_a[i]);
	}

	Closedir (&dir);

	return 0;
}

static void* map(void* v){
	struct map_res *res = (struct map_res *) v;
	res->year_root = NULL;
	res->cntry_root = NULL;
	res->tot_duration = 0;
	res->datum_cnt = 0;

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

		while(*ptr != '\0')	if(*ptr++ == ',')	cnt++; // count number of comma

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

	for(int i = 0; i < f_cnt; i++){
		struct map_res *now = mr_a[i];
		debug("Reduce check for result %3d: %s\n", i, now->filename);
		print_map_res(now);

		double AB = (double)now->tot_duration / (double)now->datum_cnt;
		if(res_value[0] < AB){
			res_value[0] = AB;
			res[0] = now;
		} 

		if(res_value[1] > AB){
			res_value[1] = AB;
			res[1] = now;
		}

		struct list *y_now = now->year_root;
		if(y_now == NULL){
			warn("No year_root found!\n");
		} else {
			int dist_year = 0; // count number of distinct years
			while(y_now != NULL){
				dist_year++;
				y_now = y_now->next;
			}

			double CD = (double)now->datum_cnt/(double)dist_year;
			if(CD > res_value[2]){
				res_value[2] = CD;
				res[2] = now;
			} 

			if(CD < res_value[3]){
				res_value[3] = CD;
				res[3] = now;
			}
		}

		struct list *c_now = now->cntry_root;
		if(c_now == NULL){
			warn("No cntry_root found!\n");
		} else {
			while(c_now != NULL){
				if(res_value[4] < c_now->value){
					res_value[4] = c_now->value;
					res[4] = now;
				}
				c_now = c_now->next;
			}
		}
	}

	for(int i =0; i < 5 ; i++){
		debug("Part: %s\nQuery: %s\nResult: %.5f, %s\n", PART_STRINGS[i], QUERY_STRINGS[i], res_value[i], res[i]->filename);
	}

	/* Where result is variables you define */
	printf("Part: %s\nQuery: %s\nResult: %.5f, %s\n",
		PART_STRINGS[current_part], QUERY_STRINGS[current_query], 
		res_value[current_query], res[current_query]->filename);

	return NULL;
}