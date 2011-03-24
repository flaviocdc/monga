/*line 5: ... must be the last parameter of a function*/

void printf(char[] s, ...);

void fun(..., int a){
	printf("%d", a);
}
