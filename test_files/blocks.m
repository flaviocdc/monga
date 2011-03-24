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
      printf("%i\n", a);
    }
  }
  printf("%i\n", a);
  return(0);
}
