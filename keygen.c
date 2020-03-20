#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char** argv) {
  char* lenStr = argv[1];
  char* endPtr;
  srand(time(0));
  int len = strtol(lenStr, &endPtr, 10);
  char key[len];
  int i;
  for (i = 0; i < len; i++) {
    char letter = 'A' + rand() % 27;
    if (letter == 91)
      letter = 32;
    key[i] = letter;
    printf("%c", letter);
  }
  // key[len] = 0;
  printf("\n");
  return 0;
}