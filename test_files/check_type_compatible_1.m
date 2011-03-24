/*line 21: expression is of an incompatible type*/

int mult(int x, int y){
	int z;
	z = x*y;
	return z;
}

int main(){

	char[] x, y;
	int z;
	
	float b;
	int a;
	char c;
		
	x = "error";
	y = "here";
	
	z = mult(x, y);

	return 0;
	
}

