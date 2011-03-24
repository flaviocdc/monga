/* Blocos aninhados */

void printf(char[] s, ...);

int main() {
  int a;
  a = 2;
  {
    int a;
    a = 3;
    {
      int a;
      a = 0;
      printf("%i ", a);
    }
    printf("%i ", a);
  }
  printf("%i\n", a);
  return(0);
}
