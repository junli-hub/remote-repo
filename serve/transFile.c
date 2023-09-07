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
        NETDISK_LOG_INFO("Sign");
        serveMkdir(pDataSql,username,-1,netfd);
        NETDISK_LOG_INFO("Mkdir");
    }
    else
    {
        ret=recvLogin(netfd,pDataSql);
        NETDISK_LOG_INFO("Login");
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
                NETDISK_LOG_INFO("remove");
                serveRemove(pDataSql,fileID,username,netfd);
                break;
            }
        case FILE_GETS:
            {
                NETDISK_LOG_INFO("gets");
                getServer(netfd,fileID,pDataSql);
                break;
            }
        case FILE_PUTS:
            {
                NETDISK_LOG_INFO("puts");
                putsServe(pDataSql,fileID,username,netfd);
                break;
            }
        case DIR_CD:
            {
                NETDISK_LOG_INFO("CD");
                cdServe(pDataSql,&fileID,username,netfd);
                break;
            }
        case DIR_LS:
            {
                NETDISK_LOG_INFO("LS");
                lsServe(pDataSql,username,fileID,netfd);
                break;
            }
        case DIR_PWD:
            {
                NETDISK_LOG_INFO("PWD");
                pwdServe(pDataSql,fileID,netfd);
                break;
            }
        case DIR_MKDIR:
            {
                NETDISK_LOG_INFO("Mkdir");
                serveMkdir(pDataSql,username,fileID,netfd);
                break;
            }
        case DIR_RMDIR:
            {
                NETDISK_LOG_INFO("rmdir");
                serveRmdir(pDataSql,fileID,username,netfd);
                break;
            }
        case USER_EXIT:
            {
                NETDISK_LOG_INFO("exit");
                printf("USER_EXIT\n");
                goto end;
            }
        case OTHER:
            {
                NETDISK_LOG_INFO("other");
                printf("command not find or command error\n");
                break;
            }
        }
    }
end:
    mysql_close(pDataSql);
    return 0;
}
