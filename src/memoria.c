#include "memoria.h"
#include <stdint.h>
#include <stdio.h>

void teste(void) {
  printf("Teste\n");
  int add = 1;
  int reg_1 = 1;
  int reg_2 = 2;
  int reg_3 = 3;
  int extra = 10;

  uint32_t ins = 0;
  ins |= add << 26;
  ins |= reg_1 << 21;
  ins |= reg_2 << 16;
  ins |= reg_3 << 11;
  ins |= extra;
}
