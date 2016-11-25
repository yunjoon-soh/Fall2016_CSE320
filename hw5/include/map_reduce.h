#ifndef _MAP_REDUCE_H_
#define _MAP_REDUCE_H_

struct map_res{
	size_t datum_cnt;
	char *filename;
	unsigned long tot_duration; // total duaration
	struct tree_root *year_root;
	struct tree_root *cntry_root; // country based
};

#endif