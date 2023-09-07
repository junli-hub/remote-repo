#include <func.h> 
#ifndef _THREAD_POOL_
#define _THREAD_POOL_
typedef struct node_s{
      int netfd;
      struct node_s *pNext;
} node_t;
typedef struct taskQueue_s{
   node_t *pFront;
   node_t *pRear;
   int queueSize;
} taskQueue_t;

typedef struct threadPool_s{
    int workerNum;
    pthread_t  *workerArr;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    taskQueue_t taskQueue;
}threadPool_t;
typedef struct train_s {
    int length;
    char data[1000]; //1000是上限值
}train_t;
#define IP "192.168.50.129"
#define PORT "1234"
#define NUM "3"
#define NETDISK_LOG_INFO(op) { \
    syslog(LOG_INFO, "File:%s Function:%s line:%d User_operation:%s\n",__FILE__,__FUNCTION__,__LINE__,op);\
} 

#define NETDISK_LOG_ERROR(op,errorfun,intflag){syslog(LOG_ERR, "File:%s Function:%s line:%d  User_operation:%s errorfun:%s flag=%d\n",__FILE__,__FUNCTION__,__LINE__,op,errorfun,intflag);}  
//线程池
int initThreadPool(threadPool_t *pThreadPool, int workerNum);
int makeWorker(threadPool_t *pThreadPool);
int tcpInit(char *ip, char *port, int * psockfd);
int transFile(int netfd);
void *threadFunc(void *arg);
int epollAdd(int epfd, int fd);
int epollDel(int epfd, int fd);
enum{
    //目录相关操作
    DIR_CD = 100,
    DIR_LS,
    DIR_PWD,
    DIR_MKDIR,
    DIR_RMDIR,

    //文件相关操作
    FILE_PUTS = 200,
    FILE_GETS,
    FILE_REMOVE,
    FILE_RETRAIN,

    //用户直接操作
    USER_EXIT,
    USER_SIGN,
    OTHER

};

//任务队列
int taskEnQueue(taskQueue_t *pTaskQueue, int netFd);
int taskDeQueue(taskQueue_t *pTaskQueue);
int enQueue(taskQueue_t *pTaskQueue, int netfd);
int initTaskQueue(taskQueue_t *pTaskQueue);
int deQueue(taskQueue_t *pTaskQueue);
//登录注册
int recvn(int sockfd, void* buf, int lengthgth);
int recvUsername(int netfd,MYSQL *pMySql,char *username); 
int recvSign(int netfd,MYSQL *pMySql);
int recvLogin(int netfd,MYSQL *pMySql);

//输入：数据库指针，根目录ID数组，用户名数组,网络socket
//返回值：成功返回0，异常返回-1
int serveRemove(MYSQL *pMySql,int fileID,const char* username,int netfd);
//输入：数据库指针，根目录ID数组，用户名数组,网络socket
//返回值：成功返回0，异常返回-1
int putsServe(MYSQL *pMySql,int fileID,const char* username,int netfd);
//数据库中插入文件
int insertFile(MYSQL *pMySql,char *filename,char *md5,int fileID,const char *username); 
//fileID为-1则建立用户根目录
int serveMkdir(MYSQL *pMySql,char *username,int fileID,int netfd);
int findFileID(MYSQL *pMySql,char *username);  
int cdServe(MYSQL *pMySql,int *fileID,char *username,int netfd);
int pwdServe(MYSQL *pMySql,int fileID,int netfd);
int serveRmdir(MYSQL *pMySql,int fileID,const char* username,int netfd);
int lsServe(MYSQL *pDataSql,char *username,int fileID,int netfd); 
int getServer(int netfd,int id,MYSQL* pmysql);

#endif
