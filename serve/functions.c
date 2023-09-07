#include "threadPool.h"
//return 0 正常，return 1 ，没有找到文件
int getServer(int netfd,int id,MYSQL* pmysql)
{
    //接收文件名
    char filename[4096] = {0};
    train_t train;
    bzero(&train,sizeof(train));
    recvn(netfd,&train.length,sizeof(train.length));
    recvn(netfd,train.data,train.length);
    memcpy(filename,train.data,train.length);
    //查找md5
    char sql[1024] ;
    sprintf(sql, "select md5 from fileInfo where filename='%s' and pre_id=%d;" ,filename,id);
    int qret = mysql_query(pmysql,sql);
    if(qret != 0){
        fprintf(stderr,"mysql_query 1:%s\n", mysql_error(pmysql));
        return -1;
    }
    //没有md5，文件不存在
    int tag=1;
    MYSQL_RES * result = mysql_store_result(pmysql);
    if(mysql_num_rows(result)==0)
    {
        tag=0;
    }
    //发送文件是否存在状态
    bzero(&train,sizeof(train));
    train.length=sizeof(train.length);
    memcpy(train.data,&tag,train.length);
    send(netfd,&train,sizeof(train.length)+train.length,MSG_NOSIGNAL);
    if(tag==0) return -1;
    //计算文件路径
    MYSQL_ROW row;
    char readname[1024]={0};
    row=mysql_fetch_row(result);
    char path1[]="/home/lijun/remote-repo/serve/";
    strcpy(readname, path1);
    strcat(readname, row[0]);
    //接受偏移量
    train_t breakPoint;
    int lseekSize=0;//偏移量
    bzero(&breakPoint,sizeof(breakPoint));
    recvn(netfd,&breakPoint.length,sizeof(breakPoint.length));
    recvn(netfd,&lseekSize,breakPoint.length);
    //打开要发送的文件对象，获取文件内容大小信息，并发送给客户端
    int fd = open(readname,O_RDWR,0666);
    ERROR_CHECK(fd,-1,"open");
    struct stat statbuf;
    fstat(fd,&statbuf);
    train.length = sizeof(statbuf.st_size);
    memcpy(train.data,&statbuf.st_size,train.length);
    send(netfd,&train,sizeof(train.length)+train.length,MSG_NOSIGNAL);
    //偏移fd
    lseek(fd,lseekSize,SEEK_SET);
    while(statbuf.st_size-lseekSize>0){
        bzero(train.data,sizeof(train.data));
        ssize_t sret = read(fd,train.data,sizeof(train.data));
        if(sret == 0){
            break;
        }
        train.length = sret;
        send(netfd,&train,sizeof(train.length)+train.length,MSG_NOSIGNAL);
        lseekSize+=sret;
    }
    close(fd);

    return 0;

}
int lsServe(MYSQL *pMySql,char *username,int fileID,int netfd)
{
    train_t train;
    bzero(&train,sizeof(train));
    char sql[4096]={0};
    sprintf(sql,"select * from fileInfo WHERE user='%s' and pre_id=%d and tomb=0;"
            ,username,fileID);
    int qret = mysql_query(pMySql,sql);
    if(qret != 0){
        fprintf(stderr,"ls 1:%s\n", mysql_error(pMySql));
        return -1;
    }
    MYSQL_RES * result = mysql_store_result(pMySql);
    if(result==NULL)
    {
        fprintf(stderr, "ls 2: %s\n", mysql_error(pMySql));
        return -1;
    }
    int nums=mysql_num_rows(result);
    train.length=sizeof(train.length);
    memcpy(train.data,&nums,train.length);
    send(netfd, &train, sizeof(train.length) + train.length, MSG_NOSIGNAL);
    MYSQL_ROW row;
    while( (row = mysql_fetch_row(result)) != NULL)
    {
        bzero(&train,sizeof(train));
        train.length=strlen(row[1]);
        memcpy(train.data,row[1],train.length);
        send(netfd, &train, sizeof(train.length) + train.length, MSG_NOSIGNAL);
    }
    return 0;
}
int serveRmdir(MYSQL *pMySql,int fileID,const char* username,int netfd)
{
    //接收目录名
    train_t train;
    bzero(&train,sizeof(train));
    recvn(netfd,&train.length,sizeof(train.length));
    recvn(netfd,train.data,train.length);
    //目录名,用户名，pre_id 确定 主键id
    char ID[1000]={0};
    char sql2[4096]={0};
    sprintf(sql2,"select * from fileInfo WHERE filename = '%s' and user = '%s' and pre_id = %d;"
            ,train.data,username,fileID);
    int qret = mysql_query(pMySql,sql2);
    if(qret != 0){
        fprintf(stderr,"serveRmdir 1:%s\n", mysql_error(pMySql));
        return -1;
    }
    MYSQL_RES * resultSql3 = mysql_store_result(pMySql);
    if(resultSql3==NULL)
    {
        fprintf(stderr, "serveRmdir 2: %s\n", mysql_error(pMySql));
        return -1;
    }
    MYSQL_ROW row3;
    row3 = mysql_fetch_row(resultSql3);
    memcpy(ID,row3[0],strlen(row3[0]));
    //判断是否为空目录
    bzero(sql2,sizeof(sql2));
    sprintf(sql2,"select * from fileInfo WHERE pre_id= %s",ID);
    qret = mysql_query(pMySql,sql2);
    if(qret != 0){
        fprintf(stderr,"serveRmdir 3:%s\n", mysql_error(pMySql));
        return -1;
    }
    MYSQL_RES * result = mysql_store_result(pMySql);
    if(result==NULL)
    {
        fprintf(stderr, "serveRmdir 4: %s\n", mysql_error(pMySql));
        return -1;
    }
    if(mysql_num_rows(result)!=0) return 0;
    //发送最终指令
    char sql1[1024] = "DELETE FROM fileInfo WHERE id=";
    strcat(sql1,ID);
    strcat(sql1,";");
    qret = mysql_query(pMySql,sql1);
    if(qret != 0){
        fprintf(stderr,"mysql_query:%s\n", mysql_error(pMySql));
        return -1;
    }
    return 0;
}
int pwdServe(MYSQL *pMySql,int fileID,int netfd)
{
    train_t train;
    bzero(&train,sizeof(train));
    char sql[4096]={0};
    sprintf(sql,"select * from fileInfo WHERE id=%d and tomb=0;",fileID);
    int qret = mysql_query(pMySql,sql);
    if(qret != 0){
        fprintf(stderr,"pwd 1:%s\n", mysql_error(pMySql));
        return -1;
    }
    MYSQL_RES * result = mysql_store_result(pMySql);
    if(result==NULL)
    {   
        fprintf(stderr, "pwd  2: %s\n", mysql_error(pMySql));
        return -1;
    }
    if(mysql_num_rows(result)==0)
    {
        char a[]="Serve:PWD ERROR";
        train.length=sizeof(a);
        memcpy(train.data,a,train.length);
        send(netfd, &train, sizeof(train.length) + train.length, MSG_NOSIGNAL);
    }
    else
    {
        MYSQL_ROW row;
        row = mysql_fetch_row(result);
        train.length=strlen(row[2]);
        memcpy(train.data,row[2],train.length);
        send(netfd, &train, sizeof(train.length) + train.length, MSG_NOSIGNAL);
    }
    return 0;
}
// cd . cd .. cd ~ cd + (com)
int cdServe(MYSQL *pMySql,int *fileID,char *username,int netfd)
{
    char command[1000]={0};
    char sql[4096]={0};
    //接受参数
    train_t train;
    bzero(&train,sizeof(train));
    recvn(netfd,&train.length,sizeof(train.length));
    recvn(netfd,command,train.length);
    if(strcmp(command,".")==0)
    {

    }
    else if(strcmp(command,"..")==0)
    {
        sprintf(sql,"select * from fileInfo WHERE id=%d and tomb=0;",*fileID);
        int qret = mysql_query(pMySql,sql);
        if(qret != 0){
            fprintf(stderr,"cd .. 1:%s\n", mysql_error(pMySql));
            return -1;
        }
        MYSQL_RES * result = mysql_store_result(pMySql);
        if(result==NULL)
        {
            fprintf(stderr, "cd ..  2: %s\n", mysql_error(pMySql));
            return -1;
        }
        if(mysql_num_rows(result)==0) return -1;
        MYSQL_ROW row;
        row = mysql_fetch_row(result);
        *fileID=atoi(row[4]);  
    }
    else if(strcmp(command,"~")==0)
    {
        sprintf(sql,"select * from fileInfo WHERE user='%s' and pre_id=-1 and tomb=0;",username);
        int qret = mysql_query(pMySql,sql);
        if(qret != 0){
            fprintf(stderr,"cd ~ 1:%s\n", mysql_error(pMySql));
            return -1;
        }
        MYSQL_RES * result = mysql_store_result(pMySql);
        if(result==NULL)
        { 
            fprintf(stderr, "cd ~  2: %s\n", mysql_error(pMySql));
            return -1;
        } 
        if(mysql_num_rows(result)==0) return -1;
        MYSQL_ROW row;
        row = mysql_fetch_row(result);
        *fileID=atoi(row[0]);
    }
    else
    {
        sprintf(sql,"select * from fileInfo WHERE user='%s' and filename='%s' and md5='0' and tomb=0;"
                ,username,command);
        int qret = mysql_query(pMySql,sql);
        if(qret != 0){
            fprintf(stderr,"cd other 1:%s\n", mysql_error(pMySql));
            return -1;
        }
        MYSQL_RES * result = mysql_store_result(pMySql);
        if(result==NULL)
        {
            fprintf(stderr,"cd other 2: %s\n", mysql_error(pMySql));
            return -1;
        }
        if(mysql_num_rows(result)==0) return -1;
        MYSQL_ROW row;
        row = mysql_fetch_row(result);
        *fileID=atoi(row[0]);
    }
    return 0;
}
int findFileID(MYSQL *pMySql,char *username)
{
    char sql[4096]={0};
    sprintf(sql,"select * from fileInfo WHERE user = '%s' and pre_id=-1 and tomb =0;",username);
    int qret = mysql_query(pMySql,sql);                      
    if(qret != 0){                                           
        fprintf(stderr,"findFileID 1:%s\n", mysql_error(pMySql));
        return -1;                                           
    }
    MYSQL_RES * result = mysql_store_result(pMySql);
    if(result==NULL)
    {
        fprintf(stderr, "findFileID 2: %s\n", mysql_error(pMySql));
        return -1;
    }
    if(mysql_num_rows(result)==0) return -1;
    MYSQL_ROW row;
    row = mysql_fetch_row(result);
    int id=atoi(row[0]);
    return id;
}
//fileID为-1则建立用户根目录
int serveMkdir(MYSQL *pMySql,char *username,int fileID,int netfd)
{
    char sql[4096]={0};
    if(fileID==-1)
    {
        sprintf(sql,"insert into fileInfo (filename, path, user,pre_id,md5,tomb) value ('%s', '%s', '%s','%d','%s','%d');",
                "/","/",username,fileID,"0",0);
        int qret = mysql_query(pMySql,sql);
        if(qret != 0){
            fprintf(stderr,"serveMkdir 1:%s\n", mysql_error(pMySql));
            return -1;
        }    
    }
    else
    {
        train_t train;
        char filename[1000]={0};
        bzero(&train,sizeof(train));
        recvn(netfd,&train.length,sizeof(train.length));
        recvn(netfd,filename,train.length);
        char md5[]="0";
        insertFile(pMySql,filename,md5,fileID,username);
    }
    return 0;
}
//数据库中插入文件
int insertFile(MYSQL *pMySql,char *filename,char *md5,int fileID,const char *username)
{
    char path[1000]={0}; 
    char sql[4096]={0};
    sprintf(sql,"select * from fileInfo WHERE id = %d;",fileID);
    int qret = mysql_query(pMySql,sql);
    if(qret != 0){
        fprintf(stderr,"insertFile 1:%s\n", mysql_error(pMySql));
        return -1;
    }
    MYSQL_RES * result = mysql_store_result(pMySql);
    if(result==NULL)
    {
        fprintf(stderr, "insertFile 2: %s\n", mysql_error(pMySql));
        return -1;
    }
    MYSQL_ROW row;
    row = mysql_fetch_row(result);
    if(atoi(row[4])==-1)
    {
        sprintf(path,"/%s",filename);
    }
    else
    {
        sprintf(path,"%s/%s",row[2],filename);
    }
    //查看有无相同行
    bzero(sql,sizeof(sql));
    sprintf(sql,"select * from fileInfo WHERE filename='%s' and path ='%s' and user='%s' and pre_id=%d and md5='%s' and tomb=0;"
            ,filename,path,username,fileID,md5);
    qret = mysql_query(pMySql,sql);
    if(qret != 0){
        fprintf(stderr,"insertFile 1:%s\n", mysql_error(pMySql));
        return -1;
    } 
    MYSQL_RES * result1 = mysql_store_result(pMySql);
    if(result1==NULL)
    {
        fprintf(stderr, "insertFile 2: %s\n", mysql_error(pMySql));
        return -1;
    }

    if(mysql_num_rows(result1)!=0) return 0;
    //插入行
    bzero(sql,sizeof(sql));
    sprintf(sql,"insert into fileInfo (filename, path, user,pre_id,md5,tomb) value ('%s', '%s', '%s','%d','%s','%d');",
            filename,path,username,fileID,md5,0);
    qret = mysql_query(pMySql,sql);
    if(qret != 0){
        fprintf(stderr,"insertFile 3:%s\n", mysql_error(pMySql));
        return -1;
    }

    return 0;
}
int putsServe(MYSQL *pMySql,int fileID,const char* username,int netfd)
{
    //tag客户端文件存在确认码
    int tag;
    train_t train;
    bzero(&train,sizeof(train));
    recvn(netfd,&train.length,sizeof(train.length));
    recvn(netfd,&tag,train.length);
    if(tag==0) return -1;
    //接受md5码与文件名
    bzero(&train,sizeof(train));
    recvn(netfd,&train.length,sizeof(train.length));
    recvn(netfd,train.data,train.length);
    char md5[1000]={0};
    memcpy(md5,train.data,train.length);
    char filename[1000]={0};
    bzero(&train,sizeof(train));
    recvn(netfd,&train.length,sizeof(train.length));
    recvn(netfd,train.data,train.length);
    memcpy(filename,train.data,train.length);
    //判断是否需要秒传，给客户端发送状态码，1秒传，0非秒传
    char sql[4096]={0};
    sprintf(sql,"select * from fileInfo WHERE filename = '%s';",md5);
    int qret = mysql_query(pMySql,sql);
    if(qret != 0){
        fprintf(stderr,"putsServe 1:%s\n", mysql_error(pMySql));                     
        return -1;
    }
    MYSQL_RES * result = mysql_store_result(pMySql);
    if(result==NULL)
    {
        fprintf(stderr, "putsServe 2: %s\n", mysql_error(pMySql));
        return -1;
    }
    if(mysql_num_rows(result)!=0)
    {
        //秒传
        tag=1;
        bzero(&train,sizeof(train));
        train.length=sizeof(train.length);
        memcpy(train.data,&tag,train.length);
        send(netfd, &train, sizeof(train.length) + train.length, MSG_NOSIGNAL);
    }
    else
    {
        //正常传输
        tag=0;
        bzero(&train,sizeof(train));
        train.length=sizeof(train.length);
        memcpy(train.data,&tag,train.length);
        send(netfd, &train, sizeof(train.length) + train.length, MSG_NOSIGNAL);
        //创建md5文件并接受文件内容
        char configMd5[2048]={0};
        sprintf(configMd5,"/home/lijun/remote-repo/serve/config/%s",md5);
        int fd=open(configMd5,O_RDWR | O_CREAT,0777);
        int length;
        bzero(&train,sizeof(train));
        recvn(netfd,&train.length,sizeof(train.length));
        recvn(netfd,&length,train.length);
        printf("%d\n",length);
        ftruncate(fd,length);
        char *p = (char *)mmap(NULL,length,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
        recvn(netfd,p,length);
        munmap(p,length);
        close(fd);
    }
    insertFile(pMySql,filename,md5,fileID,username);
    return 0;
}
//输入：数据库指针，根目录ID数组，用户名数组,网络socket
//返回值：成功返回0，异常返回-1
int serveRemove(MYSQL *pMySql,int fileID,const char* username,int netfd)
{

    //接收文件名
    train_t train;
    bzero(&train,sizeof(train));
    recvn(netfd,&train.length,sizeof(train.length));
    recvn(netfd,train.data,train.length);

    //文件名,用户名，pre_id 确定 主键id
    char ID[1000]={0};
    char sql2[4096]={0};
    sprintf(sql2,"select * from fileInfo WHERE filename = '%s' and user = '%s' and pre_id = %d;"
            ,train.data,username,fileID);
    int qret = mysql_query(pMySql,sql2);
    if(qret != 0){
        fprintf(stderr,"mysql_query 2:%s\n", mysql_error(pMySql));
        return -1;
    }
    MYSQL_RES * resultSql3 = mysql_store_result(pMySql);
    if(resultSql3==NULL)
    {
        fprintf(stderr, "Remove Failed to get result set: Error: %s\n", mysql_error(pMySql));
        return -1;
    }
    MYSQL_ROW row3;
    row3 = mysql_fetch_row(resultSql3);
    memcpy(ID,row3[0],strlen(row3[0]));
    //发送最终指令
    char sql1[1024] = "update fileInfo set tomb = 1 where id = ";
    strcat(sql1,ID);
    strcat(sql1,";");
    qret = mysql_query(pMySql,sql1);
    if(qret != 0){
        fprintf(stderr,"mysql_query:%s\n", mysql_error(pMySql));
        return -1;
    }

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
//用户存在返回1
int recvUsername(int netfd,MYSQL *pMySql,char *username)
{
    train_t train;
    int m=0;
    bzero(&train,sizeof(train));
    recvn(netfd,&train.length,sizeof(train.length));
    recvn(netfd,train.data,train.length);
    char sql1[2048]={0};
    memcpy(username,train.data,train.length);
    sprintf(sql1,"select *from userInfo WHERE user = '%s' and tomb = 0;",train.data);
    int qret=mysql_query(pMySql,sql1);
    bzero(&train,sizeof(train));
    train.length=sizeof(train.length);
    if(qret != 0){
        fprintf(stderr,"recvUsername1:%s\n", mysql_error(pMySql));
        memcpy(train.data,&m,train.length);
        send(netfd, &train, sizeof(train.length) + train.length, MSG_NOSIGNAL);
        return -1;
    }
    MYSQL_RES * resultSql2 = mysql_store_result(pMySql);
    if(resultSql2==NULL)
    {
        fprintf(stderr, "recvUsername2: %s\n", mysql_error(pMySql));
        memcpy(train.data,&m,train.length);
        send(netfd, &train, sizeof(train.length) + train.length, MSG_NOSIGNAL);
        return -1;
    }
    if(mysql_num_rows(resultSql2)!=0)
    {
        m=1;
    }
    memcpy(train.data,&m,train.length);
    send(netfd, &train, sizeof(train.length) + train.length, MSG_NOSIGNAL);
    return m;
}
int recvLogin(int netfd,MYSQL *pMySql)
{
    //接收用户名/密码
    train_t train;
    char username[1000]={0};
    bzero(&train,sizeof(train));
    recvn(netfd,&train.length,sizeof(train.length));
    recvn(netfd,username,train.length);
    bzero(&train,sizeof(train));
    recvn(netfd,&train.length,sizeof(train.length));
    recvn(netfd,train.data,train.length);
    //取出盐值，加密值
    char salt[1024]={0};
    char regHash[1024]={0};
    char query[1024]={0};
    sprintf(query,"select *from userInfo WHERE user = '%s' and tomb = 0;",username);

    int qret = mysql_query(pMySql, query);
    if(qret!=0)
    {
        printf("recvLogin : not find user\n");
        return -1;
    }
    MYSQL_RES* result =mysql_store_result(pMySql);
    if(result==0)
    {
        printf("recvLogin : not find user\n");
        return -1;
    }
    MYSQL_ROW row2;
    row2 = mysql_fetch_row(result);
    memcpy(salt,row2[2],strlen(row2[2]));
    memcpy(regHash,row2[3],strlen(row2[3]));
    //验证密码，失败返回-1、并发送登录状态
    char *trash1=crypt(train.data, salt);
    char trash2[1000]={0};
    strcat(trash2,salt);
    strcat(trash2,regHash);
    bzero(&train,sizeof(train));
    int m=0;
    train.length=sizeof(train.length);
    if(strcmp(trash1,trash2)!=0)
    {
        m=-1;
        memcpy(train.data,&m,train.length);
        send(netfd, &train, sizeof(train.length) + train.length, MSG_NOSIGNAL);
        return -1;
    }
    memcpy(train.data,&m,train.length);  
    send(netfd, &train, sizeof(train.length) + train.length, MSG_NOSIGNAL);
    return 0;
}
int recvSign(int netfd,MYSQL *pMySql)
{
    //接收用户名/密码
    train_t train;
    char username[1000]={0};
    bzero(&train,sizeof(train));
    recvn(netfd,&train.length,sizeof(train.length));
    recvn(netfd,username,train.length);
    bzero(&train,sizeof(train));
    recvn(netfd,&train.length,sizeof(train.length));
    recvn(netfd,train.data,train.length);
    //随机生成16位的盐值
    srand(time(NULL));
    char salt[1024]={0};
    char String[17] = {0};
    int index;
    char characters[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    for(int i=0;i<16;i++)
    {
        index = rand() % 51;
        String[i] = characters[index];
    }
    String[16]='\0';
    strcat(salt,"$6$");
    strcat(salt,String);
    strcat(salt,"$");
    //生成加密后的密码
    char* trash = crypt(train.data, salt);
    strtok(trash, "$");
    strtok(NULL, "$");
    char *regHash=strtok(NULL,"$");
    char query[1000]={0};
    bzero(query, sizeof(query));
    sprintf(query, "insert into userInfo (user, salt, encrypted_password,tomb) value ('%s', '%s', '%s',0);"
            ,username,salt,regHash);
    //将用户名，盐值，加密密码存入库中并发送注册状态
    int qret = mysql_query(pMySql, query);
    bzero(&train,sizeof(train));
    train.length=sizeof(train.length);
    memcpy(train.data,&qret,train.length);
    send(netfd, &train, sizeof(train.length) + train.length, MSG_NOSIGNAL);

    return 0;
}
