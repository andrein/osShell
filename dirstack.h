#ifndef __DIRSTACK_H
#define __DIRSTACK_H

#define STACKSIZE 128

typedef struct {
  int size;
  char *dir[STACKSIZE];
} dir_stack_t;

void init(dir_stack_t *ds);
int push(dir_stack_t *ds, char *path);
char* pop(dir_stack_t *ds);
void print(dir_stack_t *ds);
#endif 
