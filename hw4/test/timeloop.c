#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX 1
int main(){
	int cnt = 0;
	while(1){
		fprintf(stdout, "Hello(%d)\n", cnt++);
		sleep(1);
		if(cnt > MAX){
			break;
		}
	}
}