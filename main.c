#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>

#include "parser.h"
#include "dirstack.h"

#define EXIT "exit"
#define QUIT "quit"
#define CHDIR  "cd"
#define POPD "popd"
#define PUSHD "pushd"
#define DIRS "dirs"

/* errors */
#define EXEC_ERR "Execution failed"

#define INCR 1

/* directory stack */
dir_stack_t ds;

void parse_error(const char * str, const int where)
{
	fprintf(stderr, "Parse error near %d: %s\n", where, str);
}

char *env_arg(word_t *arg) {
  char *arg_val, *arg_exp;

  arg_exp = (char *) calloc(1, sizeof(char));

  if (arg->next_part != NULL && !strcmp(arg->next_part->string, "=")) {
    /* set environment variable */
    setenv(arg->string, arg->next_part->next_part->string, 1);

    return arg_exp;
  }

  while (arg != NULL) {
    if (arg->expand) {
      /* if to be expanded, gen env var */
      arg_val = getenv(arg->string);
    }
    else
      arg_val = strdup(arg->string);


    if (arg_val != NULL) {
      arg_exp = (char *) realloc(arg_exp, (strlen(arg_exp) +
      strlen(arg_val) + 1) * sizeof(char));
      strcat(arg_exp, arg_val);
    }

    arg = arg->next_part;
  }

  return arg_exp;
}


int is_internal(simple_command_t *s) 
{
  /* check for internal */
  if (!strcmp(s->verb->string, EXIT) ||
      !strcmp(s->verb->string, QUIT) ||
      !strcmp(s->verb->string, CHDIR) ||
      !strcmp(s->verb->string, PUSHD) ||
      !strcmp(s->verb->string, POPD) ||
      !strcmp(s->verb->string, DIRS) ||
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
      printf("todo error handling\n");
      return 0;
    }
    /* insert the current directory onto the stack */
    push(&ds, getcwd(NULL, 256));
    
    if(chdir(env_arg(s->params))){
      printf("Directory does not exist!\n");
      pop(&ds);
    }
  }
  
  if(!strcmp(verb_str, DIRS)) {
    print(&ds);
  }
  if(!strcmp(verb_str, POPD)) {
    dir_str = pop(&ds);
    if(dir_str) {
      chdir(dir_str);
    }else{
      printf("todo error handling\n");
    }
  }
  
  return 0;
}

void get_args(simple_command_t *s, int *pargc, char ***pargv) {
  word_t *param;
  char *arg_exp;
  int size;

  param = s->params;
  *pargv = (char **) calloc(INCR, sizeof(char *));

  *pargc = 0;
  (*pargv)[0] = strdup(s->verb->string);
  size = INCR;

  /* build params list */
  while(param != NULL) {
    if (*pargc + 1 == size) {
      size += INCR;
      /* realloc if incoming out of bounds */
      *pargv = (char **) realloc(*pargv, (size + 1) * sizeof(char *));
    }
    if (strcmp((arg_exp = strdup(env_arg(param))), "")) {
      *pargc = *pargc + 1;
      (*pargv)[*pargc] = arg_exp;
    }

    param = param->next_word;
  }
  (*pargv)[*pargc+1] = NULL;
}

