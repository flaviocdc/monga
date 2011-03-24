/*line 3: array size cannot be specified here*/

void printf(char[] s, ...);

int[2][2] MatrixMultiply( int[][] A, int[][] B, int N ) {
	int[][] C;
	int i, j, k;
	i = 0;
	while (i < N) {
		j = 0;
    	while(j < N) {
        	C[i][j] = 0;
        	j = j + 1;
        }	
        i = i + 1;
    }    
	
    i = 0;
    while(i < N) {
    	j = 0;
    	while(j < N){
    		k = 0;    		
        	while(k < N) {
            	C[i][j] = C[i][j] + (A[i][k] * B[k][j]);
            	k = k + 1;
            }	
            j = j + 1;	
        }    	
        i = i + 1;
    }
}
         
int main() {

	int[][] A, C;
	A[1][1] = 1;
	A[1][2] = 2;
	A[2][1] = 3;
	A[2][2] = 4;

    C = MatrixMultiply( A, A, 2 );
    printf( "%6.2f %6.2f\n%6.2f %6.2f\n", C[0][0], C[0][1], C[1][0], C[1][1] );
    return 0;
}
