#include "threadPool.h"
int initTaskQueue(taskQueue_t *pTaskQueue){
    bzero(pTaskQueue,sizeof(taskQueue_t));
    return 0;
}
int enQueue(taskQueue_t *pTaskQueue, int netfd){
    node_t * pNew = (node_t *)calloc(1,sizeof(node_t));
    pNew->netfd = netfd;
    if(pTaskQueue->queueSize == 0){
        pTaskQueue->pFront = pNew;
        pTaskQueue->pRear = pNew;
    }
    else{
        pTaskQueue->pRear->pNext = pNew;
        pTaskQueue->pRear = pNew;
    }
    ++pTaskQueue->queueSize;
    return 0;
}
int deQueue(taskQueue_t *pTaskQueue){
    node_t *pCur = pTaskQueue->pFront;
    pTaskQueue->pFront= pCur->pNext;
    free(pCur);
    if(pTaskQueue->queueSize == 1){
        pTaskQueue->pRear = NULL;
    }
    --pTaskQueue->queueSize;
    return 0;
}
