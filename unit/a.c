#include "head.h"
int main(void)
{
    fdSolt_t *tree=initRBTree();
    int m[16]={20,10,5,30,40,57,3,2,4,35,25,18,22,23,24,19};
    for(int i=0;i<16;i++)
    {
        insertNode(tree,m[i]);
    }
    inorder(tree,tree->root);
    printf("\n");

    for(int i=0;i<16;i++)
    {
        deleteNode(tree,m[i]);
        printf("i=%d m[i]=%d\n",i,m[i]);
    }
    printf("inorder:\n");
    inorder(tree,tree->root);
    return 0;
}
