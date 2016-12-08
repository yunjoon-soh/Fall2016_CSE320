# Lord of the Threads

## Author: Yun Joon (Daniel) Soh
### ID: 108256259
### Used 2 late days/grace days

## Part 1

### Data structure
Note. More fields are added for later parts. Only the ones that are used in this part is explained

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

Note. Number of files excluding . and .. is same as # of map_res. This value is kept in "static size_t f_cnt"

### Program Flow
1. Count the number of files
2. Spawn f_cnt # of threads, each call map()
3. Join threads
4. reduce()
5. Clean up

#### map() work flow
* Argument
Argument is *struct map_res\** with its filename set.
* Work flow
1. Initialize *struct map_res*
2. Fopen the assigned file
3. Read the file per line
  1. Count number of comma, to check invalid input
  2. Split by comma
  3. Add year value
  4. Add country value
  5. Sum up the total duration
  6. Increment the user cnt
4. Clean up

#### reduce() work flow
* Argument
Argument is *struct map_res\*\** with all of the values calculated.
* Work flow
1. Init result values
  * Note. result is stored in two arrays: *struct map_res\*res[]* and *double[]*
    * *struct map_res\*[]* stores the reference to *struct map_res* that stores the corresponding query result at any moment.
    * *doubel[]* stores the value of correspnding query result at any moment.
    * e.g., if at any point of reduce() if *res[0]=0x400* and *double[0]=10* then it means the *struct map_res* that contains "Max average duration per website" is stored at 0x400 and the vale of it is 10.
  * Note. For query E, I create a key-value list called *struct list \*cntry_based*. This list keeps track of all the map()\'s return value for query E. Key is the name of the country encoded in decimal, treating char[2] as a number in base-26(treat character A as 0 and Z as 26).
2. Iterate mr_a and calculate values
  1. Set max, min for query A/B
  2. Set max, min for query C/D
  3. Find max cnt per cntry and append it to final result
3. Find country with max count
  * Note. This is finding the maximum country in the cntry_based list.
4. Print out the final result according to the query.
  * Note. if DEBUG is set, whole result will be printed out, before the printing of query result.
5. Clean up

## Part 2
### Data structure
Exactly same as part1.

### Program Flow
1. Count the number of files
2. Count the number of files per thread
3. Prepare map_res array: f_cnt of them have name, others are dummy
4. Spawn "upto" # of threads, each call map()
5. Join threads
6. reduce()
7. Clean up

#### map() work flow
* Argument
Subset of mr_a (i * per_thread ... (i+1) * per_thread - 1) inclusive.
* Work flow
map() for part2 is just a wrapper for per_thread calls to map_part1

#### reduce() work flow
Exactly same as part1.

## Part 3
### Basic concurrency related variable 
All global variables

*size_t linecnt, running_threads;*
* linecnt: # of new lines in tmp file, if linecnt==0, usleep the reduce thread
* running_threads: # of running threads, if reached 0 and linecnt is 0, reduce breaks the while loop
*sem_t mutex, w, line;*
* mutex: used to lock *running_threads*
* w    : used to lock read/write to the tmp file
* lien : used to lock *linecnt*

Note. For details of concurrency related issues check *Fread_r(..)* and *Fwrite_r(..)* defined in *helper.c*

### Data structure
Exactly same as part1.

### Program Flow
1. Count the number of files
2. Count the number of files per thread
3. Prepare filenames
4. Prepare threads and semaphores
5. Spawn "upto" # of threads, each call map()
6. Spawn "reduce" thread
7. Join threads
8. Clean up

#### map() work flow
* Argument
Subset of filenames (i * per_thread ... (i+1) * per_thread - 1) inclusive.
* Work flow
map() for part3 is just a wrapper for per_thread calls to map_part1.
To call map_part1_3(..), it creates a temp *struct map_res* and set the filename and pass on the struct to map_part1_3(..). It then writes the result of *Fwrite_r(..)* which is defined in *helper.c*.

#### reduce() work flow
* Argument
Pointer to a tmp file
* Work flow
Pretty much same as part1 and part2 except that it now reference to the filename instead of the whole *struct map_res*.

1. Init result values
2. Iterate mr_a and calculate values
  1. *Fread_r(..)* from tmp file
  2. Set max, min for query A/B
  3. Set max, min for query C/D
  4. Find max cnt per cntry and append it to final result
3. Find country with max count
  * Note. This is finding the maximum country in the cntry_based list.
4. Print out the final result according to the query.
  * Note. if DEBUG is set, whole result will be printed out, before the printing of query result.
5. Clean up

## Part 4
### Data structure
Newly introduced field in Part 4

* *unsigned long unique_years;* : Number of unique years
* *unsigned long max_cntry_cnt;*: Number of users for country with most users
* *int max_cntry_code;*         : Country code for country with most users

struct map_res{
	char *filename;
	unsigned long datum_cnt;
	unsigned long tot_duration; // total duaration
	struct list \*year_root; // deprecated: for compatibility reason (part 1,2,3)
	unsigned long unique_years; // introduced at part 4
	struct list \*cntry_root; // deprecated: for compatibility reason (part 1,2,3)
	unsigned long max_cntry_cnt; // introduced at part 4
	int max_cntry_code; // introduced at part 4
};

