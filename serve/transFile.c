#include "threadPool.h"
int transFile(int netfd){
    MYSQL * pDataSql=mysql_init(NULL);
    MYSQL * ret1 = mysql_real_connect(pDataSql, "localhost","root","123456qwe",
                                        "lijun_test",0,NULL,0);
      if(ret1 == NULL){
          fprintf(stderr,"mysql_real_connect:%s\n", mysql_error(pDataSql));
          return -1;
      }

    //注册登录
    char username[1000]={0};
    int ret=recvUsername(netfd,pDataSql,username);
    if(ret!=1)
    { 
        ret=recvSign(netfd,pDataSql);
        serveMkdir(pDataSql,username,-1,netfd);
    }
    else
    {
        ret=recvLogin(netfd,pDataSql);
    }
    if(ret==-1)
    {
        mysql_close(pDataSql);
        return -1;
    }
    int fileID=findFileID(pDataSql,username);
    if(fileID<0)
    {
        mysql_close(pDataSql);
        return -1;
    }
    train_t train;  
    int op=0;
    while(1)
    {
        //接收操作符
        bzero(&train,sizeof(train));
        recvn(netfd,&train.length,sizeof(train.length));
        recvn(netfd,&op,train.length);
        switch(op){
        case FILE_REMOVE:
            {
                serveRemove(pDataSql,fileID,username,netfd);
                break;
            }
        case FILE_GETS:
            {
                printf("FILE_GETS\n");
                break;
            }
        case FILE_PUTS:
            {
                putsServe(pDataSql,fileID,username,netfd);
                break;
            }
        case DIR_CD:
            {
                cdServe(pDataSql,&fileID,username,netfd);
                break;
            }
        case DIR_LS:
            {
                printf("DIR_LS\n");
                break;
            }
        case DIR_PWD:
            {
                pwdServe(pDataSql,fileID,netfd);
                break;
            }
        case DIR_MKDIR:
            {
                serveMkdir(pDataSql,username,fileID,netfd);
                break;
            }
        case DIR_RMDIR:
            {
                serveRmdir(pDataSql,fileID,username,netfd);
                break;
            }
        case USER_EXIT:
            {
                printf("USER_EXIT\n");
                goto end;
            }
        case OTHER:
            {
                printf("command not find or command error\n");
                break;
            }
        }
    }
end:
    mysql_close(pDataSql);
    return 0;
}
