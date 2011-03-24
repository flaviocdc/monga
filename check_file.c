#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
#include "symtab.h"
#include "y.tab.h"

extern char *filename;

int main(int argc, char** argv) {
  FILE *file;
  filename = argv[1];
  file = fopen(argv[1], "r");
  yyrestart(file);
  yyparse();
  check_prog();
  fprintf(stderr, "Ok\n");
} 
