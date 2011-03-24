/*line 10: cannot convert a float to a char*/

void printf(char[] s, ...);

int main(){

	char a;
	char b;
	
	a = 0.5;
	b = 2349 * a;
	
	printf("%s", b);
}
