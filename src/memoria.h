#pragma once

#include <stdint.h>

typedef struct _memoria memoria_t;
typedef struct _instrucao_r instrucao_r;
typedef struct _instrucao_i instrucao_i;
typedef struct _instrucao_j instrucao_j;
typedef struct _registrador reg_t;

struct _registrador {
  uint32_t valor;
};

struct _instrucao_r {
  uint8_t opcode;
  uint8_t orig_s;
  uint8_t orig_t;
  uint8_t dest;
  uint16_t extra;
};

struct _instrucao_i {
  uint8_t opcode;
  uint8_t orig_s;
  uint8_t orig_t;
  uint16_t imm;
};

struct _instrucao_j {
  uint8_t opcode;
  uint32_t address;
};

void teste(void);
