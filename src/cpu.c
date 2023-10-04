#include "cpu.h"

#include "barramento.h"
#include "defs.h"
#include "hashtables.h"
#include "memoria.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct _lista_instrucoes Lista_Instrucoes;
typedef struct _instrucao_no     Instrucao_No;

struct _instrucao_no {
  uint32_t      instrucao;
  int           clocks_restantes;
  tipo          tipo;
  Instrucao_No *proximo;
};

struct _lista_instrucoes {
  Instrucao_No *cabeca;
  Instrucao_No *fim;
};

global Lista_Instrucoes    lista_emissao    = { 0 };
global Lista_Instrucoes    lista_leitura    = { 0 };
global Lista_Instrucoes    lista_executando = { 0 };
global Lista_Instrucoes    lista_escrita    = { 0 };
global uint32_t            ClockMap[17]     = { 0 };
global uint32_t            add_usados = 0, mul_usados = 0, int_usados = 0;
global int                 PC                  = 100;
global CPU_Specs           _cpu_specs          = { 0 };
global Banco_Registradores banco_registradores = { 0 };
global Banco_UF            banco_uf            = { 0 };

internal void adicionar_instrucao(uint32_t instrucao);
internal void printar_scoreboard(void);
internal void emitir(void);
internal void checar_waw(void);
internal void leitura_operandos(void);
internal void checar_raw(void);
internal void executar(void);
internal void escrever(void);
internal void checar_war(void);
internal void mandar_ler(Instrucao_No *instrucao);
internal void mandar_executar(Instrucao_No *instrucao);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                          PUBLIC FUNCTIONS                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void
scoreboard_inicializar(CPU_Specs *cpu_specs) {
  _cpu_specs   = *cpu_specs;
  ClockMap[0]  = _cpu_specs.clock_add;
  ClockMap[1]  = _cpu_specs.clock_addi;
  ClockMap[2]  = _cpu_specs.clock_sub;
  ClockMap[3]  = _cpu_specs.clock_subi;
  ClockMap[4]  = _cpu_specs.clock_mul;
  ClockMap[5]  = _cpu_specs.clock_div;
  ClockMap[6]  = _cpu_specs.clock_and;
  ClockMap[7]  = _cpu_specs.clock_or;
  ClockMap[8]  = _cpu_specs.clock_not;
  ClockMap[9]  = _cpu_specs.clock_blt;
  ClockMap[10] = _cpu_specs.clock_bgt;
  ClockMap[11] = _cpu_specs.clock_beq;
  ClockMap[12] = _cpu_specs.clock_bne;
  ClockMap[13] = _cpu_specs.clock_jump;
  ClockMap[14] = _cpu_specs.clock_load;
  ClockMap[15] = _cpu_specs.clock_store;
  ClockMap[16] = _cpu_specs.clock_exit;

  banco_uf.add     = (UF *) malloc(sizeof(UF) * _cpu_specs.uf_add);
  banco_uf.mul     = (UF *) malloc(sizeof(UF) * _cpu_specs.uf_mul);
  banco_uf.inteiro = (UF *) malloc(sizeof(UF) * _cpu_specs.uf_int);
}

