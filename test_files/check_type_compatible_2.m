/*line 11: expression cannot be of type void*/

void printf(char[] s, ...);

void func(){
	printf("do nothing");
}

int main(){
	int x;
	x = func();
}
