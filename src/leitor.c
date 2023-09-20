#include "leitor.h"

#include "hashtables.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void  prosseguir(Leitor *l);
static char  espiar(Leitor *l);
static void  ignorar_espacos(Leitor *l);
static void  ler_ufs(Leitor *l, CPU_Specs *cpu_specs);
static void  ler_specs(Leitor *l, CPU_Specs *cpu_specs);
static char *ler_identificador(Leitor *l);
static void  ignorar_comentario(Leitor *l);
static void ler_dados(Leitor *l, uint32_t *memoria, uint32_t *idx, uint32_t *PC,
                      CPU_Specs *cpu_specs);
static void ler_texto(Leitor *l, uint32_t *memoria, uint32_t *idx);
static void ler_campos_r(Leitor *l, uint32_t *memoria, uint32_t *idx);
static void ler_campos_i_1(Leitor *l, uint32_t *memoria, uint32_t *idx);
static void ler_campos_i_2(Leitor *l, uint32_t *memoria, uint32_t *idx);
static void ler_campos_i_3(Leitor *l, uint32_t *memoria, uint32_t *idx);
static void ler_campos_j(Leitor *l, uint32_t *memoria, uint32_t *idx);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                          PUBLIC FUNCTIONS                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void
leitor_ler_arquivo(char *input, uint32_t *memoria, uint32_t *PC,
                   CPU_Specs *cpu_specs) {
  Leitor l;

  l.input    = input;
  l.ch       = '\0';
  l.pos      = 0;
  l.prox_pos = 0;

  prosseguir(&l);

  uint32_t idx = 0;

  // Faz a leitura do arquivo e salva as informações na memória.
  while (l.ch != '\0') {
    ignorar_espacos(&l);

    switch (l.ch) {
    case '/':
      if (espiar(&l) == '*') {
        prosseguir(&l);
        prosseguir(&l);
        ler_specs(&l, cpu_specs);
      } else {
        perror("Erro de sintaxe.");
        exit(EXIT_FAILURE);
      }
      break;

    case '.':
      if (isalpha(espiar(&l))) {
        prosseguir(&l);
        if (strncmp(ler_identificador(&l), "data", 4) == 0) {
          ler_dados(&l, memoria, &idx, PC, cpu_specs);
        } else if (strncmp(ler_identificador(&l), "text", 4) == 0) {
          ler_texto(&l, memoria, &idx);
        } else {
          perror("Erro de sintaxe.");
          exit(EXIT_FAILURE);
        }
      } else {
        perror("Erro de sintaxe.");
        exit(EXIT_FAILURE);
      }
      break;

    case '#': ignorar_comentario(&l); break;
    }
  }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                          PRIVATE FUNCTIONS                              *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static void
prosseguir(Leitor *l) {
  if (l->prox_pos < strlen(l->input)) {
    l->ch = l->input[l->prox_pos++];
  } else {
    l->ch = '\0';
  }
}

static void
ignorar_espacos(Leitor *l) {
  while (isspace(l->ch)) {
    prosseguir(l);
  }
}

static void
ignorar_comentario(Leitor *l) {
  while (l->ch != '\n') {
    prosseguir(l);
  }
}

static char
espiar(Leitor *l) {
  if (l->prox_pos < strlen(l->input)) {
    return l->input[l->prox_pos];
  } else {
    return '\0';
  }
}

static char *
ler_identificador(Leitor *l) {
  char *palavra;
  int   pos = 0;

  while (isalpha(l->ch)) {
    pos++;
    prosseguir(l);
  }

  palavra = malloc(pos + 1);
  strncpy(palavra, l->input + l->pos, pos);

  return palavra;
}

static void
ler_dados(Leitor *l, uint32_t *memoria, uint32_t *idx, uint32_t *PC,
          CPU_Specs *cpu_specs) {
  while (l->ch != '\0') {
    ignorar_espacos(l);
  }

  ler_ufs(l, cpu_specs);

  if (*memoria || *idx || *PC) {
  }
}

static void
ler_texto(Leitor *l, uint32_t *memoria, uint32_t *idx) {
  struct OpCodeMap *op_code_map = NULL;

  while (l->ch != '\0') {
    ignorar_espacos(l);
    if (isalpha(l->ch)) {
      char *palavra  = ler_identificador(l);
      op_code_map    = encontra_operacao(palavra, strlen(palavra));
      memoria[*idx] |= op_code_map->opcode << 26;

      switch (op_code_map->t) {
      case R: // add, sub, mul, div, and, or, not
        ler_campos_r(l, memoria, idx);
        break;
      case I:
        switch (op_code_map->opcode) {
        case 0x1: // addi
        case 0x3: // subi
          ler_campos_i_1(l, memoria, idx);
          break;
        case 0x9: // blt
        case 0xA: // bgt
        case 0xB: // beq
        case 0xC: // bne
          ler_campos_i_2(l, memoria, idx);
          break;
        case 0xE: // lw
        case 0xF: // sw
          ler_campos_i_3(l, memoria, idx);
          break;
        }
        break;
      case J: // j, exit
        if (op_code_map->opcode == 0x13) {
          ler_campos_j(l, memoria, idx);
        } else {
          memoria[*idx] |= 0x0;
        }
        break;
      }
    }
    idx++;
  }
}

static void
ler_specs(Leitor *l, CPU_Specs *cpu_specs) {}

static void
ler_campos_r(Leitor *l, uint32_t *memoria, uint32_t *idx) {
  // Ignora o espaço entre a operação e o primeiro registrador.
  ignorar_espacos(l);

  if (l->ch == 'r') {
    char   *identificador  = ler_identificador(l);
    uint8_t registrador    = encontra_reg(identificador, strlen(identificador));
    memoria[*idx]         |= registrador << 21;
  } else {
    perror("Erro de sintaxe.");
    exit(EXIT_FAILURE);
  }

  if (l->ch == ',') {
    prosseguir(l);
  } else {
    perror("Erro de sintaxe.");
    exit(EXIT_FAILURE);
  }

  // Ignora o espaço entre o primeiro e o segundo registrador.
  ignorar_espacos(l);

  if (l->ch == 'r') {
    char   *identificador  = ler_identificador(l);
    uint8_t registrador    = encontra_reg(identificador, strlen(identificador));
    memoria[*idx]         |= registrador << 16;
  } else {
    perror("Erro de sintaxe.");
    exit(EXIT_FAILURE);
  }

  if (l->ch == ',') {
    prosseguir(l);
  } else {
    perror("Erro de sintaxe.");
    exit(EXIT_FAILURE);
  }

  // Ignora o espaço entre o segundo e o terceiro registrador.
  ignorar_espacos(l);

  if (l->ch == 'r') {
    char   *identificador  = ler_identificador(l);
    uint8_t registrador    = encontra_reg(identificador, strlen(identificador));
    memoria[*idx]         |= registrador << 11;
  } else {
    perror("Erro de sintaxe.");
    exit(EXIT_FAILURE);
  }
}

static void
ler_campos_i_1(Leitor *l, uint32_t *memoria, uint32_t *idx) {
  // Ignora o espaço entre a operação e o primeiro registrador.
  ignorar_espacos(l);

  if (l->ch == 'r') {
    char   *identificador  = ler_identificador(l);
    uint8_t registrador    = encontra_reg(identificador, strlen(identificador));
    memoria[*idx]         |= registrador << 16;
  } else {
    perror("Erro de sintaxe.");
    exit(EXIT_FAILURE);
  }

  if (l->ch == ',') {
    prosseguir(l);
  } else {
    perror("Erro de sintaxe.");
    exit(EXIT_FAILURE);
  }

  // Ignora o espaço entre o primeiro e o segundo registrador.
  ignorar_espacos(l);

  if (l->ch == 'r') {
    char   *identificador  = ler_identificador(l);
    uint8_t registrador    = encontra_reg(identificador, strlen(identificador));
    memoria[*idx]         |= registrador << 21;
  } else {
    perror("Erro de sintaxe.");
    exit(EXIT_FAILURE);
  }

  if (l->ch == ',') {
    prosseguir(l);
  } else {
    perror("Erro de sintaxe.");
    exit(EXIT_FAILURE);
  }

  // Ignora o espaço entre o segundo registrador e o valor imediato.
  ignorar_espacos(l);

  if (isdigit(l->ch)) {
    char *palavra  = ler_identificador(l);
    memoria[*idx] |= atoi(palavra);
  } else {
    perror("Erro de sintaxe.");
    exit(EXIT_FAILURE);
  }
}

static void
ler_campos_i_2(Leitor *l, uint32_t *memoria, uint32_t *idx) {
  // Ignora o espaço entre a operação e o primeiro registrador.
  ignorar_espacos(l);

  if (l->ch == 'r') {
    char   *identificador  = ler_identificador(l);
    uint8_t registrador    = encontra_reg(identificador, strlen(identificador));
    memoria[*idx]         |= registrador << 21;
  } else {
    perror("Erro de sintaxe.");
    exit(EXIT_FAILURE);
  }

  if (l->ch == ',') {
    prosseguir(l);
  } else {
    perror("Erro de sintaxe.");
    exit(EXIT_FAILURE);
  }

  // Ignora o espaço entre o primeiro e o segundo registrador.
  ignorar_espacos(l);

  if (l->ch == 'r') {
    char   *identificador  = ler_identificador(l);
    uint8_t registrador    = encontra_reg(identificador, strlen(identificador));
    memoria[*idx]         |= registrador << 16;
  } else {
    perror("Erro de sintaxe.");
    exit(EXIT_FAILURE);
  }

  if (l->ch == ',') {
    prosseguir(l);
  } else {
    perror("Erro de sintaxe.");
    exit(EXIT_FAILURE);
  }

  // Ignora o espaço entre o segundo registrador e o valor imediato.
  ignorar_espacos(l);

  if (isdigit(l->ch)) {
    char *palavra  = ler_identificador(l);
    memoria[*idx] |= atoi(palavra);
  } else {
    perror("Erro de sintaxe.");
    exit(EXIT_FAILURE);
  }
}

static void
ler_campos_i_3(Leitor *l, uint32_t *memoria, uint32_t *idx) {
  // Ignorar o espaço entre a operação e o primeiro registrador.
  ignorar_espacos(l);

  if (l->ch == 'r') {
    char   *identificador  = ler_identificador(l);
    uint8_t registrador    = encontra_reg(identificador, strlen(identificador));
    memoria[*idx]         |= registrador << 16;
  } else {
    perror("Erro de sintaxe.");
    exit(EXIT_FAILURE);
  }

  if (l->ch == ',') {
    prosseguir(l);
  } else {
    perror("Erro de sintaxe.");
    exit(EXIT_FAILURE);
  }

  // Ignorar o espaço entre o primeiro registrador e o valor imediato.
  ignorar_espacos(l);

  if (isdigit(l->ch)) {
    char *palavra  = ler_identificador(l);
    memoria[*idx] |= atoi(palavra);
  } else {
    perror("Erro de sintaxe.");
    exit(EXIT_FAILURE);
  }

  if (l->ch == '(') {
    prosseguir(l);
  } else {
    perror("Erro de sintaxe.");
    exit(EXIT_FAILURE);
  }

  // Ignorar o espaço entre o valor imediato e o segundo registrador.
  ignorar_espacos(l);

  if (l->ch == 'r') {
    char   *identificador  = ler_identificador(l);
    uint8_t registrador    = encontra_reg(identificador, strlen(identificador));
    memoria[*idx]         |= registrador << 21;
  } else {
    perror("Erro de sintaxe.");
    exit(EXIT_FAILURE);
  }

  if (l->ch == ')') {
    prosseguir(l);
  } else {
    perror("Erro de sintaxe.");
    exit(EXIT_FAILURE);
  }
}

static void
ler_campos_j(Leitor *l, uint32_t *memoria, uint32_t *idx) {
  // Ignorar o espaço entre a operação e o valor imediato.
  ignorar_espacos(l);

  if (isdigit(l->ch)) {
    char *palavra  = ler_identificador(l);
    memoria[*idx] |= atoi(palavra);
  } else {
    perror("Erro de sintaxe.");
    exit(EXIT_FAILURE);
  }
}

static void
ler_ufs(Leitor *l, CPU_Specs *cpu_specs) {}
