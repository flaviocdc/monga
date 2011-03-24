/* Exemplo de "dangling else". Identação mostra análise correta. */

void printf(char[] s, ...);

int main()
{
  int a;
  int b;
  a = 2;
  b = 2;
  if (a == 1)
    if (b == 2)
      printf("a is 1 and b is 2\n");
    else
      printf("a is not 1\n");
  return(0);
}
