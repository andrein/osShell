#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "parser.h"
#include "args.h"
#include "internals.h"
#include "dirstack.h" 
#include "runner.h"

void parse_error(const char * str, const int where)
{
	fprintf(stderr, "Parse error near %d: %s\n", where, str);
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
    input = readline(strcat(cwd, " #> "));
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
    free(input);
  }

  return EXIT_SUCCESS;
}