int run_simple(simple_command_t *s)
{
    int pid;
    int fdin, fdout, fderr; 
    int status;

    char *in_str, *out_str, *err_str, *verb_str;

    char **argv;
    int argc;

    /* if internal command */
    if (is_internal(s))
      return run_internal(s);

    /* get args */
    get_args(s, &argc, &argv);
    /* get the command's verb */
    verb_str = strdup(env_arg(s->verb));
    /* init */
    fdin = fdout = fderr = -1;

    if ((pid = fork()) < 0) {
      /* fork to call exec */
      perror("fork error\n");
      exit(-1);
    }
    else if (pid == 0) {
      if (s->in != NULL) {
        /* redirect input to s->in file */
        in_str = strdup(env_arg(s->in));
        fdin = open(in_str, O_RDONLY);
        dup2(fdin, STDIN_FILENO);
      }
      if (s->out != NULL) {
        /* redirect output to s-out file */
        out_str = strdup(env_arg(s->out));
        if (s->io_flags & IO_OUT_APPEND) {
          /* open output file for append */
          fdout = open(out_str, O_WRONLY | O_CREAT | O_APPEND, 0644);
        }
        else
          /* open output file for creation & overwrite */
          fdout = open(out_str, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        dup2(fdout, STDOUT_FILENO);
      }
      if (s->err != NULL) {
        err_str = strdup(env_arg(s->err));
        if (fdout == -1 || strcmp(out_str, err_str)) {
          /* if output file != error file */
          if (s->io_flags & IO_ERR_APPEND) {
            /* open error file for app */
            fderr = open(err_str, O_WRONLY | O_CREAT | O_APPEND, 0644);
          }
          else
            /* open error fiel for creat & overwr */
            fderr = open(err_str, O_WRONLY | O_CREAT, 0644);
        } else fderr = fdout;
        /* redirect err to fderr file descr */
        dup2(fderr, STDERR_FILENO);
      }


      /* execute cmd verb with vector of args */
      execvp(verb_str, argv);
      fprintf(stderr, "%s for \'%s\'\n", EXEC_ERR, s->verb->string);
      exit(-1);
    } 
    /* wait for child exit in parent */
    waitpid(pid, &status, 0);
    if (WIFEXITED(status)) {
      return WEXITSTATUS(status);
    }
    
    return -1;
}

int run_cmd(command_t *c)
{
    int ret1, ret2, ret;
    int status;
    int pid1, pid2;
    int fds[2];

    assert(c != NULL);

    if (c->op == OP_NONE) {
        /* no operator, assuming no composed commands */
        assert(c->cmd1 == NULL);
        assert(c->cmd2 == NULL);
        /* run simple command */
        return run_simple(c->scmd);
    }
    else {
        /* operator found, assuming no simple command */
        assert(c->scmd == NULL);

      switch (c->op) {
          case OP_SEQUENTIAL:
            /* run each command sequentially */
            run_cmd(c->cmd1);
            run_cmd(c->cmd2);
            return 0;
          case OP_PARALLEL:
            /* run commands in parallel */
            if ((pid1 = fork()) < 0) {
              perror("fork error");
              exit(-1);
            }
            else if (pid1 == 0) {
              if ((pid2 = fork()) < 0) {
                perror("fork error");
                exit(-1);
              }
              else if (pid2 == 0) {
                /* run cmd1 in child's child */
                ret1 = run_cmd(c->cmd1);
                exit(ret1);
              }
              /* run cmd2 in child */
              ret2 = run_cmd(c->cmd2);
              waitpid(pid2, NULL, 0);
              exit(ret2);
            }
            waitpid(pid1, NULL, 0);
            return ret1 | ret2;
          case OP_CONDITIONAL_ZERO:
            ret = run_cmd(c->cmd1);
            /* if cmd1 returned successfully, run cmd2 */
            if (ret == 0)
              ret = run_cmd(c->cmd2);
            return ret;
          case OP_CONDITIONAL_NZERO:
            ret = run_cmd(c->cmd1);
            /* run cmd2 only if cmd1 returned unsuccesfully */
            if (ret != 0)
              ret = run_cmd(c->cmd2);
            return ret;
          case OP_PIPE:
            if ((pid1 = fork()) < 0) {
              perror("fork error");
              exit(-1);
            }
            else if (pid1 == 0) {
              /* open pipe in child */
              if (pipe(fds) != 0) {
                perror("pipe error\n");
                return 1;
              }
              if ((pid2 = fork()) < 0) {
                perror("fork error");
                exit(-1);
              }
              else if (pid2 == 0) {
                /* close STDIN end of pipe */
                close(fds[0]);
                /* redirect output to pipe */
                dup2(fds[1], STDOUT_FILENO);
                /* run cmd1 in child's child */
                ret1 = run_cmd(c->cmd1);
                exit(ret1);
              }
              /* close STDOUT end of pipe */
              close(fds[1]);
              /* redirect input from pipe */
              dup2(fds[0], STDIN_FILENO);
              /* run cmd2 in child */
              ret2 = run_cmd(c->cmd2);
              waitpid(pid2, NULL, 0);
              exit(ret2);
            }
            waitpid(pid1, &status, 0);
            return status;
          default:
            assert(false);
      }
    }
  return 0;
}

int main(void)
{
  char *input;
  char cwd[128];
  command_t *root;
  
  /* initialize directory stack */
  init(&ds);

  for (;;) {
    root = NULL;
    /* get current directory */
    getcwd(cwd, 128);
    /* print shell prompt */
    input = (char*) readline(strcat(cwd, " #> "));
    /* we don't need empty lines in history */
    if (input){
      add_history(input);
    } else {
      return EXIT_FAILURE;
    }
    
    if (parse_line(input, &root)) {
      /* valid command */
      if (root != NULL)
        /* run command */
        run_cmd(root);
    }
    else {
      /* error parsing the command */
      printf("%s", input);
      fprintf(stderr, "Error parsing!\n");
    }

    /* free memory */
    free_parse_memory();
  }

  return EXIT_SUCCESS;
}