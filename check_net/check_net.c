/*
 * 
 * @file     check_net.c
 * @brief    check net state
 *				./check_net		./check_net wlan0	 
 *             
 * @author   feng.yang
 * @date     20160315
 *
 */

#include<stdio.h>
#include<string.h>
#include <sys/types.h>
#include<dirent.h>
int check_net(char* argv){
	DIR *dir;
	FILE *file;
	struct dirent *next;
	int state;
	char file_name[32] = "/sys/class/net";	
	if(strcmp(argv,"all") == 0){
		dir = opendir(file_name);
		if(!dir){
			printf("Cannot open /sys/class/net\n");
			return 0;
		}
		while((next = readdir(dir)) != NULL){
			if ((strcmp(next->d_name, ".") == 0)  ||
				(strcmp(next->d_name, "..") == 0) ||
				(strcmp(next->d_name, "lo") == 0) ||
				(strcmp(next->d_name, "sit0") == 0) )
				continue;
			
			memset(file_name,0,32);
			strcat(file_name,"/sys/class/net");
			strcat(file_name,"/");
			strcat(file_name,next->d_name);
			strcat(file_name,"/carrier");
			
			if (!(file = fopen(file_name, "r"))){
				printf("open error: %s\n",file_name);
				continue;
			}
			else{
				fscanf(file, "%d",&state);
				if(state == 1){
					printf("%s %d\n",next->d_name,state);
					return state;
				}
			}
		}
	}
	else{
		if(strcmp(argv,"--help") == 0){
			printf("usage:\n\targv[0]\t\t---check all network cards state\nor:\targv[0] [arguments]\t---check network card state\narguments:\n\twlan0,eth0,etc..\n");
			return 0;
		}
		strcat(file_name,"/");
		strcat(file_name,argv);
		strcat(file_name,"/carrier");
		if (!(file = fopen(file_name, "r"))){
			printf("there is no connect named %s\n",argv);
			return 0;
		}
		else{
			fscanf(file, "%d",&state);
			printf("%s %d\n",argv,state);
			return state;
		}	
	}
	
	return 0;
}

int main(int argc, char* argv[]){
	if(argc < 2)
		return check_net("all");
	 else
		return check_net(argv[1]);
	
}
