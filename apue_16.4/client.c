/*
 * 
 * @file        client.c
 * @brief       connect server to get process num and make output
 *             
 * @author      yangf
 * @date        2015-12-21
 *
 */


#include<arpa/inet.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netinet/in.h>
#include<string.h>
#include<pthread.h>
#include<fcntl.h>


/**
 * @func        func
 * @brief       get process num and optput
 *
 * @param       arg - sockfd
 * 
 * @return
 *
 */

void* fun(void *arg)
{
    char buff[10];
    memset(buff,0,10);
    int err = read((int)arg,buff,BUFSIZ);
    if(err<0)	
    {
        printf("read error\n");
        exit(1);
    }
    else
    {
        printf("current process : %s",buff);
    }

    exit(0);
}

/**
 * @func        function name
 * @brief       function description
 *
 * @param       argv[1] - server's IP
 * 
 * @return  
 *
 */


int main(int arg,char *argv[])
{
	if(!argv[1])
	{
        argv[1]="127.0.0.1";
	}
	
    pthread_t id;
    struct sockaddr_in sock;
    int clifd;
    int err;
	
    int sockfd = socket(AF_INET,SOCK_STREAM,0);
    if(sockfd == -1)
    {
        perror("socket");
        exit(1);
    }

    /*initialization of socket()*/
    memset(&sock,0,sizeof(struct sockaddr_in));
    sock.sin_family = AF_INET;
    sock.sin_addr.s_addr = inet_addr(argv[1]);    /*get server's IP from input*/
    sock.sin_port = htons(8080);                  /*must be server's port*/

    err = connect(sockfd,(struct sockaddr*)&sock,sizeof(sock));
    if(err == -1)
    {
        perror("connect fail\n");
        exit(1);
    }
    pthread_create(&id,NULL,fun,(void *)sockfd);
		
    pause();
    close(sockfd);
    return 0;

}

