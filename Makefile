all: compile test_parser checker test_scanner runtime.o

runtime.o: codegen.h runtime.c
	cc -c runtime.c

yacc: monga.y
	yacc -d monga.y

lex.yy.c: yacc

y.tab.h: yacc

lex.yy.c: y.tab.h monga.l
	flex monga.l

test_scanner: ast.h ast_pretty_printer.c y.tab.h test_scanner.c lex.yy.c y.tab.c
	cc -o test_scanner ast_pretty_printer.c test_scanner.c lex.yy.c y.tab.c

test_parser: ast.h symtab.h y.tab.h ast_pretty_printer.c test_parser.c symtab.c lex.yy.c y.tab.c
	cc -o test_parser ast_pretty_printer.c test_parser.c symtab.c lex.yy.c y.tab.c

checker: ast.h symtab.h y.tab.h ast_pretty_printer.c type_checker.c check_file.c symtab.c lex.yy.c y.tab.c
	cc -o check_file ast_pretty_printer.c type_checker.c check_file.c symtab.c lex.yy.c y.tab.c

compile: ast.h symtab.h codegen.h y.tab.h ast_pretty_printer.c type_checker.c codegen.c compile.c symtab.c lex.yy.c y.tab.c
	cc -o compile ast_pretty_printer.c type_checker.c codegen.c compile.c symtab.c lex.yy.c y.tab.c

clean:
	rm *.o
	rm y.tab.*
	rm lex.yy.c
	rm compile

