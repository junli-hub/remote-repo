#include "head.h"
void inorder(fdSolt_t *T,rbTreeNode_t *root)
{
    if(T->nil==root)
    {
        return;
    }
    inorder(T,root->left);
    printf("%d ",root->fd);
    if(root->color==RED)
    {
        printf("color:RED\n");
    }
    else
    {
        printf("color:BLACK\n");
    }
    inorder(T,root->right);
}
fdSolt_t *initRBTree()
{
    fdSolt_t *p=(fdSolt_t *)calloc(1,sizeof(fdSolt_t));
    if(p==NULL)
    {
        printf("initRBTree 1\n");
        return NULL;
    }
    //nil 为叶子结点哨兵,表示指向NULL
    p->nil=(rbTreeNode_t *)malloc(sizeof(rbTreeNode_t));
    p->nil->right=NULL;
    p->nil->left=NULL;
    p->nil->parent=NULL;
    p->nil->color=BLACK;
    p->nil->fd=-1;
    p->root=p->nil;
    return p;
}
void leftRotate(fdSolt_t *T,rbTreeNode_t *x)
{
    //找到y,x与y交换位置
    rbTreeNode_t *y=x->right;
    x->right=y->left;
    if(y->left!=T->nil)
    {
        y->left->parent=x;
    }
    y->left=x;
    //y交接x父节点
    y->parent=x->parent;
    if(x->parent==T->nil)//如果x为根节点
    {
        T->root=y;
    }
    else if(x==x->parent->left)
    {
        x->parent->left=y;
    }
    else
    {
        x->parent->right=y;
    }
    x->parent=y;
}
void rightRotate(fdSolt_t *T,rbTreeNode_t *x)
{
    //找到y,x与y交换位置
    rbTreeNode_t *y=x->left;
    x->left=y->right;
    if(y->right!=T->nil)
    {
        y->right->parent=x;
    }
    y->right=x;
    //y交接x父节点
    y->parent=x->parent;
    if(x->parent==T->nil)//如果x为根节点
    {
        T->root=y;
    }
    else if(x->parent->left==x)
    {
        x->parent->left=y;
    }
    else
    {
        x->parent->right=y;
    }
    x->parent=y;
}
rbTreeNode_t *findInsertPlace(fdSolt_t *T,rbTreeNode_t *newp)
{
    rbTreeNode_t *cur=T->root;
    rbTreeNode_t *result=cur;
    while(cur!=T->nil)
    {
        if(cur->fd>newp->fd)
        {
            result=cur;
            cur=cur->left;
        }
        else if(cur->fd<newp->fd)
        {
            result=cur;
            cur=cur->right;
        }
        else
        {
            result=NULL;
            break;
        }
    }
    return result;
}
int checkUncleColor(rbTreeNode_t *father)
{
    rbTreeNode_t *grandFather=father->parent;
    if(grandFather->left==father)
    {
        return grandFather->right->color;
    }
    return grandFather->left->color;
}
void alterUncle(fdSolt_t *T,rbTreeNode_t *father,rbTreeNode_t *newp)
{
    if(father->color==BLACK)
    {
        return;
    }
    int colorUncle=checkUncleColor(father);
    if(colorUncle==RED)//父爷叔颜色反转
    {
        father->color=BLACK;
        rbTreeNode_t *grandFather=father->parent;
        grandFather->color=RED;
        if(grandFather->left==father)
        {
            grandFather->right->color=BLACK;
        }
        else
        {
            grandFather->left->color=BLACK;
        }
        if(grandFather==T->root)//爷为根则变黑后结束调整
        {
            grandFather->color=BLACK;
            return;
        }
        alterUncle(T,grandFather->parent,grandFather);//爷为新节点，递归调用更新函数
    }        
    else
    {
        rbTreeNode_t *grandFather=father->parent;
        if(grandFather->left==father && father->left==newp)
        {
            //交换父爷颜色
            ColorTyper changColor=father->color;
            father->color=grandFather->color;
            grandFather->color=changColor;
            //以爷为原点右旋
            rightRotate(T,grandFather);
        }
        else if(grandFather->left==father && father->right==newp)
        {
            //以父为原点左旋
            leftRotate(T,father);
            //交换父爷颜色（相对位置）
            ColorTyper changColor=newp->color;
            newp->color=grandFather->color;
            grandFather->color=changColor;
            //以爷为原点右旋
            rightRotate(T,grandFather);
        }
        else if(grandFather->right==father && father->right==newp)
        {
            //交换父爷颜色
            ColorTyper changColor=father->color;
            father->color=grandFather->color;
            grandFather->color=changColor;
            //以爷为原点左旋
            leftRotate(T,grandFather);
        }
        else
        {
            //以父为原点右旋
            rightRotate(T,father);
            //交换父爷颜色（相对位置）
            ColorTyper changColor=newp->color;
            newp->color=grandFather->color;
            grandFather->color=changColor;
            //以爷为原点左旋
            leftRotate(T,grandFather);
        }
    }
}
void insertNode(fdSolt_t *T,int fd)
{
    rbTreeNode_t *newp=(rbTreeNode_t *)calloc(1,sizeof(rbTreeNode_t));
    if(newp==NULL)
    {
        printf("insert error 1:calloc\n");
        return;
    }
    newp->left=T->nil;
    newp->right=T->nil;
    newp->color=RED;
    newp->fd=fd;
    if(T->root==T->nil)//树为空
    {
        T->root=newp;
        newp->parent=T->nil;
        newp->color=BLACK;//树根为黑色
        T->size++;
        return;
    }
    rbTreeNode_t *father=findInsertPlace(T,newp);
    if(father==NULL)
    {
        printf("insert error 2:same fd\n");
        return;
    }
    //插入节点的父节点为黑色:直接插入后返回
    if(father->fd>newp->fd)
    {
        father->left=newp;
        newp->parent=father;
    }
    else
    {
        father->right=newp;
        newp->parent=father;
    }
    T->size++;
    if(father->color==BLACK)
    {
        return;
    }
    //插入结点的父节点为红色：看叔父节点黑红
    alterUncle(T,father,newp);
}
rbTreeNode_t *findDeleteNode(fdSolt_t *T,int fd)
{
    if(T->root==T->nil) return T->nil;
    rbTreeNode_t *cur=T->root;
    while(cur!=T->nil)
    {
        if(cur->fd>fd)
        {
            cur=cur->left;
        }
        else if(cur->fd<fd)
        {
            cur=cur->right;
        }
        else
        {
            return cur;
        }
    }
    return T->nil;
}
void deleteNode(fdSolt_t *T,int fd)
{
    rbTreeNode_t *node=findDeleteNode(T,fd);
    if(node==T->nil) return;
    //如果删除的结点为红色
    if(node->color==RED)
    {

        return;
    }



}    
