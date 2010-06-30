#ifndef __INTERNALS_H
#define __INTERNALS_H

#include "parser.h"
#include "dirstack.h"

// internal functions
#define EXIT "exit"
#define QUIT "quit"
#define CHDIR  "cd"
#define POPD "popd"
#define PUSHD "pushd"
#define DIRS "dirs"
#define ECHO "echo"

//TODO
#define HIST "history"
#define PWD "pwd"

dir_stack_t ds;

/* checks if s is an internal command */
int is_internal(simple_command_t *s);

/* runs the internal command s */
int run_internal(simple_command_t *s);

#endif