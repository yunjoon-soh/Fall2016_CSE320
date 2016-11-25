#ifndef _MAP_REDUCE_H_
#define _MAP_REDUCE_H_

struct map_res{
	size_t datum_cnt;
	char *filename;
	unsigned long tot_duration; // total duaration
	struct list *year_root;
	struct list *cntry_root; // country based
};

#endif