#ifndef __DIRSTACK_H
#define __DIRSTACK_H

#define STACKSIZE 128

/*
 * structure to hold a stack of directories.
 * size represents it's size.
 * dir is an array of STACKSIZE strings
 */
#include "internals.h"

typedef struct {
  int size;
  char *dir[STACKSIZE];
} dir_stack_t;

/* 
 * initializes the stack ds
 */

void init(dir_stack_t *ds);

/* 
 * pushes path to the stack pointed by ds.
 */

int push(dir_stack_t *ds, char *path);

/*
 * pops a directory from the stack pointed by ds.
 * 
 * returns a pointer to the string.
 * must be freed manually when done with it.
 */

char* pop(dir_stack_t *ds);

/*
 * displays the contents of the stack pointed by ds.
 */

int print(dir_stack_t *ds);

#endif 
