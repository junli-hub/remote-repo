#include "clientHead.h"
int main(void)
{
    //TCP连接
    int sfd=socket(AF_INET,SOCK_STREAM,0);
    ERROR_CHECK(sfd,-1,"socket");

    char ip[]=IP;
    char port[]=PORT;

    struct sockaddr_in clientaddr;
    memset(&clientaddr,0,sizeof(clientaddr));
    clientaddr.sin_family=AF_INET;
    clientaddr.sin_port=htons(atoi(port));
    clientaddr.sin_addr.s_addr=inet_addr(ip);

    int ret=connect(sfd,(struct sockaddr *)&clientaddr,sizeof(clientaddr));
    ERROR_CHECK(ret,-1,"connect");
    //用户登录与注册
    char username[1000];
    ret=getAndSendname(username,sfd);//发送并查找用户名
    if(ret!=0)
    {
        printf("sign:\n");
        ret=sign(sfd,username);//注册
    }
    else
    {
        printf("login:\n");
        ret=login(username,sfd);//登录
    }
    if(ret!=0)//注册或登录错误则返回
    {
        printf("ERROR,code exit\n");
        close(sfd);
        return 0;
    }
    char command[1000]={0};
    char c;
    int cmdLen=1,op;
    train_t train;
    printTips();
    printf("->");
    while((c=getchar())=='\n');
    command[0]=c;
    while(1)
    {
        while((c=getchar())!='\n')
        {
            command[cmdLen++]=c;
        }
        command[cmdLen]='\0';
        if(cmdLen==0)   continue;
        op=makeOP(command,1000);
        //发送操作符
        bzero(&train,sizeof(train));
        train.length=sizeof(op);
        memcpy(train.data,&op,sizeof(op));
        send(sfd, &train, sizeof(train.length) + train.length, MSG_NOSIGNAL);
        switch(op){
        case FILE_REMOVE:
            {
                clientRemove(command,sfd);
                break;
            }
        case FILE_GETS:
            {
                getClient(sfd,command);
                break;
            }
        case FILE_PUTS:
            {
                putsClient(command,sfd);
                break;
            }
        case DIR_CD:
            {
                clientRemove(command,sfd);
                break;
            }
        case DIR_LS:
            {
                clientLs(sfd);
                break;
            }
        case DIR_PWD:
            {
                clientPwd(sfd);
                break;
            }
        case DIR_MKDIR:
            {
                clientRemove(command,sfd);
                break;
            }
        case DIR_RMDIR:
            {
                clientRemove(command,sfd);
                break;
            }
        case USER_EXIT:
            {
                printf("USER_EXIT\n");
                close(sfd);
                return 0;
            }
        case OTHER:
            {
                printf("command not find or command error\n");
                break;
            }
        }
        bzero(command,sizeof(command));
        cmdLen=0;
        printf("->");
    }

    close(sfd);
    return 0;
}

