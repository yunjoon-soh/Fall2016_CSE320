#include "lott.h"
#include "helper.h"
#include "map_reduce.h"
#include "list.h"

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

	char *base_dir = "./data/";
	int base_dir_len = 8;
	Opendir(base_dir, &dir);
	while ((ent = readdir (dir)) != NULL) {
		f_cnt++;
	}
	Closedir (&dir);

	struct map_res mr_a[f_cnt];
	pthread_t t[f_cnt];

	/* print all the files and directories within directory */
	// assume no recursive sub dirs
	Opendir(base_dir, &dir);
	for(int i = 0; i < f_cnt; i++){
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

		struct map_res *mr = &mr_a[i];
		mr->datum_cnt = 0;
		
		int filename_len = sizeof(ent->d_name) + base_dir_len;
		mr->filename = (char*) malloc(filename_len);
		memcpy(mr->filename, base_dir, base_dir_len);
		strncat(mr->filename, ent->d_name, sizeof(ent->d_name));

		mr->tot_duration = 0;
		mr->year_root = NULL;
		mr->cntry_root = NULL;

		Pthread_create(&t[i], NULL, map, mr);
	}

	// after spawning all of the children, start joining them
	for (int i = 0; i< f_cnt; i++){
		debug("Joining %d\n", i);
		Pthread_join(t[i], (void **) &mr_a[i]);
	}

	reduce(&mr_a);
	for (int i = 0; i< f_cnt; i++){
		free(mr_a[i].filename);
	}

	Closedir (&dir);

	return 0;
}

static void* map(void* v){
	struct map_res *res = (struct map_res *) v;
	res->year_root = NULL;
	res->cntry_root = NULL;
	res->tot_duration = 0;

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
		localtime_r( (time_t*) &buf[0], &lt);

		// year root
		add(&res->year_root, lt.tm_year + 1900, 1);

		res->tot_duration += atoi(buf[2]);
	}

	return (void*)res;
}

static void* reduce(void* v){
	debug("reduce(%p)\n", v);
	return NULL;
}
