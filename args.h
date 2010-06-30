#ifndef __ARGS_H
#define __ARGS_H

#include "parser.h"

/* 
 * this function performs environment variable expansion 
 * 
 * it takes it's input (arg), expands the environment variables where appropriate and 
 * returns a char* containing the expanded string
 * 
 */

char* env_arg(word_t *arg);

/* 
 * this function builds an array of parameters from a simple_command_t.
 * it is required because execvp() expects a char** argv as a parameter.
 * also useful whenever we need an easy to parse array of parameters.
 */

void get_args(simple_command_t *s, int *pargc, char ***pargv);

#endif
