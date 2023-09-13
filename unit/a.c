#include <func.h>
typedef struct token_s{
      int length;
      int op;
      char token[1000];
      char username[1000];
  }token_t;

int main(void)
{
    int m=sizeof(token_t);
    printf("%d\n",m);
    return 0;
}

