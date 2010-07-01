#include "runner.h"
#include "args.h"
#include "internals.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <assert.h>
/* errors */
#define EXEC_ERR "Execution failed"

int run_simple(simple_command_t *s){
  int pid; // Process ID
  int fdin, fdout, fderr; // stdin, stdout, stderr file descriptors
  int status; // esit status

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
  /* init file descriptors */
  fdin = fdout = fderr = -1;

  if ((pid = fork()) < 0) {
    /* fork to call exec */
    perror("fork error\n");
    exit(-1);
  } else if (pid == 0) { /* if pid == 0 we are the child */
    /* perform redirections */
    if (s->in != NULL) {
      /* redirect input to s->in file */
      in_str = strdup(env_arg(s->in));
      fdin = open(in_str, O_RDONLY);
      dup2(fdin, STDIN_FILENO);
    }
    if (s->out != NULL) {
      /* redirect output to s->out file */
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
      /* redirect errors to s->err file */
      err_str = strdup(env_arg(s->err));
      if (fdout == -1 || strcmp(out_str, err_str)) {
	/* if output file != error file */
	if (s->io_flags & IO_ERR_APPEND) {
	  /* open error file in append mode */
	  fderr = open(err_str, O_WRONLY | O_CREAT | O_APPEND, 0644);
	}
	else
	  /* open error file for creat & overwrite */
	  fderr = open(err_str, O_WRONLY | O_CREAT, 0644);
      } else fderr = fdout;
      dup2(fderr, STDERR_FILENO);
    }

    /* execute cmd verb with vector of args */
    execvp(verb_str, argv);
    fprintf(stderr, "%s for \'%s\'\n", EXEC_ERR, s->verb->string);
    exit(-1);
  } 
  /* wait for child to exit in parent */
  waitpid(pid, &status, 0);
  if (WIFEXITED(status)) {
    return WEXITSTATUS(status);
  }
  
  return -1;
}

int run_cmd(command_t *c){
  int ret1, ret2, ret; // return values for the first and second command, and a return value for the command the first two form 
  int status; // exit status
  int pid1, pid2; // pids for the two commands
  int fds[2]; // file descriptors, used for pipelining.

  assert(c != NULL);

  if (c->op == OP_NONE) {
    /* no operator, assuming no compound commands */
    assert(c->cmd1 == NULL);
    assert(c->cmd2 == NULL);
    /* run simple command */
    return run_simple(c->scmd);
  }else {
    /* operator found, assuming this is a compound command */
    assert(c->scmd == NULL);

    /* check what kind of operator we have */
    switch (c->op) {
      
      case OP_SEQUENTIAL:
	/*
	 * run the commands sequentially.
	 * also known as synchronous operation.
	 */
	run_cmd(c->cmd1);
	run_cmd(c->cmd2);
	return 0;
	
      case OP_PARALLEL:
	/*
	* run the commands in parallel.
	* also known as asynchronous operation.
	*/
	if ((pid1 = fork()) < 0) {
	  perror("fork error");
	  exit(-1);
	}else if (pid1 == 0) {
	  /* we're in the child here */
	  if ((pid2 = fork()) < 0) {
	    perror("fork error");
	    exit(-1);
	  }else if (pid2 == 0) {
	    /* run cmd1 in grandson */
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
	/*
	 * run the second command only if the first one terminated successfully.
	 * i.e. it's exit status was 0
	 */	
	ret = run_cmd(c->cmd1);
	/* if cmd1 returned successfully, run cmd2 */
	if (ret == 0)
	  ret = run_cmd(c->cmd2);
	return ret;
	      
      case OP_CONDITIONAL_NZERO:
	/*
	 * run the second command only if the first one terminated unsuccesfully.
	 * i.e. it's exit status was different than 0.
	 */
	ret = run_cmd(c->cmd1);
	/* run cmd2 only if cmd1 returned unsuccesfully */
	if (ret != 0)
	  ret = run_cmd(c->cmd2);
	return ret;
	
      case OP_PIPE:
	/*
	 * the pipe connects the stdout from the first command to the stdout of the second.
	 */
	if ((pid1 = fork()) < 0) {
	  perror("fork error");
	  exit(-1);
	}else if (pid1 == 0) {
	  /* open pipe in child */
	  if (pipe(fds) != 0) {
	    perror("pipe error\n");
	    return 1;
	  }
	  if ((pid2 = fork()) < 0) {
	    perror("fork error");
	    exit(-1);
	  }else if (pid2 == 0) {
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
	/* 
	 * execution should never reach this point,
	 * really...
	 */
	assert(false);
    }
  }
  return 0;
}
