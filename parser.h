#ifndef __PARSER_H
#define __PARSER_H

typedef enum {
  false,
  true
} bool;

/*
 * a string literal list
 * 
 * string literals can be made up of multiple strings
 * (next_part points to the next part of a string or
 * is NULL if there are no more parts)
 * 
 * some parts might need environment variable expansion (expand == true);
 * if that is the case, "string" points to the environment variable name
 * 
 * the next string literal is pointed to by next_word
 * (NULL if there are no more list elements)
 */

typedef struct word_t {
  const char *string;
  bool expand;
  struct word_t *next_part;
  struct word_t *next_word;
} word_t;


/*
 * describes a simple command
 * 
 * verb points to a single string literal (possibly made up of parts)
 * that is the executable name or the internal command name.
 * 
 * params points to a list of parameters (possibly none) in the order
 * they were entered in the command line.
 * 
 * in out and err point to the file names of the redirections for the command;
 * they can point to a single literal, no literal (the command does not
 * have that redirection) or multiple literals (the user entered multiple
 * redirections for a command; e.g. cat >out1 >out2). 
 * 
 * within any of these lists, the literals are in the original order.io_flags 
 * is used to specify special modes for redirection (e.g. appending)
 * 
 * some string literals can be found in both the out list and the err list
 * (those entered as "command &> out").
 * 
 * up points to the command_t structure that points to this simple_command_t
 * (up != NULL)
 */

#define IO_REGULAR	0x00
#define IO_OUT_APPEND	0x01
#define IO_ERR_APPEND	0x02

typedef struct {
  word_t *verb;
  word_t *params;
  word_t *in;
  word_t *out;
  word_t *err;
  int io_flags;
  struct command_t *up;
} simple_command_t;


/*
 * operators
 * 
 * OP_NONE means no operator
 * (scmd points to a simple command and cmd1 == cmd2 == NULL)
 * 
 * if any other operators are set
 * (scmd == NULL)
 * 
 * OP_DUMMY is a dummy value that can be used to count the number of operators
*/

typedef enum {
  OP_NONE,
  OP_SEQUENTIAL,
  OP_PARALLEL,
  OP_CONDITIONAL_ZERO,
  OP_CONDITIONAL_NZERO,
  OP_PIPE,
  OP_DUMMY
} operator_t;


/*
 * describes a command tree (a node of the parse tree)
 * 
 * if (op == OP_NONE)
 *   scmd != NULL
 *   cmd1 == cmd2 == NULL
 *   scmd points to a command to be executed
 * else
 *   scmd == NULL
 *   cmd1 != NULL
 *   cmd2 != NULL
 *   cmd1 op cmd2 must be executed, according to the rules for op
 * 
 * up points to the command_t that points to this structure
 * (the father of the current node in the parse tree)
 * The root of the tree has up == NULL
 * 
 */

typedef struct command_t {
  struct command_t *up;
  struct command_t *cmd1;
  struct command_t *cmd2;
  operator_t op;
  simple_command_t *scmd;
} command_t;

/*
 * this function must be defined inside the program.
 * it is called by the parser whenever a parse error occurs.
 * 
 * str describes the error.
 * where points to the location in the line where the error occured
 */

void parse_error(const char *str, const int where);

/*
 * this function is used to parse a line.
 * 
 * line must point to a string containing a line of input, terminated
 * by a newline and a null character.
 * (*root) must point to NULL 
 * ((*root) == NULL)
 * 
 * parse_line returns true if there was no error parsing the line
 * and false if there was an error parsing or the arguments were invalid
 * 
 * when parse_line returns true, (*root) points to the root of the parse tree
 * 
 * (*root) may still be NULL even if parse_line returns true
 * this may be because the line is empty (or contained just blanks)
 */

bool parse_line(const char *line, command_t **root);

/*
 * this function must be called to free the memory used by the parse tree
 */

void free_parse_memory();

/*
 * parser and lexer common internal stuff
*/

#ifdef __PARSER_H_INTERNAL_INCLUDE

/* 
 * generic pointer typedef
 */

typedef void* GenericPointer;

/*
 * internal structure used to populate the simple_command_t structure
 * 
 * the red_{i,o,e} fields hold the file names for the redirection
 * 
 * i - input
 * o - output
 * e - error
 *
 * red_flags hold the flags (overwrite, append)
 */ 

typedef struct {
  word_t *red_i;
  word_t *red_o;
  word_t *red_e;
  int red_flags;
} redirect_t;

/*
 * the lexical analyzer main function.
 * must be defined because bison expects it.
 */

int yylex();

/*
 * this function is called by parse_line to do the actual parsing
 * it starts the lexer and keeps track of it's internal buffers.
 */

void flex_parse_string(const char *str);

/* 
 * this function is used for memory management. 
 * it clears flex's buffers when called.
 */

void flex_free_buffers();

/*
 * this function adds a pointer to the memory pool.
 * must be called after every memory allocation
 * 
 * free_parse_memory uses this information to clear unused memory
 */

void add_to_memory_pool(const void *ptr);
#endif

#endif
