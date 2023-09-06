#include "threadPool.h"
int tcpInit(char *ip, char *port, int * psockfd){
    *psockfd = socket(AF_INET,SOCK_STREAM,0);
    int arg = 1;
    int ret = setsockopt(*psockfd,SOL_SOCKET,SO_REUSEADDR,&arg,sizeof(arg));
    ERROR_CHECK(ret,-1,"setsockopt");
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(port));
    addr.sin_addr.s_addr = inet_addr(ip);
    ret = bind(*psockfd,(struct sockaddr *)&addr,sizeof(addr));
    ERROR_CHECK(ret,-1,"bind");
    listen(*psockfd,10);
    return 0;
}
