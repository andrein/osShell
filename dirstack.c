#include "dirstack.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void init(dir_stack_t *ds){
  ds->size = 0;
}

int push(dir_stack_t *ds, char *path){
  if (ds->size == STACKSIZE) // stack is full
    return -1;
  
  ds->dir[ds->size++] = path;
  return ds->size;
}

char* pop(dir_stack_t *ds){
  if (ds->size == 0) // stack is empty
    return NULL;
  
  return ds->dir[--ds->size];
}

int print(dir_stack_t *ds){
  int i;
  for (i=0; i<ds->size; i++)
    printf("%s\n", ds->dir[i]);
  return ds->size;
}
