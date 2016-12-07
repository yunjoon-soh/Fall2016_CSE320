// map_reduce.c
// funcs for io of struct map_res
#include "map_reduce.h"
#include "helper.h"

#ifndef DEBUG
#define DEBUG
#endif

void print_map_res(struct map_res *res){
	debug("%s: %lu(%p)\n", res->filename, res->datum_cnt, &res->datum_cnt);
	debug("tot_duration: %lu\n", res->tot_duration);
	debug("year_head:\n");
	struct list *now = res->year_root;
	while(now != NULL){
		debug("\t%12d: %12d\n", now->key, now->value);
		now = now->next;
	}
	debug("cntry_root:\n");
	now = res->cntry_root;
	while(now != NULL){
		debug("\t%12d: %12d\n", now->key, now->value);
		now = now->next;
	}
}

struct map_res*set_cntry(struct map_res **res, FILE* fp, size_t n){
	struct map_res *mr = *res;
	char *cntry_str = (char *) malloc(sizeof(char) * 2 + 1);
	int cntry_val;
	for(int i = 0; i < n; i++){
		fscanf(fp, "%s\n%d\n", cntry_str, &cntry_val);
		add(&mr->cntry_root, cntry_code_converter(cntry_str), cntry_val);
	}
	free(cntry_str);
	return mr;
}

void fprintf_cntry(struct map_res *res, FILE* fp){
	struct list *now = res->cntry_root;
	int cnt = 0;
	char *buf = (char *) malloc(sizeof(char) * 2 + 1); // +1 for size null term
	// printf("Alloced buf:%p\n", buf);
	buf[0] = 'F';
	buf[1] = 'F';
	buf[2] = 0;
	while(now != NULL){
		cnt++;
		cntry_code_reverter(now->key, &buf);
		fprintf(fp, "%s\n%d\n", buf, now->value);
		buf[0] = 0;
		buf[1] = 0;
		buf[2] = 0;
		now = now->next;
	}
	free(buf);
}

struct map_res*set_struct(struct map_res **res, FILE* fp){
	struct map_res *mr = (struct map_res *) malloc( sizeof(struct map_res));
	*res = mr;
	size_t year_cnt = 0, cntry_cnt = 0;
	mr->year_root = NULL;
	mr->cntry_root = NULL;
	char fname[4096];

	int ret = fscanf(fp, "%s\n%lu,%lu,%lu,%lu,", fname, 
		&mr->datum_cnt, &mr->tot_duration, &year_cnt, &cntry_cnt);
	mr->year_root = (struct list *) year_cnt;
	// int ret = fscanf(fp, "%s", &fname[0]);
	// debug("LINE(ret=%d): %s\n", ret, fname);
	if(ret == -1)
		perror("");

	size_t len = strlen(&fname[0]) * sizeof(char) + 1;
	mr->filename = (char*) malloc(len);
	strncpy(mr->filename, fname, len);

	// fprintf(stdout, "%s\n%lu,%lu,%lu,%lu,\n", mr->filename, 
		// mr->datum_cnt, mr->tot_duration, year_cnt, cntry_cnt);
	set_cntry(res, fp, cntry_cnt);

	return mr;
}

void fprintf_struct(struct map_res *res, FILE* fp){
	fprintf(fp, "%s\n%zu,%zu,%zu,%zu\n", res->filename,
		res->datum_cnt, res->tot_duration, count_list(res->year_root), count_list(res->cntry_root));
	fprintf_cntry(res, fp);
	fprintf(fp, "\n");
}

// return NULL on EOF mark
struct map_res*Read_struct(int fd, struct map_res **res){
	struct map_res *mr = (struct map_res *) malloc( sizeof(struct map_res));
	*res = mr;
	mr->year_root = NULL;
	mr->cntry_root = NULL;

	// read the line
	char line[410];
	int ret = read(fd, line, 410);
	if(ret == -1)
		perror("read inside read_struct");

	// split by comma
	int cnt = 0;
	char *ptr = line;
	while(*ptr != '\0') if(*ptr++ == ',')   cnt++; // count number of comma
	char *buf[cnt];
	splitByComma(line, buf, cnt);

	if(strcmp(trimWhiteSpace(buf[0]), "EOF") == 0){
		return NULL;
	}

	mr->datum_cnt = atoi(trimWhiteSpace(buf[1]));
	mr->tot_duration = atoi(trimWhiteSpace(buf[2]));
	mr->unique_years = atoi(trimWhiteSpace(buf[3]));
	mr->max_cntry_code = atoi(trimWhiteSpace(buf[4]));
	mr->max_cntry_cnt = atoi(trimWhiteSpace(buf[5]));
	
	char *trimmed_fname = trimWhiteSpace(buf[0]);
	size_t len = strlen(trimmed_fname) * sizeof(char) + 1;
	mr->filename = (char*) malloc(len);
	strncpy(mr->filename, trimmed_fname, len);

	return mr;
}

int Write_struct(int fd, struct map_res *res){
	char line[410];
	if(res == NULL){
		sprintf(line, "%300s,%20d,%20d,%20d,%20d,%20d\n", "EOF",
			0, 0, 0, 0, 0);
	} else {
		struct list *max_cntry = find_max(res->cntry_root);
		sprintf(line, "%300s,%20zu,%20zu,%20zu,%20d,%20d\n", res->filename,
		res->datum_cnt, res->tot_duration, count_list(res->year_root), max_cntry->key, max_cntry->value);
	}
	
	int ret = write(fd, line, 410);
	if(ret == -1){
		perror("write inside write_struct");
		warn("FD=%d\n", fd);
	}

	// printf("%s\n", line);

	return ret;
}