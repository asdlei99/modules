/*
 * 
 * @file      server.c
 * @brief     to get process quantity form file recorded
 *             
 * @author    yangf
 * @date      2015-12-11
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
 * @func 	void *thread_fun(void* arg)
 * @brief	get process num and send it to client
 *
 * @param       arg - client sockfd
 * 
 * @return  
 *
 */


void* thread_fun(void* arg)
{
    char buff[10];   /*buff for process num*/  
    int  len;
    if(pthread_detach(pthread_self()))
    {
        printf("thread detach fail\n");
        exit(1);
    }
    int file_fd = open("temp.txt",O_RDONLY); 
    if(file_fd == -1)
    {
        printf("open file fail\n");
        exit(1);
    }

    bzero(buff,10);			
    len = read(file_fd,buff,10);
    if(len == -1)
    {
        printf("read fail\n");
        exit(1);
    }
    if(write((int)arg,buff,len) == -1)
    {
        printf("write fail\n");
        exit(1);
    }

    return (void*)0;
}




int main()
{
    pthread_t id;
    struct sockaddr_in servaddr,clientaddr;
    socklen_t addlen;
    int clifd;
    int err;
    addlen = sizeof(struct sockaddr_in);

    /*creat tcp option*/	
    int sockfd = socket(AF_INET,SOCK_STREAM,0);
    if(sockfd == -1)
    {
        printf("socket\n");
        exit(1);
    }
    int opt = SO_REUSEADDR;	/*address reuse*/
    setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));

    /*initialization of socket()*/
    memset(&servaddr,0,addlen);
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(8080);
    bind(sockfd,(struct sockaddr*)&servaddr,sizeof(servaddr));
	
    /*waiting for client*/
    while(1)
    {
        listen(sockfd,10);
        clifd = accept(sockfd,(struct sockaddr*)&clientaddr,&addlen);	
        if(clifd>=0)
        {
            printf("new client\n");	
            pthread_create(&id,NULL,thread_fun,(void*)clifd);
        }	
    }

    psuse();	/*sleep,waiting for any signal*/
    close(clifd);
    close(sockfd);
    return 0;
}
