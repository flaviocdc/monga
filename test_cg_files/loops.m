/* Loops aninhados */

void printf(char[] s, ...);

int
main() {
    int a, b, c, d, e, f, x, n;

    x = 0;

    n = 3;
    
    a = 0;
    while(a < n) {
      b = 0;
      while(b < n) {
        c = 0;
        while(c < n) {
          d = 0;
          while(d < n) {
            e  = 0;
            while(e < n) {
              f = 0;
              while(f < n) {
                x = x + 1;
                f = f + 1;
              }
              e = e + 1;
            }
            d = d + 1;
          }
          c = c + 1;
        }
        b = b + 1;
      }
      a = a + 1; 
    }
    printf("%d\n", x);
    return(0);
}

