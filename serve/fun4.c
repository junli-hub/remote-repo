#include "threadPool.h"
#include "l8w8jwt/encode.h"
#include "l8w8jwt/decode.h"
int rotateTimeWheel(timeWheel_t *timewheel,map_t *fdmap,int epfd)
{
    train_t train;
    bzero(&train,sizeof(train));
    train.length=0;
    int cur=timewheel->curPointer;
    int netfd;
    for(int i=0;i<timewheel->timewheel[cur].size;i++)
    {
        netfd=timewheel->timewheel[cur].map[i];
        fdmap->map[netfd]=-1;
        send(netfd,&train,sizeof(train.length)+train.length,MSG_NOSIGNAL);
        epoll_ctl(epfd, EPOLL_CTL_DEL, netfd,NULL);
        printf("netfd:%d close\n",netfd);
        close(netfd);
    }
    timewheel->timewheel[cur].size=0;
    timewheel->lastPointer=cur;
    timewheel->curPointer=(cur+1)%TIMENUM;
    timewheel->time=time(NULL);
    return 0;
}
int updateTimeWheel(timeWheel_t *timewheel,map_t *fdmap,int netfd)
{
    delTimeWheel(timewheel,fdmap,netfd);
    addTimeWheel(timewheel,fdmap,netfd);
    return 0;
}
int delTimeWheel(timeWheel_t *timewheel,map_t *fdmap,int netfd)
{
    int flag=0;
    for(int i=0;i<timewheel->timewheel[fdmap->map[netfd]].size;i++)
    {
        if(flag==1)
        {
            timewheel->timewheel[fdmap->map[netfd]].map[i-1]
                =timewheel->timewheel[fdmap->map[netfd]].map[i];
        }
        if(timewheel->timewheel[fdmap->map[netfd]].map[i]==netfd)
        {
            flag=1;
        }
    }
    timewheel->timewheel[fdmap->map[netfd]].size--;
    fdmap->map[netfd]=-1;
    return 0;
}
int addTimeWheel(timeWheel_t *timewheel,map_t *fdmap,int netfd)
{
    fdmap->map[netfd]=timewheel->lastPointer;
    int len=timewheel->timewheel[timewheel->lastPointer].size;
    timewheel->timewheel[timewheel->lastPointer].map[len++]=netfd;
    timewheel->timewheel[timewheel->lastPointer].size=len;
    return 0;
}
void initTimeWheel(timeWheel_t *timewheel,map_t *fdmap)
{
    timewheel->curPointer=0;
    timewheel->lastPointer=TIMENUM-1;
    bzero(&timewheel->timewheel,sizeof(timewheel->timewheel));
    for(int i=0;i<MAP;i++)
    {
        fdmap->map[i]=-1;
    }
    timewheel->time=time(NULL);
}
int signLoginToken(MYSQL *pDataSql,int netfd)
{
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
    train_t train;
    bzero(&train,sizeof(train));
    size_t lenjwt;
    char *jwt;
    makeToken(&jwt,&lenjwt,username);
    train.length=lenjwt;
    memcpy(train.data,jwt,train.length);
    send(netfd,&train,sizeof(train.length)+train.length,MSG_NOSIGNAL);
    bzero(&train,sizeof(train));
    int fileID=findFileID(pDataSql,username);
    train.length=sizeof(fileID);
    memcpy(train.data,&fileID,train.length);
    send(netfd,&train,sizeof(train.length)+train.length,MSG_NOSIGNAL);
    return 0;
}
//返回值，成功返回0，失败返回-1
//传入：lenjwt:token长度指针，jwt:token的二级空指针，用户名
int makeToken(char **jwt,size_t *lenjwt,char *username)
{
    int userLen=strlen(username);
    struct l8w8jwt_encoding_params params;
    l8w8jwt_encoding_params_init(&params);

    params.alg = L8W8JWT_ALG_HS512;

    params.sub = "Netdisk";
    params.iss = "teamQiangGe";
    params.aud = "classmate";

    params.iat = 0;
    params.exp = 0x7fffffff; /* Set to expire after 10 minutes (600 seconds). */

    params.secret_key = (unsigned char*)username;
    params.secret_key_length = userLen;

    params.out = jwt;
    params.out_length = lenjwt;

    int r = l8w8jwt_encode(&params);
    if(r!=L8W8JWT_SUCCESS)
    {
        return -1;
    }
    return 0;
}
//succsee:0 fail:-1
int verifyToken(MYSQL* pmysql,char *jwtToken,char *username,int fileID)
{
    //校验fileID 
    char sql[2048]={0};
    sprintf(sql,"select * from fileInfo WHERE id=%d and user='%s' and tomb =0;",fileID,username);
    int qret = mysql_query(pmysql,sql);
    if(qret != 0){
        fprintf(stderr,"mysql_query 1:%s\n", mysql_error(pmysql));
        return -1;
    }
    int tag=1;
    MYSQL_RES * result = mysql_store_result(pmysql);
    if(mysql_num_rows(result)==0)
    {
        tag=0;       
    }
    //校验token
    struct l8w8jwt_decoding_params params;
    l8w8jwt_decoding_params_init(&params);

    params.alg = L8W8JWT_ALG_HS512;

    params.jwt = (char*)jwtToken;
    params.jwt_length = strlen(jwtToken);

    params.verification_key = (unsigned char*)username;
    params.verification_key_length = strlen(username);

    /*
     * Not providing params.validate_iss_length makes it use strlen()
     * Only do this when using properly NUL-terminated C-strings!
     */
    params.validate_iss = "teamQiangGe";
    params.validate_sub = "Netdisk";

    /* Expiration validation set to false here only because the above example token is already expired! */
    params.validate_exp = 0;
    params.exp_tolerance_seconds = 255;

    params.validate_iat = 0;
    params.iat_tolerance_seconds = 60;

    enum l8w8jwt_validation_result validation_result;

    int decode_result = l8w8jwt_decode(&params, &validation_result, NULL, NULL);

    if (decode_result == L8W8JWT_SUCCESS && validation_result == L8W8JWT_VALID && tag==1)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

