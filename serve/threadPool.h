#include <func.h> 
#ifndef _THREAD_POOL_
#define _THREAD_POOL_
#include "l8w8jwt/encode.h"
#define MAP 1000
#define TIMENUM 30

/*typedef struct fdSolt_s
{
    int map[100];
    int size;
}fdSolt_t;*/
typedef enum ColorTyper
{
    RED=500,
    BLACK
}ColorTyper;
typedef struct rbTreeNode_s
{
    struct rbTreeNode_s *left;
    struct rbTreeNode_s *right;
    struct rbTreeNode_s *parent;
    int fd;
    ColorTyper color;
}rbTreeNode_t;

typedef struct fdSolt_s
{
    rbTreeNode_t *root;
    rbTreeNode_t *nil;
    int size;
}fdSolt_t;

typedef struct timeWheel_s
{
    fdSolt_t timewheel[TIMENUM];
    
    int curPointer;
    int lastPointer;
    time_t time;
}timeWheel_t;
typedef struct map_s
{
    int map[MAP];
}map_t;
typedef struct node_s{
    timeWheel_t *timewheel;
    map_t *fdmap;
    int fileID;
    MYSQL * pDataSql;
    int netfd;
    char username[1000];
    struct node_s *pNext;
} node_t;
typedef struct taskQueue_s{
    node_t *pFront;
    node_t *pRear;
    int queueSize;
} taskQueue_t;
typedef struct token_s{
    int length;
    int op;
    int fileID;
    char token[1000];                                                                              
    char username[1000];
}token_t;
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
#define PORT2 "3456"
#define NUM "5"
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
//fileID:fileID或epfd
int enQueue(taskQueue_t *pTaskQueue, int netfd,int fileID,MYSQL *p,char *username,timeWheel_t *timewheel,map_t *fdmap);
int taskEnQueue(taskQueue_t *pTaskQueue, int netFd);
int taskDeQueue(taskQueue_t *pTaskQueue);
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
//返回值，成功返回0，失败返回-1
//传入：lenjwt:token长度指针，jwt:token的二级空指针，用户名
int makeToken(char **jwt,size_t *lenjwt,char *username);
//succsee:0 fail:-1
int verifyToken(MYSQL* pmysql,char *jwtToken,char *username,int fileID);

int signLoginToken(MYSQL *pDataSql,int netfd);
void initTimeWheel(timeWheel_t *timewheel,map_t *fdmap);
int addTimeWheel(timeWheel_t *timewheel,map_t *fdmap,int netfd);
int delTimeWheel(timeWheel_t *timewheel,map_t *fdmap,int netfd);
int updateTimeWheel(timeWheel_t *timewheel,map_t *fdmap,int netfd);
int rotateTimeWheel(timeWheel_t *timewheel,map_t *fdmap,int epfd);
//---------rbTree------------------
fdSolt_t *initRBTree();
void rightRotate(fdSolt_t *T,rbTreeNode_t *x);
void leftRotate(fdSolt_t *T,rbTreeNode_t *x);
void insertNode(fdSolt_t *T,int fd);
//查找插入结点的父节点
rbTreeNode_t *findInsertPlace(fdSolt_t *T,rbTreeNode_t *newp);
int checkUncleColor(rbTreeNode_t *father); 
void alterUncle(fdSolt_t *T,rbTreeNode_t *father,rbTreeNode_t *newp);
void inorder(fdSolt_t *T,rbTreeNode_t *root,int *arr,int *len);
void deleteNode(fdSolt_t *T,int fd);
rbTreeNode_t *findDeleteNode(fdSolt_t *T,int fd); 
rbTreeNode_t *findMinNode(fdSolt_t *T,rbTreeNode_t *p);
//用v替代u
void rbTransplant(fdSolt_t *T,rbTreeNode_t *u,rbTreeNode_t *v);

void RB_delete_fixup(fdSolt_t *T, rbTreeNode_t** x);
void deleteOrder(fdSolt_t *T,rbTreeNode_t *root);
#endif