void
rodar_programa(char *nome_saida) {
  int rodando = 1;
  while (rodando) {                                       // 0x10 = EXIT
    uint32_t instrucao = barramento_buscar_instrucao(PC); // Busca inicial
    escrever();                                           // Escrita
    executar();                                           // Execução
    leitura_operandos();            // Leitura dos operandos
                                    // Busca e emite no mesmo clock
    adicionar_instrucao(instrucao); // Parte da busca
    emitir();                       // Emissão
    // TODO: Mudar local do PC
    PC        += 4; // Incrementa o PC
    instrucao  = barramento_buscar_instrucao(PC);
  }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                          PRIVATE FUNCTIONS                              *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

internal void
leitura_operandos(void) {
  // Encontra a instrução que está sendo executada
  UF *add     = banco_uf.add;
  UF *mul     = banco_uf.mul;
  UF *inteiro = banco_uf.inteiro;

  Banco_Registradores registradores_usados = { 0 };

  while (add) {
    if (add->Fi > -1) {
      registradores_usados[add->Fi] = 1;
    }
    add++;
  }

  while (mul) {
    if (mul->Fi > -1) {
      registradores_usados[mul->Fi] = 1;
    }
    mul++;
  }

  while (inteiro) {
    if (inteiro->Fi > -1) {
      registradores_usados[inteiro->Fi] = 1;
    }
    inteiro++;
  }

  // Checar RAW
  Instrucao_No *instrucao = lista_leitura.cabeca;
  while (instrucao) {
    uint8_t opcode = instrucao->instrucao >> 26;
    int8_t  rd = -1, rs = -1, rt = -1;
    int16_t imm = -1, extra = -1;
    int32_t end = -1;

    switch (OpCodeTipo[opcode]) {
    case R:
      rd    = (instrucao->instrucao >> 11) & 0x1F;
      rs    = (instrucao->instrucao >> 21) & 0x1F;
      rt    = (instrucao->instrucao >> 16) & 0x1F;
      extra = instrucao->instrucao & 0x7FF;
      break;
    case I:
      rt  = (instrucao->instrucao >> 16) & 0x1F;
      rs  = (instrucao->instrucao >> 21) & 0x1F;
      imm = instrucao->instrucao & 0xFFFF;
      break;
    case J: end = instrucao->instrucao & 0x3FFFFFF; break;
    default: perror("Tipo de instrução não reconhecido"); exit(EXIT_FAILURE);
    }

    if (banco_registradores[rs]) {
      instrucao = instrucao->proximo;
      continue;
    }

    if (opcode != 0xD && banco_registradores[rt]) {
      instrucao = instrucao->proximo;
      continue;
    }

    // Não tem RAW
    mandar_executar(instrucao);

    if (opcode == 0 || opcode == 2) {
      add_usados++;
      add = banco_uf.add;
      while (add) {
        if (add->busy == 0) {
          add->busy     = 1;
          add->operacao = opcode;
          add->Fi       = rd;
          add->Fj       = -1;
          add->Fk       = -1;
          add->Qj       = NULL;
          add->Qk       = NULL;
          add->Rj       = 1;
          add->Rk       = 1;
          break;
        }
      }
    }

    // banco_registradores[rd] = 1;
    // banco_registradores[rs] = 1;
    // banco_registradores[rt] = 1;

    instrucao = instrucao->proximo;
  }
}

internal void
adicionar_instrucao(uint32_t instrucao) {
  Instrucao_No *no     = (Instrucao_No *) malloc(sizeof(Instrucao_No));
  no->instrucao        = instrucao;
  no->proximo          = NULL;
  no->clocks_restantes = ClockMap[instrucao >> 26];
  no->tipo             = OpCodeTipo[instrucao >> 26];

  if (lista_emissao.cabeca == NULL) {
    lista_emissao.cabeca = no;
    lista_emissao.fim    = no;
  } else {
    lista_emissao.fim->proximo = no;
    lista_emissao.fim          = no;
  }

  printf("Instrução adicionada: %u\n", instrucao);
}

internal void
printar_scoreboard(void) {
  printf("Scoreboard:\n");
}

internal void
emitir(void) {
  Instrucao_No *instrucao_emitida    = lista_emissao.cabeca;
  Instrucao_No *instrucao_executando = lista_executando.cabeca;

  Banco_Registradores registradores_usados = { 0 };

  while (instrucao_executando) {
    switch (instrucao_executando->tipo) {
    case R:
      registradores_usados[(instrucao_executando->instrucao >> 11) & 0x1F] = 1;
      break;
    case I:
      if ((instrucao_executando->instrucao >> 26) < 4) {
        registradores_usados[(instrucao_executando->instrucao >> 16) & 0x1F] =
            1;
      } else {
        registradores_usados[(instrucao_executando->instrucao >> 21) & 0x1F] =
            1;
      }
      break;
    case J: break;
    }

    instrucao_executando = instrucao_executando->proximo;
  }

  while (instrucao_emitida) {
    uint8_t opcode = instrucao_emitida->instrucao >> 26;

    // Checa se UF disponível
    // Tá feio mas acho que tá certo
    if (opcode < 4 && add_usados < _cpu_specs.uf_add) {
    } else if (opcode < 6 && mul_usados < _cpu_specs.uf_mul) {
    } else if (opcode < 16 && int_usados < _cpu_specs.uf_int) {
    } else {
      instrucao_emitida = instrucao_emitida->proximo;
      continue;
    }

    // Checando WAW
    switch (instrucao_emitida->tipo) {
    case R:
      if (!registradores_usados[(instrucao_emitida->instrucao >> 11) & 0x1F]) {
        mandar_ler(instrucao_emitida);
      }
      break;
    case I:
      if ((instrucao_emitida->instrucao >> 26) < 4) {
        if (!registradores_usados[(instrucao_emitida->instrucao >> 16) & 0x1F])
        {
          mandar_ler(instrucao_emitida);
        }
      } else {
        if (!registradores_usados[(instrucao_emitida->instrucao >> 21) & 0x1F])
        {
          mandar_ler(instrucao_emitida);
        }
      }
      break;
    case J: break;
    }

    instrucao_emitida = instrucao_emitida->proximo;
  }
}

internal void
escrever(void) {}

internal void
executar(void) {}

internal void
mandar_ler(Instrucao_No *instrucao) {
  Instrucao_No *emitida = lista_emissao.cabeca;

  while (emitida->proximo != instrucao) {
    emitida = emitida->proximo;
  }

  emitida->proximo = instrucao->proximo;

  if (lista_leitura.cabeca == NULL) {
    lista_leitura.cabeca = instrucao;
    lista_leitura.fim    = instrucao;
  } else {
    lista_leitura.fim->proximo = instrucao;
    lista_leitura.fim          = instrucao;
  }
}

internal void
mandar_executar(Instrucao_No *instrucao) {
  Instrucao_No *leitura = lista_leitura.cabeca;

  while (leitura->proximo != instrucao) {
    leitura = leitura->proximo;
  }

  leitura->proximo = instrucao->proximo;

  if (lista_executando.cabeca == NULL) {
    lista_executando.cabeca = instrucao;
    lista_executando.fim    = instrucao;
  } else {
    lista_executando.fim->proximo = instrucao;
    lista_executando.fim          = instrucao;
  }
}
