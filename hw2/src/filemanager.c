#include "filemanager.h"

int openToRead(char* filename){
	int ret;
	ret = open(filename, O_RDONLY);
	if(ret == NO_FD){
		fprintf(stderr, "%s: %s(%s)\n", "File not opened to read: ", strerror(errno), filename);
		return NO_FD;
	}
	else{
		return ret;
	}
}

int openToWrite(char* filename, endianness target){
	int ret, readFd, retFd;
	struct stat statbuf;
	endianness currentFileEnd;

	if(filename == NULL){
		writeBom(STDOUT_FILENO, target);
		return STDOUT_FILENO;
	}

	ret = stat(filename, &statbuf);

	if(ret < 0 && errno == ENOENT){
		/* if file does not exists go for create mode */
		retFd = openToCreate(filename);
		if(retFd == NO_FD){
			writeBom(STDOUT_FILENO, target);
			return STDOUT_FILENO;
		}
		else{
			writeBom(retFd, target);
			return retFd;
		}
	}
	else if(ret < 0){ /* if stat failed for some reason*/
		fprintf(stderr, "%s: %s\n", "Outfile verification failed... changing to default stdout", strerror(errno));
		writeBom(STDOUT_FILENO, target);
		return STDOUT_FILENO;
	}
	else { /* if file does exists go for append mode */
		readFd = openToRead(filename);
		if(readFd == NO_FD){
			fprintf(stderr, "Cannot open existing file... Aborting\n");
			writeBom(STDOUT_FILENO, target);
			return NO_FD;
		}

		currentFileEnd = checkBom(readFd);
		if(currentFileEnd == NO_BOM){
			fprintf(stderr, "Cannot append to existing file... Existing file has no BOM... changing to default stdout\n");
			writeBom(STDOUT_FILENO, target);
			return STDOUT_FILENO;
		}

		close(readFd);

		if(currentFileEnd != target){
			fprintf(stderr, "Cannot append to existing file... Targetted encoding: %d, existing file encoding: %d... changing to default stdout\n", target, currentFileEnd);
			writeBom(STDOUT_FILENO, target);
			return STDOUT_FILENO;
		}

		retFd = openToAppend(filename);

		if(retFd == NO_FD){
			writeBom(STDOUT_FILENO, target);
			return STDOUT_FILENO;
		}
		else{ /* File exists and successfully opens with append mode*/
			unsigned char newLine = '\n';
			unsigned char na = 0;
			switch(target){
				case BIG:
					write(retFd, &na,  1);
					write(retFd, &newLine,  1);
					break;
				case LITTLE:
					write(retFd, &newLine,  1);
					write(retFd, &na,  1);
					break;
				default:
					break;
			}
			
			return retFd;
		}
	}
}

int openToCreate(char* filename){
	int ret;
	ret = open(filename, O_WRONLY|O_CREAT);
	if(ret == NO_FD){
		fprintf(stderr, "%s: %s\n", "File not opened to create: ", strerror(errno));
		return NO_FD;
	}
	else{
		return ret;
	}
}

int openToAppend(char* filename){
	int ret;
	ret = open(filename, O_WRONLY|O_APPEND);
	if(ret == NO_FD){
		fprintf(stderr, "%s: %s\n", "File not opened to append: ", strerror(errno));
		return NO_FD;
	}
	else{
		return ret;
	}
}