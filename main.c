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
  char *input; // holds the input string.
  char cwd[128]; // the current working directory.
  command_t *root; // the root of the parse tree.
  
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
      continue;
    }
    
    if (parse_line(input, &root)) {
      /* is root valid? */
      if (root != NULL)
        /* run command */
        run_cmd(root);
    }
    else {
      /* error parsing the command */
      printf("%s\n", input);
      fprintf(stderr, "Error parsing!\n");
    }

    /* free memory */
    free_parse_memory();
    free(input);
  }

  return EXIT_SUCCESS;
}
