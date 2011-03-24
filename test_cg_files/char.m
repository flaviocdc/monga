int strlen(char[] s);
void printf(char[] s, ...);

int main() {
  char a, b;
  int i;
  char[] c;
  
  c = "fabio";

  i = strlen(c) - 1;
  while(i >= 0) {
    a = c[i];
    b = a + 1;
    c[i] = b;
    i = i - 1;
  }

  printf("%s\n", c);
}
