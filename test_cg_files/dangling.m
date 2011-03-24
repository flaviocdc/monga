/* Exemplo de "dangling else". Identação mostra análise correta. */

void printf(char[] s, ...);

int main()
{
  int a;
  int b;
  a = 1;
  b = 1;
  if (a == 1)
    if (b == 2)
      printf("a is 1 and b is 2\n");
    else
      printf("a is 1 and b is not 2\n");
  return(0);
}
