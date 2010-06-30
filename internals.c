#include "internals.h"
#include "dirstack.h"
#include "args.h"
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <readline/history.h>

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
      !strcmp(s->verb->string, PWD) ||
      !strcmp(s->verb->string, HIST) ||
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
  if (!strcmp(verb_str, EXIT) || !strcmp(verb_str, QUIT)){
    /* write the command history to disk */
    write_history(NULL);
    /* exit gracefully */
    exit(0);
  }
  
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
  
  /* show directories in the stack */
  if(!strcmp(verb_str, DIRS)) {
    if (!print(&ds)){
      printf("Stack is empty.\n");
      return 1;
    }
  }
  
  /* pop directory off the stack */
  if(!strcmp(verb_str, POPD)) {
    dir_str = pop(&ds);
    if(dir_str) {
      chdir(dir_str);
    }else{
      printf("Stack is empty.\n");
      return 1;
    }
  }
  
  /* echo [params ...] just echoes the parameters back to the user */
  if(!strcmp(verb_str, ECHO)) {
    int argc, i;
    char **argv;
    get_args(s, &argc, &argv);
    for(i=1; i<=argc; i++) {
      printf("%s ", argv[i]);
    }
    printf("\n");
  }
  
  /* prints working directory */
  if(!strcmp(verb_str, PWD)) {
    printf("%s \n", getcwd(NULL, 256));
  }
  
  /* prints the command history */
  if(!strcmp(verb_str, HIST)){
    HIST_ENTRY **history;
    int i;
    
    if (history = history_list())
      for (i=0; history[i] != NULL; i++)
	printf ("%d %s\n", i, history[i]->line);
  }
    
  return 0;
}