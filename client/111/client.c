#include <func.h>
#define IP "192.168.50.129"
#define PORT "1234"
typedef struct train_s{
    int length;
    char data[1000];//上限值
}train_t;
// 1、输入:sockfd管道的文件描述符;fd:已存在的文件，偏移该文件后的该文件的文件描述符
// filename 文件名称
// 返回值,正确0，失败-1
// 注意：仅接受服务端sendBreakPoint.c的发送
int recvBreakPoint(int sockfd,int fd,char *filename,int len){
    //接受文件名并判断文件名是否正确
    train_t train;
    bzero(&train,sizeof(train));

    ssize_t  sret = recv(sockfd,&train.length,sizeof(train.length),MSG_WAITALL);
    ERROR_CHECK(sret,-1,"recv");

    printf("sret = %ld,train.length=%d\n",sret,train.length);
    recv(sockfd,train.data,train.length,MSG_WAITALL);
    char fileIsCorrect[4096] = {0};
    printf("train.data=%s\n",train.data);
    memcpy(fileIsCorrect,train.data,train.length);
    //文件长度扩展
    recv(sockfd,&train.length,sizeof(train.length),MSG_WAITALL);
    recv(sockfd,train.data,train.length,MSG_WAITALL);
    ftruncate(fd,atoi(train.data));
    printf("filesize=%d\n",atoi(train.data));

    lseek(fd, 0, SEEK_SET);
    lseek(fd, len, SEEK_SET);

    while(1){
        recv(sockfd,&train.length,sizeof(train.length),MSG_WAITALL);
        if(train.length == 0){
            break;
        }
        recv(sockfd,train.data,train.length,MSG_WAITALL);
        write(fd,train.data,train.length);
    }
    close(fd);
    return 0;
}

//1.输入：filename：文件名数组；length 文件长度指针，只在返回值为0时有效
//2.返回值：成功返回0，失败返回-1，文件不存在返回-2
//运行环境：当前目录
int FileExistOrSize(char *filename,int *length)
{
    //检测文件是否存在
    int ret=access(filename,F_OK);
    if(ret!=0)
    {
        return -2;
    }
    //求出文件长度
    struct stat fileStat;
    if (stat(filename, &fileStat) == -1) {
        return -1;
    }
    *length=fileStat.st_size;
    return 0;
}

int main(int argc,char *argv[])
{
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
    

    int length=0;
    char filename[]="test";
    if(FileExistOrSize(filename,&length)==0)
    {
        int fd=open(filename,O_RDWR);
        ERROR_CHECK(fd,-1,"open");
        recvBreakPoint(sfd,fd,filename,length);
    }
    else
    {
        printf("file noexit\n");
    }
    
    close(ret);
    close(sfd);
    return 0;
}

