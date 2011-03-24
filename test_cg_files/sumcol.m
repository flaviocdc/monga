/* Soma uma lista de números obtidas do console */

int gets(char[] s);
int atoi(char[] s);
void printf(char[] s, ...);

int
main() {
    int sum;
    char[128] line;

    sum = 0;

    while(gets(line)) {
      sum = sum + atoi(line);
    }
    printf("%d\n", sum);
    return(0);
}
