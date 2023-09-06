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
        int netfd = pThreadPool->taskQueue.pFront->netfd;
        deQueue(&pThreadPool->taskQueue);//取任务
        pthread_mutex_unlock(&pThreadPool->mutex);

        //执行任务，无需加锁
        transFile(netfd);
        close(netfd);
        printf("worker finished a job!\n");
    }
}
