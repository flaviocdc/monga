/*line 9: cannot compare a void value*/

void mul(int x, int y){ int z; z = x*y; }

int is_prime(int n) {    
	int count, r;
	count = 2;	
	while(count < n){		
		if(mul((n/count),count) == n){
			return 0;
		}
	}
	return 1;
}

int main(char[][] argv) {
    int n;
    n = atoi(argv[1]);
    
    printf("%ld\n", fib(n));
    return(0);
}