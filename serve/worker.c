#include "threadPool.h"
int makeWorker(threadPool_t *pThreadPool){
    for(int i = 0; i < pThreadPool->workerNum; ++i){
        pthread_create(&pThreadPool->workerArr[i],NULL,threadFunc,pThreadPool);
    }
    return 0;
}
void *threadFunc(void *arg){
    threadPool_t * pThreadPool = (threadPool_t *)arg;
    while(1){
        pthread_mutex_lock(&pThreadPool->mutex);//任务队列取出任务，需要加锁
        while(pThreadPool->taskQueue.queueSize == 0){
            pthread_cond_wait(&pThreadPool->cond,&pThreadPool->mutex);
        }
        printf("worker got a job!\n");
        int netfd=pThreadPool->taskQueue.pFront->netfd; 
        int fileID=pThreadPool->taskQueue.pFront->fileID;
        MYSQL *pDataSql=pThreadPool->taskQueue.pFront->pDataSql;
        char username[1000]={0};
        int len=strlen(pThreadPool->taskQueue.pFront->username);
        if(len!=0)
        {
            memcpy(username,pThreadPool->taskQueue.pFront->username,len);   
        }
        timeWheel_t *p1=pThreadPool->taskQueue.pFront->timewheel;
        map_t *p2=pThreadPool->taskQueue.pFront->fdmap; 
        deQueue(&pThreadPool->taskQueue);//取任务

        pthread_mutex_unlock(&pThreadPool->mutex);   
        //执行任务，无需加锁
        printf("netfd:%d\n",netfd);
        if(len==0 && p1==NULL)
        {
            printf("worker going a get job!\n");
            getServer(netfd,fileID,pDataSql);
            close(netfd);
        }
        else
        {
            if(p1==NULL)
            {
                printf("worker going a puts job!\n");
                putsServe(pDataSql,fileID,username,netfd);
                close(netfd);
            }
            else
            {
                printf("worker going a long_sign job!\n");
                signLoginToken(pDataSql,netfd);
                struct epoll_event event;
                event.events=EPOLLIN;
                event.data.fd=netfd;
                epoll_ctl(fileID, EPOLL_CTL_ADD, netfd, &event);
                addTimeWheel(p1,p2,netfd);

            }
        }
        printf("worker finished a job!\n");
    }
}
