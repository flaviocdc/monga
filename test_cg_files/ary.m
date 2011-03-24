/* Acesso a array, aloca dois arrays de n inteiros,
   onde n é um número passado na linha de comando
   (default 1), e faz operações com eles. */

int atoi(char[] s);
int[] ialloc(int n);
void printf(char[] s, ...);
void ifree(int[] arr);

int
main(int argc, char[][] argv) {
    int i, k, n;
    int[] x, y;

    if(argc == 2)
      n = atoi(argv[1]);
    else
      n = 3;

    x = ialloc(n);
    y = ialloc(n);

    i = 0;
    while(i < n) {
      x[i] = i + 1;
      i = i + 1;
    }

    k = 0;
    while(k < 1000) {
      i = n - 1;
      while(i >= 0) {
        y[i] = y[i] + x[i];
        i = i - 1;
      }
      k = k + 1;
    }
    
    printf("%i %i\n", y[0], y[n-1]);

    ifree(x);
    ifree(y);

    return(0);
}
