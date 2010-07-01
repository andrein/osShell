#ifndef __RUNNER_H
#define __RUNNER_H

#include "parser.h"

/* runs a simple_command_t */
int run_simple(simple_command_t *s);

/* runs a compound command */
int run_cmd(command_t *c);

#endif
