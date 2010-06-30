#ifndef __ARGS_H
#define __ARGS_H

#include "parser.h"

/* this function performs environment variable expansion */
char *env_arg(word_t *arg);
/* 
 * this function builds an array of parameters from a simple_command_t.
 * it is required because execvp() expects a char** argv as a parameter.
 */
void get_args(simple_command_t *s, int *pargc, char ***pargv);

#endif
