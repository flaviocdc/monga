/* N�meros de Fibonacci, calcula o n-�simo no. de
   Fibonacci, onde n � um n�mero passado na linha de
   comando (default 1) */

int atoi(char[] s);
void printf(char[] s, ...);

int
fib(int n) {
    if (n < 2)
      return(1);
    else
      return(fib(n-2) + fib(n-1));
}

int
main() {
    int n;
    n = 10;
    printf("%i\n", fib(n));
    return(0);
}
