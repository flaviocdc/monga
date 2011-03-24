/*line 10: cannot convert a float to an int*/

void printf(char[] s, ...);

int main(){

	int a;
	int b;
	
	a = 0.5;
	b = 2 * a;
	
	printf("%i", b);
}
