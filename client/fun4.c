#include "clientHead.h"
void *threadFuncGets(void *arg)
{
    char *p=(char *)arg;
    char ip[]=IP;
    char port[]=PORT;
    int sockfd = socket(AF_INET,SOCK_STREAM,0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port=htons(atoi(port));
    addr.sin_addr.s_addr=inet_addr(ip);
    int ret=connect(sockfd,(struct sockaddr *)&addr,sizeof(addr));
    ERROR_CHECK(ret,-1,"connect");
    char *command=(char *)calloc(1,(strlen(p)+1)*sizeof(char));
    strcpy(command,p);
    getClient(sockfd,command);
    close(sockfd);
    return  NULL;
}
void *threadFuncPuts(void *arg)
{
    char *p=(char *)arg;
    char ip[]=IP;
    char port[]=PORT;
    int sockfd = socket(AF_INET,SOCK_STREAM,0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port=htons(atoi(port));
    addr.sin_addr.s_addr=inet_addr(ip);
    int ret=connect(sockfd,(struct sockaddr *)&addr,sizeof(addr));
    ERROR_CHECK(ret,-1,"connect");
    char *command=(char *)calloc(1,(strlen(p)+1)*sizeof(char));
    strcpy(command,p);
    putsClient(command,sockfd);
    close(sockfd);
    return NULL;
}
int longConnectGetClient(char *command)
{
    pthread_t thread;
    pthread_create(&thread,NULL,threadFuncGets,(void *)command);
    pthread_join(thread, NULL);
    return 0;
}
int longConnectPutsClient(char *command)
{
    pthread_t thread;
    pthread_create(&thread,NULL,threadFuncPuts,(void *)command);
    pthread_join(thread, NULL);
    return 0;
}
