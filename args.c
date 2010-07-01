#include "args.h"
#include <stdlib.h>
#include <string.h>

char *env_arg(word_t *arg) {
  char *arg_val, *arg_exp;

  arg_exp = (char *) calloc(1, sizeof(char));

  /* if we have an '=' in our string, we treat it like an environment variable assignment */
  if (arg->next_part != NULL && !strcmp(arg->next_part->string, "=")) {
    /* set environment variable */
    setenv(arg->string, arg->next_part->next_part->string, 1);

    return arg_exp;
  }

  while (arg != NULL) {
    if (arg->expand) {
      /* if to expand is true, we perform environment variable expansion */
      arg_val = getenv(arg->string);
    }
    else
      arg_val = strdup(arg->string);

    if (arg_val != NULL) {
      /* allocate more memory for arg_val */
      arg_exp = (char *) realloc(arg_exp, (strlen(arg_exp) +
					    strlen(arg_val) + 1) * sizeof(char));
      /* concatenate the parts */
      strcat(arg_exp, arg_val);
    }

    arg = arg->next_part;
  }

  return arg_exp;
}

void get_args(simple_command_t *s, int *pargc, char ***pargv) {
  word_t *param;
  char *arg_exp;
  int size;

  param = s->params;
  *pargv = (char **) calloc(1, sizeof(char *));

  *pargc = 0;
  (*pargv)[0] = strdup(s->verb->string);
  size = 1;

  /* build params array */
  while(param != NULL) {
    if (*pargc + 1 == size) {
      size += 1;
      /* realloc if incoming out of bounds */
      *pargv = (char **) realloc(*pargv, (size + 1) * sizeof(char *));
    }
    /* add param to the argument list, if it is not empty */
    if (strcmp((arg_exp = strdup(env_arg(param))), "")) {
      *pargc = *pargc + 1;
      (*pargv)[*pargc] = arg_exp;
    }

    param = param->next_word;
  }
  /* terminate the argv array with a NULL */
  (*pargv)[*pargc+1] = NULL;
}