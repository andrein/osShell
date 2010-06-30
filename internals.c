#include "internals.h"
#include "dirstack.h"
#include "args.h"
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int is_internal(simple_command_t *s) 
{
  /* check for internal */
  if (!strcmp(s->verb->string, EXIT) ||
      !strcmp(s->verb->string, QUIT) ||
      !strcmp(s->verb->string, CHDIR) ||
      !strcmp(s->verb->string, PUSHD) ||
      !strcmp(s->verb->string, POPD) ||
      !strcmp(s->verb->string, DIRS) ||
      !strcmp(s->verb->string, ECHO) ||
      s->verb->next_part != NULL)
    return 1;

  return 0;
}

int run_internal(simple_command_t *s)
{
  char *dir_str, *verb_str;
  
  verb_str = strdup(env_arg(s->verb));
  if (strlen(verb_str) == 0)
    return 0;

  /* exit internal cmd */
  if (!strcmp(verb_str, EXIT) ||
      !strcmp(verb_str, QUIT))
    exit(0);
  
  /* cd internal cmd */
  if (!strcmp(verb_str, CHDIR)) {
    if (s->params == NULL) { // if cd has no parameters, change to $HOME directory
      chdir(getenv("HOME"));
      return 0;
    }
    /* change dir */
    dir_str = strdup(env_arg(s->params));
    if (chdir(dir_str) != 0) {
      perror("Error when changing directory\n");
      return 1;
    }
  }
  
  /* push directory onto the stack */
  if(!strcmp(verb_str, PUSHD)) {
    if (s->params == NULL){
      printf("Must insert directory name\n");
      return 1;
    }
    /* insert the current directory onto the stack */
    push(&ds, getcwd(NULL, 256));
    
    if(chdir(env_arg(s->params)) != 0){
      printf("Directory does not exist\n");
      pop(&ds);
    }
  }
  
  if(!strcmp(verb_str, DIRS)) {
    if (!print(&ds)){
      printf("Stack is empty.\n");
      return 1;
    }
  }
  if(!strcmp(verb_str, POPD)) {
    dir_str = pop(&ds);
    if(dir_str) {
      chdir(dir_str);
    }else{
      printf("Stack is empty.\n");
      return 1;
    }
  }
  if(!strcmp(verb_str, ECHO)) {
    if(!s->params->string) {
      printf("\n");
    }else{
      printf("%s \n", s->params->string);
    }
  }
  
  return 0;
}