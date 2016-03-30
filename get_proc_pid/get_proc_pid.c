/*
 * 
 * @file        get_proc_pid.c
 * @brief       get process pid from process name
 *             
 * @author      feng.yang
 * @date        2016-03-16
 *
 */
#include<stdio.h>
#include<string.h>
#include <dirent.h>
#include <stdlib.h>

int main(int argc, char* argv[]){
	if(argc < 2){
		printf("expected proc name\nusage:\t%s [proc name]\n",argv[0]);
		return -1;
	}
	if(strcmp(argv[1],"--help") == 0){
		printf("usage:\t%s [proc name]\n",argv[0]);
		return 0;
	}

	#define READ_BUF_SIZE 64
	DIR* dir;
	struct dirent* next;
	FILE* status;
	char file_name[READ_BUF_SIZE];
	char proc_name[READ_BUF_SIZE];
	dir = opendir("/proc");
	if(!dir){
		//printf("open error /proc\n");
		return -1;
	}
	while((next = readdir(dir)) != NULL){
		if(!isdigit(*(next->d_name)))
			continue;

		memset(file_name,0,READ_BUF_SIZE);
		memset(proc_name,0,READ_BUF_SIZE);
		sprintf(file_name,"/proc/%s/comm",next->d_name);
		if(!(status = fopen(file_name,"r"))){
			//printf("open error %s",file_name);
			continue;
		}
		fscanf(status, "%s\n",proc_name);
		if(strcmp(argv[1],proc_name) == 0){
			printf("%d",atoi(next->d_name));
			fclose(status);
			return atoi(next->d_name);
		}
	}
	//printf("no such a process named %s\n",argv[1]);
	fclose(status);
	return 0;
}

