#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
#include "symtab.h"
#include "y.tab.h"

extern int yylineno;
extern char *filename;
extern FILE *outfile;
extern DeclrListNode *ast;

char* samples[] = {
  "./test_files/ack.m",
  "./test_files/ary.m",
  "./test_files/blocks.m",
  "./test_files/dangling.m",
  "./test_files/fibo.m",
  "./test_files/hello.m",
  "./test_files/loops.m",
  "./test_files/priority.m",
  "./test_files/random.m",
  "./test_files/sumcol.m",
  NULL
};

int main() {
  int i;
  int test_passed = 1;
  for(i = 0; test_passed && samples[i]; i++) {
    FILE *sample, *tmp1, *tmp2;
    char c1, c2;
    char temp1[L_tmpnam];
    char temp2[L_tmpnam];
    filename = samples[i];
    sample = fopen(samples[i],"r");
    yyrestart(sample);
    yylineno = 1;
    yyparse();
    tmp1 = tmpfile();
    outfile = tmp1;
    print_declrlist(0, ast);
    fflush(tmp1);
    rewind(tmp1);
    fclose(sample);
    yyrestart(tmp1);
    yylineno = 1;
    yyparse();
    tmp2 = tmpfile();
    outfile = tmp2;
    print_declrlist(0, ast);
    fflush(tmp2);
    rewind(tmp1);
    rewind(tmp2);
    while((c1=fgetc(tmp1))!= EOF && (c2=fgetc(tmp2))!= EOF)
	test_passed = (test_passed && c1 == c2);
    fclose(tmp1);
    fclose(tmp2);
  }
  if(test_passed) printf("Ok\n");
}

