#include <func.h>
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
fdSolt_t *initRBTree();
void rightRotate(fdSolt_t *T,rbTreeNode_t *x);
void leftRotate(fdSolt_t *T,rbTreeNode_t *x);
void insertNode(fdSolt_t *T,int fd);
//查找插入结点的父节点
rbTreeNode_t *findInsertPlace(fdSolt_t *T,rbTreeNode_t *newp);
int checkUncleColor(rbTreeNode_t *father); 
void alterUncle(fdSolt_t *T,rbTreeNode_t *father,rbTreeNode_t *newp);
void inorder(fdSolt_t *T,rbTreeNode_t *root);
void deleteNode(fdSolt_t *T,int fd);
rbTreeNode_t *findDeleteNode(fdSolt_t *T,int fd); 
