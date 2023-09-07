#include <func.h>
#include "md5.h"
#define IP "192.168.50.129"                                                                     
#define PORT "1234"
#define READ_DATA_SIZE  1024
#define MD5_SIZE        16
#define MD5_STR_LEN     (MD5_SIZE * 2)

typedef struct train_s{
    int length;
    char data[1000];//上限值
}train_t;
int sign(int netfd,char *username);
int login(char *username,int netFd);
int recvn(int sockfd, void* buf, int lengthgth);
int getAndSendname(char *username,int netFd);
int makeOP(char *command,int len);
void printTips();
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

int Compute_file_md5(const char *file_path, char *value);
int clientRemove(const char *command,int sfd);
//1.输入：filename：文件名数组；length 文件长度指针，只在返回值为0时有效                         
//2.返回值：成功返回0，失败返回-1，文件不存在返回-2
//运行环境：当前目录
int FileExistOrSize(char *filename,int *length);
int putsClient(char *command,int sfd);
//返回值：成功返回0，失败返回-1
int clientPwd(int sfd);
int clientLs(int sfd);
int getClient(int netfd,char* fileName);
