/*
	Chad Coates
	ECE373
	Homework #3
	April 18, 2017

	This is the reader/writer for the acme_pci driver
*/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int acme_error(int);
int reader(int,char *);
int writer(int,uint32_t,char *);

int main(int argc, char *argv[]){
	char *file="/dev/acme";
	int val,fd=open(file,O_RDWR);
	
	if(fd<0){
		fprintf(stderr,"open failed: %s\n",file);
		exit(1);
	}

	switch(argc){
		case 1:
			reader(fd,file);
			break;
		case 2:
			val=atoi(argv[argc-1]);
			switch(val){
				case 0:val=0x0F;break;
				case 1:val=0x4E;break;
				default: return acme_error(fd);		 
			}
			fprintf(stderr,"**0x%08x**\n",val);
			return writer(fd,val,file);
		default: return acme_error(fd);		 
	}

	close(fd);
	exit(0);
}

int acme_error(int fd){
	fprintf(stderr,"Error, cowardly refusing to proceed!\n");
	close(fd);
	exit(1);
}

int reader(int fd,char *file){
	uint32_t val;
	if(read(fd,&val,sizeof(uint32_t))<0){
		fprintf(stderr,"read failed!: %s\n",file);
		exit(1);
	}

	printf("acme: 0x%08x\n",val);

	return 0;
}

int writer(int fd,uint32_t val,char *file){
	if(write(fd,&val,sizeof(int))<0){
		fprintf(stderr,"write failure!: %s\n",file);
		exit(1);
	}
	fprintf(stderr,"write success!: %s\n",file);
	return 0;
}


