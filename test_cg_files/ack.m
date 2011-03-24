/* Fun��o de Ackermann, calcula Ack(3, n), onde
   n � um no. passado na linha de comando, ou
   ack(3, 1) se n�o foi passado nenhum n�mero */

void printf(char[] s, ...);

int 
Ack(int M, int N) {
    if (M == 0) return( N + 1 );
    if (N == 0) return( Ack(M - 1, 1) );
    return( Ack(M - 1, Ack(M, (N - 1))) );
}

int
main(int argc, char[][] argv) {
    int n;
    n = 7;
    printf("Ack(3,%d): %d\n", n, Ack(3, n));
    return(0);
}


