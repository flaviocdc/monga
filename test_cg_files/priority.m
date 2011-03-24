/* Teste de prioridade */

void printf(char[] s, ...);

int main() {
  int a, b, c;
  a = 1;
  b = 2;
  c = 3;
  printf("%i\n", (!a && b + c / 3 * a || !c == 0 && a <= 3 || b >= 2));
}

