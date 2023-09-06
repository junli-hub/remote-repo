#include "clientHead.h"
//返回值：成功返回0，失败返回-1
int clientPwd(int sfd)
{
    train_t train;
    bzero(&train,sizeof(train));
    recvn(sfd,&train.length,sizeof(train.length));
    recvn(sfd,train.data,train.length);
    printf("%s\n",train.data);
    return 0;
}
//返回值：成功返回0，失败返回-1
int putsClient(char *command,int sfd)
{
    int length=0,tag=0;
    train_t train;
    int ret=FileExistOrSize(command,&length);
    if(ret==-2)
    {
        bzero(&train,sizeof(train));
        train.length=sizeof(train.length);
        memcpy(train.data,&tag,train.length);
        send(sfd, &train, sizeof(train.length) + train.length, MSG_NOSIGNAL);
        printf("file no exit\n");
        return -1;
    }
    else if(ret==0)
    {
        tag=1;
        bzero(&train,sizeof(train));
        train.length=sizeof(train.length);
        memcpy(train.data,&tag,train.length);
        send(sfd, &train, sizeof(train.length) + train.length, MSG_NOSIGNAL);
        char md5_str[1000]={0};
        //计算md5,并发送md5与文件名
        bzero(&train,sizeof(train));
        Compute_file_md5(command, md5_str);        
        train.length=strlen(md5_str);
        memcpy(train.data,md5_str,train.length);
        send(sfd, &train, sizeof(train.length) + train.length, MSG_NOSIGNAL);
        bzero(&train,sizeof(train));
        train.length=strlen(command);
        memcpy(train.data,command,train.length);
        send(sfd, &train, sizeof(train.length) + train.length, MSG_NOSIGNAL);
        //接收服务端是否触发秒传的信号
        bzero(&train,sizeof(train));
        recvn(sfd,&train.length,sizeof(train.length));
        recvn(sfd,&tag,train.length);
        if(tag==0)
        {
            
            //发送文件大小
            bzero(&train,sizeof(train));
            train.length=sizeof(train.length);
            memcpy(train.data,&length,train.length);
            send(sfd, &train, sizeof(train.length) + train.length, MSG_NOSIGNAL);
            //传输文件
            int fd=open(command,O_RDWR);
            
            char *p = (char *)mmap(NULL,length,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
            printf("%d\n",length);
            send(sfd,p,length,MSG_NOSIGNAL);
            munmap(p,length);
            close(fd);
        }
        return 0;
    }
    return -1;
}
int Compute_file_md5(const char *file_path, char *md5_str)
{
	int i;
	int fd;
	int ret;
	unsigned char data[READ_DATA_SIZE];
	unsigned char md5_value[MD5_SIZE];
	MD5_CTX md5;

	fd = open(file_path, O_RDONLY);
	if (-1 == fd)
	{
		perror("open");
		return -1;
	}

	// init md5
	MD5Init(&md5);

	while (1)
	{
		ret = read(fd, data, READ_DATA_SIZE);
		if (-1 == ret)
		{
			perror("read");
			return -1;
		}

		MD5Update(&md5, data, ret);

		if (0 == ret || ret < READ_DATA_SIZE)
		{
			break;
		}
	}

	close(fd);

	MD5Final(&md5, md5_value);

	for(i = 0; i < MD5_SIZE; i++)
	{
		snprintf(md5_str + i*2, 2+1, "%02x", md5_value[i]);
	}
	md5_str[MD5_STR_LEN] = '\0'; // add end

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

int clientRemove(const char *command,int sfd)
{
    char com[1000]={0};
    strcpy(com,command);
    int length=strlen(com);
    train_t train;
    train.length=length;
    memcpy(train.data,com,length);
    send(sfd,&train,sizeof(train.length)+train.length,MSG_NOSIGNAL);
    return 0;
}

int recvn(int sockfd, void* buf, int lengthgth)
{
    int total = 0; // 已经收到的总长度
    char* p = (char*)buf;
    while(total < lengthgth){
        ssize_t sret = recv(sockfd, p + total, lengthgth - total, 0);
        total += sret;
    }
    return 0;
}
//成功返回0，失败返回-1
int getAndSendname(char *username,int netFd)
{
    printf("username = ");
    scanf("%s",username);
    train_t train;
    int tag;
    bzero(&train,sizeof(train));
    train.length=strlen(username);
    memcpy(train.data, username, strlen(username));  
    send(netFd, &train, sizeof(train.length) + train.length, MSG_NOSIGNAL);
    
    bzero(&train,sizeof(train));
    recvn(netFd, &train.length, sizeof(train.length));
    recvn(netFd, &tag, train.length);
    //tag=0,用户不存在
    if(tag==0)
    {
        printf("user not exist\n");
        return -1;
    }
    printf("use exit\n");
    return 0;
}
int sign(int netfd,char *username)
{
    printf("password:");
    char password[1000]={0};
    scanf("%s",password); 
    train_t train;
    //发送用户名与密码
    bzero(&train,sizeof(train));
    train.length=strlen(username);
    memcpy(train.data, username, strlen(username));  
    send(netfd, &train, sizeof(train.length) + train.length, MSG_NOSIGNAL);

    bzero(&train,sizeof(train));
    train.length=sizeof(password);
    memcpy(train.data,password,train.length);
    send(netfd, &train, sizeof(train.length) + train.length, MSG_NOSIGNAL);

    //接收注册状态
    int tag,m;
    bzero(&train,sizeof(train));
    recvn(netfd, &m, sizeof(m));
    recvn(netfd, &tag, m);

    if(tag==0)
    {
        printf("tag=%d  sign success\n",tag);
        return 0;
    }
    printf("tag=%d sign error\n",tag);
    return -1;
}
int login(char *username,int netFd){

    char keyWord[1000]={0};
    printf("keyWord:");
    scanf("%s",keyWord);
    train_t train;
    //发送用户名与密码
    bzero(&train,sizeof(train));
    train.length=strlen(username);
    memcpy(train.data,username,train.length);
    send(netFd, &train, sizeof(train.length) + train.length, MSG_NOSIGNAL);
    bzero(&train, sizeof(train));
    train.length = strlen(keyWord);
    memcpy(train.data, keyWord, train.length);
    send(netFd, &train, sizeof(train.length) + train.length, MSG_NOSIGNAL);
    //接收登录状态
    int trainlength = 0,tag=0;
    recvn(netFd, &trainlength, sizeof(trainlength));
    recvn(netFd, &tag, trainlength);
    if(tag == -1){
        printf("passwd error\n");
        return -1;
    }
    printf("login success\n");
    return 0;
}
void printTips()
{
    printf("|  remove----------->remove  |\n");
    printf("|  gets--------------->gets  |\n");
    printf("|  puts--------------->puts  |\n");
    printf("|  cd------------------->cd  |\n");
    printf("|  ls------------------->ls  |\n");
    printf("|  pwd----------------->pwd  |\n");
    printf("|  mkdir------------->mkdir  |\n");
    printf("|  rmdir------------->rmdir  |\n");
    printf("|  quit/exit----->quit/exit  |\n");
}
int makeOP(char *command,int len)
{
    char com[1000]={0};
    memcpy(com,command,len);
    char *op=strtok(com," ");
    char *p=strtok(NULL," ");
    bzero(command,len);
    if(p!=NULL) strcpy(command,p);
    if(strcmp(op,"remove")==0)
    {
        return FILE_REMOVE;
    }
    else if(strcmp(op,"gets")==0)
    {
        return FILE_GETS;
    }
    else if(strcmp(op,"puts")==0)
    {
        return FILE_PUTS;
    }
    else if(strcmp(op,"cd")==0)
    {
        return DIR_CD;
    }
    else if(strcmp(op,"ls")==0)
    {
        return DIR_LS;
    }
    else if(strcmp(op,"pwd")==0)
    {
        return DIR_PWD;
    }
    else if(strcmp(op,"mkdir")==0)
    {
        return DIR_MKDIR;
    }
    else if(strcmp(op,"rmdir")==0)
    {
        return DIR_RMDIR;
    }
    else if(strcmp(op,"quit")==0 || strcmp(op,"exit")==0)
    {
        return USER_EXIT;
    }
    return OTHER;
}
