/*line 6: cannot compare an array*/

void printf(char[] s, ...);

int main(int[] x, int y){	
	if(x == y){
		printf("The arrays are equal");
	}	
	return 0;
}
