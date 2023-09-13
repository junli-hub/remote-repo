#include "threadPool.h"
int initThreadPool(threadPool_t *pThreadPool, int workerNum){
    pThreadPool->workerNum = workerNum;
    pThreadPool->workerArr = (pthread_t *)calloc(workerNum,sizeof(pthread_t));
    initTaskQueue(&pThreadPool->taskQueue);
    pthread_cond_init(&pThreadPool->cond,NULL);
    pthread_mutex_init(&pThreadPool->mutex,NULL);
    return 0;
}
timeWheel_t timewheel;                                                                   
map_t fdmap;
int main(void)
{

    threadPool_t threadPool;// 申请内存
    initThreadPool(&threadPool,atoi(NUM));//初始化线程池
    makeWorker(&threadPool);//初始化线程，准备执行代码
    //连接数据库
    MYSQL * pDataSql=mysql_init(NULL);
    MYSQL * ret1 = mysql_real_connect(pDataSql, "localhost","root","123456qwe",
                                      "lijun_test",0,NULL,0);
    if(ret1 == NULL){
        fprintf(stderr,"mysql_real_connect:%s\n", mysql_error(pDataSql));
    }

    int sockfd;
    tcpInit(IP,PORT,&sockfd);//接受客户端连接
    int epfd = epoll_create(1);//epoll监听客户端是否发送任务
    train_t train;
    bzero(&train,sizeof(train));
    epollAdd(epfd,sockfd);

    initTimeWheel(&timewheel,&fdmap);

    time_t ti;
    struct epoll_event readyset[1000];
    while(1){
        int readyNum = epoll_wait(epfd,readyset,1000,1000);
        for(int i = 0; i < readyNum; ++i){
            if(readyset[i].data.fd == sockfd){//sign and login //send token
                int netfd = accept(sockfd,NULL,NULL);
                pthread_mutex_lock(&threadPool.mutex);

                enQueue(&threadPool.taskQueue,netfd,epfd,pDataSql,NULL,&timewheel,&fdmap);
                pthread_cond_signal(&threadPool.cond);
                pthread_mutex_unlock(&threadPool.mutex);
            }
            else
            {
                int netfd=readyset[i].data.fd;
                int op,fileID,length;
                token_t usertoken;
                bzero(&usertoken,sizeof(usertoken));
                recvn(netfd,&length,sizeof(usertoken.length));
                recvn(netfd,&op,sizeof(usertoken.op)); 
                recvn(netfd,&fileID,sizeof(usertoken.fileID));
                recvn(netfd,usertoken.token,sizeof(usertoken.token));
                recvn(netfd,usertoken.username,sizeof(usertoken.username));
                int tag=verifyToken(pDataSql,usertoken.token,usertoken.username,fileID);
                bzero(&train,sizeof(train));
                train.length=sizeof(tag);
                memcpy(train.data,&tag,train.length);
                send(netfd,&train,sizeof(train.length)+train.length,MSG_NOSIGNAL);
                if(tag==-1)
                {
                    epoll_ctl(epfd, EPOLL_CTL_DEL, netfd,NULL);
                    delTimeWheel(&timewheel,&fdmap,netfd);
                    printf("netfd:%d close\n",netfd);
                    close(netfd);
                    continue;
                }
                updateTimeWheel(&timewheel,&fdmap,netfd);
                switch(op){
                case FILE_REMOVE:
                    {
                        NETDISK_LOG_INFO("remove");
                        printf("remove:%s\n",usertoken.username);
                        serveRemove(pDataSql,fileID,usertoken.username,netfd);
                        break;
                    }
                case FILE_GETS:
                    {
                        NETDISK_LOG_INFO("gets");
                        printf("gets\n");
                        int sfd = accept(sockfd,NULL,NULL);
                        pthread_mutex_lock(&threadPool.mutex);
                        //长命令分离建立与服务端子线程TCP连接
                        enQueue(&threadPool.taskQueue,sfd,fileID,pDataSql,NULL,NULL,NULL);
                        pthread_cond_signal(&threadPool.cond);
                        pthread_mutex_unlock(&threadPool.mutex);
                        break;
                    }
                case FILE_PUTS:
                    {
                        NETDISK_LOG_INFO("puts");
                        printf("puts\n");
                        int sfd = accept(sockfd,NULL,NULL);
                        pthread_mutex_lock(&threadPool.mutex);
                        //长命令分离建立与服务端子线程TCP连接
                        enQueue(&threadPool.taskQueue,sfd,fileID,pDataSql,usertoken.username,NULL,NULL);
                        pthread_cond_signal(&threadPool.cond);
                        pthread_mutex_unlock(&threadPool.mutex);
                        break;
                    }
                case DIR_CD:
                    {
                        NETDISK_LOG_INFO("CD");
                        cdServe(pDataSql,&fileID,usertoken.username,netfd);
                        printf("cd:%d\n",fileID);
                        break;
                    }
                case DIR_LS:
                    {
                        NETDISK_LOG_INFO("LS");
                        printf("ls\n");
                        lsServe(pDataSql,usertoken.username,fileID,netfd);
                        break;
                    }
                case DIR_PWD:
                    {
                        NETDISK_LOG_INFO("PWD");
                        printf("PWD\n");
                        pwdServe(pDataSql,fileID,netfd);
                        break;
                    }
                case DIR_MKDIR:
                    {
                        NETDISK_LOG_INFO("Mkdir");
                        printf("Mkdir\n");
                        serveMkdir(pDataSql,usertoken.username,fileID,netfd);
                        break;
                    }
                case DIR_RMDIR:
                    {
                        NETDISK_LOG_INFO("rmdir");
                        printf("rmdir\n");
                        serveRmdir(pDataSql,fileID,usertoken.username,netfd);
                        break;
                    }
                case USER_EXIT:
                    {
                        NETDISK_LOG_INFO("exit");
                        printf("USER_EXIT\n");
                        epoll_ctl(epfd, EPOLL_CTL_DEL, netfd,NULL);
                        delTimeWheel(&timewheel,&fdmap,netfd);
                        printf("netfd:%d close\n",netfd);
                        close(netfd);
                        continue;
                    }
                case OTHER:
                    {
                        NETDISK_LOG_INFO("other");
                        printf("command not find or command error\n");
                        break;
                    }
                }
                bzero(&train,sizeof(train));
                train.length=sizeof(fileID);
                memcpy(train.data,&fileID,train.length);
                send(netfd, &train, sizeof(train.length)+train.length, MSG_NOSIGNAL);
            }
        }
        ti=time(NULL);
        printf("%s\n", ctime(&ti));
        if(ti-timewheel.time>=1)
        {
            rotateTimeWheel(&timewheel,&fdmap,epfd);
        }
    }
    return 0;
}
