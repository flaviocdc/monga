/*line 5: cannot declare an array inside an empty array*/

void printf(char[] s, ...);
         
int main() {

	int[][2] A;
	
	A[1][1] = 1;
	A[1][2] = 2;
	A[2][1] = 3;
	A[2][2] = 4;

	printf( "%6.2f %6.2f\n%6.2f %6.2f\n", A[0][0], A[0][1], A[1][0], A[1][1] );
    return 0;
}
