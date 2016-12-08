#ifndef _MAP_REDUCE_H_
#define _MAP_REDUCE_H_

#include <stdio.h>
#include <string.h>

#include "list.h"
#define EPSILON 0.00001

struct map_res{
	char *filename;
	unsigned long datum_cnt;
	unsigned long tot_duration; // total duaration
	
	struct list *year_root; // deprecated
	unsigned long unique_years; // introduced at part 4

	struct list *cntry_root; // deprecated
	unsigned long max_cntry_cnt; // introduced at part 4
	int max_cntry_code; // introduced at part 4
};

void print_map_res(struct map_res *res);

struct map_res*set_cntry(struct map_res **res, FILE* fp, size_t n);
void fprintf_cntry(struct map_res *res, FILE* fp);

struct map_res*set_struct(struct map_res **res, FILE* fp);
void fprintf_struct(struct map_res *res, FILE* fp);

struct map_res*Read_struct(int fd, struct map_res **res);
int Write_struct(int fd, struct map_res *res);

#endif