#pragma once

#include <stdint.h>

typedef enum { R, I, J } tipo;
typedef struct _instrucao ins_t;
typedef struct _registrador reg_t;

struct _registrador {
  uint32_t valor;
};

struct _instrucao {
  tipo tipo; // R | I | J
  uint32_t valor;
};

void teste(void);
