#pragma once

#include "memoria.h"

#include <stdint.h>

typedef int               Registrador;
typedef struct _cpu_specs CPU_Specs;
typedef Registrador       Banco_Registradores[32];
typedef struct _uf        UF;
typedef struct _banco_uf  Banco_UF;

// Define uma estrutura para armazenar uma lista de instruções no pipeline.
typedef struct _lista_instrucoes Lista_Instrucoes;

// Define uma estrutura para armazenar informações sobre uma instrução no
// pipeline.
typedef struct _instrucao_no Instrucao_No;

// Define uma estrutura para armazenar informações sobre o status de um
// registrador.
typedef struct _status_registrador Status_Registrador;

// Define uma estrutura para armazenar informações sobre o status de instruções
// no pipeline.
typedef struct _status_instrucoes Status_Instrucoes;

// Define um tipo enumerado para representar os tipos de Unidades Funcionais
// (UFs).
typedef enum _tipo_uf { none = -1, add, mul, inteiro } Tipo_UF;

// Define a estrutura de uma instrução no pipeline.
struct _instrucao_no {
  uint32_t instrucao;        // A instrução em si (32 bits).
  int      clocks_restantes; // Quantidade de ciclos de clock restantes para
                             // conclusão.
  tipo          tipo;        // O tipo da instrução pertence.
  int           uf;          // O índice da UF a que a instrução pertence.
  Tipo_UF       tipo_uf;     // O tipo de UF a que a instrução pertence.
  Instrucao_No *proximo;     // Ponteiro para a próxima instrução na lista.
  int           index;
};

// Define uma estrutura para representar uma lista de instruções no pipeline.
struct _lista_instrucoes {
  Instrucao_No *cabeca; // Ponteiro para a primeira instrução da lista.
  Instrucao_No *fim;    // Ponteiro para a última instrução da lista.
};

// Define uma estrutura para armazenar informações sobre o status de um
// registrador.
struct _status_registrador {
  Tipo_UF uf;  // O tipo de UF associado ao registrador.
  int     pos; // A posição atual do registrador.
};

// Define uma estrutura para armazenar informações sobre o status de instruções
// no pipeline.
struct _status_instrucoes {
  char instrucao[128]; // A representação da instrução (texto).
  int  busca;          // Indica se a instrução está na etapa de busca.
  int  emissao;        // Indica se a instrução está na etapa de emissão.
  int leitura; // Indica se a instrução está na etapa de leitura de operandos.
  int execucao; // Indica se a instrução está na etapa de execução.
  int escrita;  // Indica se a instrução está na etapa de escrita.
};

struct _uf {
  uint8_t     operacao; // OpCode
  Registrador Fi;       // Destino
  Registrador Fj, Fk;   // Valores
  int         Qj, Qk;   // Unidades produtoras
  int         Rj, Rk;   // Flags de disponibilidade
  int         busy;     // Flag de ocupação
};

struct _banco_uf {
  UF *add;
  UF *mul;
  UF *inteiro;
};

struct _cpu_specs {
  uint32_t uf_add;
  uint32_t uf_mul;
  uint32_t uf_int;

  uint32_t clock_add;
  uint32_t clock_addi;
  uint32_t clock_sub;
  uint32_t clock_subi;
  uint32_t clock_mul;
  uint32_t clock_div;
  uint32_t clock_and;
  uint32_t clock_or;
  uint32_t clock_not;
  uint32_t clock_blt;
  uint32_t clock_bgt;
  uint32_t clock_beq;
  uint32_t clock_bne;
  uint32_t clock_jump;
  uint32_t clock_load;
  uint32_t clock_store;
  uint32_t clock_exit;

  uint32_t qtd_instrucoes;
};

void scoreboard_inicializar(CPU_Specs *cpu_specs);
void rodar_programa(char *nome_saida);
