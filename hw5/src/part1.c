#include "lott.h"
#include "helper.h"
#include "map_reduce.h"
#include "tree_func.h"

static void* map(void*);
static void* reduce(void*);

int part1(){

	printf(
		"Part: %s\n"
		"Query: %s\n",
		PART_STRINGS[current_part], QUERY_STRINGS[current_query]);

	DIR *dir = NULL;
	struct dirent *ent;
	int f_cnt = 0;

	Opendir("../data", dir);
	while ((ent = readdir (dir)) != NULL) {
		f_cnt++;
	}
	Closedir (dir);

	struct map_res mr_a[f_cnt];
	pthread_t t[f_cnt];

	/* print all the files and directories within directory */
	// assume no recursive sub dirs
	Opendir("../data", dir);
	for(int i = 0; i < f_cnt; i++){
		debug("%3d: %s\n", i, ent->d_name);

		int tmp_errno = errno;
		ent = readdir (dir);
		if(ent == NULL && tmp_errno == errno){
			warn("Number of files differ: Not all of them are found!");
			break;
		} else if(ent == NULL && tmp_errno != errno){
			perror("");
			exit(EXIT_FAILURE); // TODO : bad idea?
		}

		struct map_res *mr = &mr_a[i];
		mr->datum_cnt = 0;
		
		int filename_len = sizeof(ent->d_name);
		mr->filename = (char*) malloc(filename_len);
		memcpy(mr->filename, ent->d_name, filename_len);

		mr->tot_duration = 0;
		mr->year_root = NULL;
		mr->cntry_root = NULL;

		Pthread_create(&t[i], NULL, map, mr);
	}

	// after spawning all of the children, start joining them
	for (int i = 0; i< f_cnt; i++){
		Pthread_join(t[i], (void **) &mr_a[i]);
	}

	reduce(&mr_a);

	Closedir (dir);

	return 0;
}

static void* map(void* v){
	struct map_res *res = (struct map_res *) v;
	res->year_root = NULL;
	res->cntry_root = NULL;
	res->tot_duration = 0;

	// read per line
	FILE *fp = NULL;
	Fopen(res->filename, "r", fp);

	char *line = NULL;
	size_t len = 0;
	ssize_t read;

	while ((read = getline(&line, &len, fp)) != -1) {
		debug("Retrieved line of length %zu : %s\n", read, line);

		int cnt = 0;
		char *ptr = line;

		while(*ptr != '\0')	if(*ptr == ',')	cnt++; // count number of comma

		char *buf[cnt];
		splitByComma(line, buf, cnt);

		// country root
		add(res->cntry_root, cntry_code_converter(buf[3]), 1);
		
		struct tm lt;
		localtime_r( (time_t*) &buf[0], &lt);

		// year root
		add(res->year_root, lt.tm_year + 1900, 1);

		res->tot_duration += atoi(buf[2]);
	}

	return (void*)res;
}

static void* reduce(void* v){
	debug("reduce(%p)\n", v);
	return NULL;
}
