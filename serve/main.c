#include "threadPool.h"
int initThreadPool(threadPool_t *pThreadPool, int workerNum){
    pThreadPool->workerNum = workerNum;
    pThreadPool->workerArr = (pthread_t *)calloc(workerNum,sizeof(pthread_t));
    initTaskQueue(&pThreadPool->taskQueue);
    pthread_cond_init(&pThreadPool->cond,NULL);
    pthread_mutex_init(&pThreadPool->mutex,NULL);
    return 0;
}
int main(void)
{

    threadPool_t threadPool;// 申请内存
    initThreadPool(&threadPool,atoi(NUM));//初始化线程池
    makeWorker(&threadPool);//初始化线程，准备执行代码
    int sockfd;
    tcpInit(IP,PORT,&sockfd);//接受客户端连接
    int epfd = epoll_create(1);//epoll监听客户端是否发送任务
    epollAdd(epfd,sockfd);
    
    struct epoll_event readyset[10];
    while(1){
        int readyNum = epoll_wait(epfd,readyset,10,-1);
        for(int i = 0; i < readyNum; ++i){
            if(readyset[i].data.fd == sockfd){
                int netfd = accept(sockfd,NULL,NULL);
                pthread_mutex_lock(&threadPool.mutex);
                enQueue(&threadPool.taskQueue,netfd);
                pthread_cond_signal(&threadPool.cond);
                pthread_mutex_unlock(&threadPool.mutex);
            }
        }
    }
    return 0;
}

