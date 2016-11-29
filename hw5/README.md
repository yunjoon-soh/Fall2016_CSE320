# Lord of the Threads

## Author: Yun Joon (Daniel) Soh
### ID: 108256259

## Part 1

### Data structure
struct map_res{
	char *filename;
	unsigned long datum_cnt;
	unsigned long tot_duration; // total duaration
	struct list *year_root;
	struct list *cntry_root; // country
};

* filename: name of the file, starts with "data/"
* datum_cnt: # of rows in a given file
* tot_duration: Sum of all the duration in a given file
* struct list: Just a linked list, that has both key and value.

### How main program gets the result?

1. map() returns a single map_res structure
2. Main program save it in map_res array
3. reduce() iterate the map_res array

Note. Number of files excluding . and .. is same as # of map_res. This value is kept in "static int f_cnt"
