/* Números aleatórios, gera N números aleatórios entre 0 e 100,
   onde N é um número passado na linha de comando (default 1) */

int atoi(char[] s);
void printf(char[] s, ...);
int mod(int a, int b);

int IM, IA, IC, last;

float
gen_random(float max) {
    last = mod(last * IA + IC, IM);
    return( max * last / IM );
}

int
main(int argc, char[][] argv) {
    int N;
    float result;
    last = 42;
    IM = 139968;
    IA = 3877;
    IC = 29573;
    
    result = 0;

    if(argc == 2)
      N = atoi(argv[1]);
    else
      N = 1;

    N = N - 1;
    while(N) {
      result = gen_random(100.0);
    }
    printf("%.9f\n", result);
    return(0);
}