### Program Flow
1. Count the number of files
2. Count the number of files per thread
3. Prepare filenames
4. Prepare shared buffer
5. Prepare threads and semaphores
6. Spawn "upto" # of threads, each call map()
7. Spawn "reduce" thread
8. Join threads
9. Clean up

### map() workflow
* Argument
Subset of filenames (i * per_thread ... (i+1) * per_thread - 1) inclusive.
* Work flow
Also a wrapper function that calls map_part1() internally. Main difference is that it uses Fwrite_r to print out the result to "mapred.tmp" file and as it enters it increment the number of running threads and decrease the value as it terminates.

### reduce() workflow
* Argument
*FILE \** to "mapred.tmp" with "r" permission is given
* Work flow
Pretty much same as other reduce parts. Only difference is that it uses *Fread_r(..)* to read from the temp file.

## Part 5
### Data Structure
Same as Part 4.

Newly added structure.

struct thread_info{
	char **fname;
	int i;
};

From my design I needed a way to pass both the pointer to file names and the thread number. So I created a *struct thread_info* to do the job.

### Program Flow
1. Count the number of files
2. Count the number of files per thread
3. Prepare filenames
3-2. Fill in with dummy filenames
4. Prepare threads, semaphores, sockets
5. Spawn nthreads # of threads
6. Spawn "reduce" thread
7. Join threads
8. Clean up

### map() workflow
Pretty much same as previous maps, except that it use *Write_struct(..)* to write the result to a socket. Also, write before the termination of a *map()* it writes a EOF mark to the socket. This is necessary for reduce() to get notified and clear the associated file descriptor from the FD_SET.

### reduce() workflow
Also similar to previous *reduce()*. Inside the main while loop, it waits for one of the buffers to be available for reading. When it is alerted that there exists a file descriptor that is ready to read from, it checks which one it was from and consume the data. When EOF mark is encountered, which is alerted by *Read_struct(..)* returning NULL value, it removes the associated file descriptor from FD_SET and continue.

## Other Helper Functions and Structures

### struct list (list.c)
#### Structure
struct list{
	struct list *next;
	int key;
	int value;
};

This is just a one-way linked list where each node contains key, value pair. No two nodes with same key allowed.

#### Helper functions

size_t count_list(struct list *head);
void add(struct list **head, int key, int value);
struct list*createNode(int key, int value);
struct list*find(struct list *head, int key);
void freeAll(struct list **head);
struct list *find_max(struct list *head);

Most of them are pretty self-explanatory.

##### add(..)
This method first find a node for matching key. If there is one, add up the parameter value to existing node's value. Else, append the new node to the front of the list.

##### createNode(..)/freeAll(..)
Internally malloc space for new node. The whole list will be freed using freeAll.

##### find(..)
Return a node with the key. If not matched with any of the nodes, return NULL.

##### find_max(..)
Iterate the whole list and return the node with the max value. In case of tie, return a node with smaller key value.

### struct map_reduce (map_reduce.c)
#### Structure
struct map_res{
	char *filename;
	unsigned long datum_cnt;
	unsigned long tot_duration; // total duaration
	struct list *year_root; // deprecated: for compatibility reason (part 1,2,3)
	unsigned long unique_years; // introduced at part 4
	struct list *cntry_root; // deprecated: for compatibility reason (part 1,2,3)
	unsigned long max_cntry_cnt; // introduced at part 4
	int max_cntry_code; // introduced at part 4
};

#### Helper functions
*set_\*()* functions are reader functions, that read in corresponding output formatted by *fprintf_\*()* from the *FILE\** and sets the corresponding values in a malloc'ed *struct map_res* and return the address of it.

*Read_struct()* and *Write_struct()* are similar to other functions except that they read/write from/to file descriptors and the format is a little different from other functions.

*print_map_res(..)* literally print out the result to stdout. Need to have DEBUG defined to see the result.

### helper.c

Functions in this file can be classified into two big categories: Wrapper and Helper. Wrapper functions are literally error handled wrappers, called *perror(..)* internally when necessary. 

#### Helper functions

##### cntry_code_converter(..) cntry_code_reverter
These pair of functions encode and decode the 2 character long country code. Mapping AA to integer 0 and ZZ to 25 * 26 + 25. Input checking is not fully done, therefore, really have to be careful when using those.

##### map_part1()
This is explained in detail earlier so skipping here.

##### Read_struct_r() Write_struct_r()
For part 4.
These pair of functions are not just a wrapper but has locks for concurrent program.

##### Fread_r() Fwrite_r() write_to_buf()
For part 5. Followed echo server example from textbook.
These pair of functions are not just a wrapper but has locks for concurrent program.

write_to_buf() copy the argument *res* to argument *buf* except the *year_root* and *cntry_root*. This is why using those two fields in the struct is deprecated.

Fwrite_r() is a wrapper for write_to_buf() with locks.

Fread_r() also contains locks.