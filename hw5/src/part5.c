#include "lott.h"
#include "helper.h"
#include "list.h"
#include "map_reduce.h"
#include <stdint.h>

static void* map(void*);
static void* reduce(void*);

#define READ 0
#define WRITE 1

int maxfd, cnt;
sem_t mutex;
// socklen_t clientlen = sizeof(struct sockaddr_in);
// struct sockaddr_in clientaddr;
fd_set read_set, ready_set;
// fd_set read_set;

int fd[FD_SETSIZE][2];

int part5(size_t nthreads){
	pthread_t treduce, t[nthreads];
	printf(
		"Part: %s\n"
		"Query: %s\n",
		PART_STRINGS[current_part], QUERY_STRINGS[current_query]);
	
	FD_ZERO(&read_set);
	// int tmp_fd[2];
	// Socketpair(AF_UNIX, SOCK_STREAM, 0, tmp_fd);
	// listenfd = tmp_fd[READ];
	// FD_SET(listenfd, &read_set);
	for(int i = 0; i < FD_SETSIZE; i++){
		fd[i][READ] = -1;
		fd[i][WRITE] = -1;
		// maxfd = listenfd;
	}
	maxfd = -1;
	cnt = 0;

	Sem_init(&mutex, 0, 1);
	debug("Create thread starts\n");

	for(int i = 0; i < nthreads; i++){
		// create pair of sockets
		Socketpair(AF_UNIX, SOCK_STREAM, 0, fd[i]);

		// add READ fd to read_set and update maxfd
		FD_SET(fd[i][READ], &read_set);
		debug("Added FD=%d to read_set, WRITE is %d\n", fd[i][READ], fd[i][WRITE]);
		P(&mutex);
		maxfd = (maxfd < fd[i][READ])?fd[i][READ]:maxfd;
		cnt++;
		V(&mutex);
		
		debug("Socketpair returned %d, %d\n", fd[i][READ], fd[i][WRITE]);
		FD_SET(fd[i][READ], &read_set);
		debug("read_set FD=%d\n", fd[i][0]);
	}

	char thread_name[20];
	for(int i = 0; i < nthreads; i++){
		debug("Create thread %d\n", i);

		Pthread_create(&t[i], NULL, map, (void *) (intptr_t)i);
		sprintf(thread_name, "map %3d", i);
		Pthread_setname(t[i], thread_name);
	}

	Pthread_create(&treduce, NULL, reduce, (void*) (intptr_t)nthreads);
	Pthread_setname(treduce, "reduce");

	debug("thread creation done\n");

	for(int i = 0; i < nthreads; i++){
		Pthread_join(t[i], NULL);
	}
	Pthread_join(treduce, NULL);

	for(int i = 0; i < nthreads; i++){
		close(fd[i][READ]);
		close(fd[i][WRITE]);
	}

	return 0;
}

static void* map(void* v){
	unsigned int i = (uintptr_t) v;

	// write data to the fd
	char content[20];
	sprintf(content, "read%3d", fd[i][WRITE]);
	int ret = write(fd[i][WRITE], content, 8);
	if(ret == -1){
		perror("write inside map");
		warn("FD[%d][WRITE]=%d\n", i, fd[i][WRITE]);
	}
	
	// clean up
	debug("Write \"%s\" Done! Read with fd=%d\n", content, (int)fd[i][READ]);

	return NULL;
}

static void* reduce(void* v){
	char buf[4096];
	unsigned int nthreads = (uintptr_t) v;

	while(1){
		// select the fdd that is ready to be read
		ready_set = read_set;
		
		debug("Before calling Select maxfd=%d\n", maxfd);
		int ret = Select(maxfd + 1, &ready_set, NULL, NULL, NULL);
		debug("Select returned %d\n", ret);

		for(int i = 0; i < nthreads; i++){ // for every thread
			debug("FD_ISSET(%d)\n", fd[i][READ]);
			if(FD_ISSET(fd[i][READ], &ready_set)) { // check if it has something readable
				int ret2 = read(fd[i][READ], buf, 4096);
				if(ret2 == -1){
					perror("read inside reduce");
					warn("FD[%d][READ]=%d\n", i, fd[i][READ]);
				}
				debug("Buf is %s:%d\n", buf, ret2);
				
				FD_CLR(fd[i][READ], &read_set);
				debug("Clear fd[i][read]=%d\n", fd[i][READ]);
				P(&mutex);
				cnt--;
				V(&mutex);
			}
		}

		P(&mutex);
		int tmp_cnt = cnt;
		V(&mutex);
		if(tmp_cnt == 0){
			debug("Size is empty\n");
			break;
		}
	}
	return NULL;
}
