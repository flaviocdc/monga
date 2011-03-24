/*line 10: missing arguments in function call*/

void printf(char[] s, ...);
char[] concat(char[] a, char[] b);

int main(){
	char[] a, b, c;
	a = "Hello";
	b = "World";
	printf(concat(a));
}
