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
  if(file) {
    yyrestart(file);
    yyparse();
    check_prog();
    gen_prog("a.out", "a.out.defs");
  } else fprintf(stderr, "Source file does not exist\n");
} 
