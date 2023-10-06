#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// Header dos módulos
#include "barramento.h"
#include "cpu.h"
#include "defs.h"
#include "hashtables.h"
#include "memoria.h"

// Define uma estrutura para armazenar informações sobre uma instrução no
// pipeline.
typedef struct _instrucao Instrucao;

// Define uma estrutura para armazenar informações sobre o status de um
// registrador.
typedef struct _status_registrador Status_Registrador;

// Define uma estrutura para armazenar informações sobre o status de instruções
// no pipeline.
typedef struct _status_instrucoes Status_Instrucoes;

// Define um tipo enumerado para representar os tipos de Unidades Funcionais
// (UFs).
typedef enum _tipo_uf { add, mul, inteiro } Tipo_UF;

// Define a estrutura de uma instrução no pipeline.
struct _instrucao {
  uint32_t instrucao;        // A instrução em si (32 bits).
  int      clocks_restantes; // Quantidade de ciclos de clock restantes para
                             // conclusão.
  tipo tipo;                 // O tipo de UF a que a instrução pertence.
  int  uf;                   // O índice da UF a que a instrução pertence.
  int  index;
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
  char busca;          // Indica se a instrução está na etapa de busca.
  char emissao;        // Indica se a instrução está na etapa de emissão.
  char leitura; // Indica se a instrução está na etapa de leitura de operandos.
  char execucao; // Indica se a instrução está na etapa de execução.
  char escrita;  // Indica se a instrução está na etapa de escrita.
};

global Instrucao *lista_emissao = NULL;    // Lista de instruções em emissão.
global Instrucao *lista_leitura = NULL;    // Lista de instruções em leitura.
global Instrucao *lista_executando = NULL; // Lista de instruções em execução.
global Instrucao *lista_escrita = NULL;    // Lista de instruções em escrita.

// Variáveis globais para armazenar listas de instruções em diferentes etapas do
// pipeline.

// Array para mapear tempos de clock para cada tipo de instrução.
global uint32_t ClockMap[17] = { 0 };

// Contadores globais para rastrear o uso de UFs.
global uint32_t add_usados = 0, mul_usados = 0, int_usados = 0;

// Registrador de programa (PC) global inicializado com o valor 100.
global int PC = 100;

// Estrutura que armazena as especificações da CPU.
global CPU_Specs _cpu_specs = { 0 };

// Banco de registradores global.
global Banco_Registradores banco_registradores = { 0 };

// Banco de UFs global.
global Banco_UF banco_uf = { 0 };

// Variável global que indica se a CPU está em execução (inicialmente definida
// como 1).
global int rodando = 1;

// Array global para armazenar o status de registradores (32 registradores no
// total).
global Status_Registrador status_registrador[32] = { 0 };

// Ponteiro global para um array de estruturas de status de instruções (alocado
// dinamicamente).
global Status_Instrucoes *status_instrucoes = NULL;

// Protótipos de funções internas.
internal void        adicionar_instrucao(uint32_t instrucao);
internal void        printar_scoreboard(void);
internal void        emitir(void);
internal void        leitura_operandos(void);
internal void        executar(void);
internal void        escrever(void);
internal void        mandar_ler(Instrucao *instrucao);
internal void        mandar_executar(Instrucao *instrucao);
internal void        mandar_escrever(Instrucao *instrucao);
internal void        printar_ufs(void);
internal void        printar_status_registradores(void);
internal void        printar_instrucoes(void);
internal void        atualizar_instrucoes(void);
internal const char *decodificar_instrucao(uint32_t instrucao);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                          PUBLIC FUNCTIONS                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void
scoreboard_inicializar(CPU_Specs *cpu_specs) {
  // Copia as especificações da CPU para uma variável global _cpu_specs
  _cpu_specs = *cpu_specs;

  // Mapeia os tempos de clock para cada tipo de instrução no array ClockMap
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

  // Aloca memória para as unidades funcionais (UFs) 'add', 'mul' e 'inteiro'
  banco_uf.add     = (UF *) malloc(sizeof(UF) * _cpu_specs.uf_add);
  banco_uf.mul     = (UF *) malloc(sizeof(UF) * _cpu_specs.uf_mul);
  banco_uf.inteiro = (UF *) malloc(sizeof(UF) * _cpu_specs.uf_int);

  // Aloca memória para a estrutura de status de instruções com base na
  // quantidade de instruções
  status_instrucoes = (Status_Instrucoes *) malloc(sizeof(Status_Instrucoes)
                                                   * _cpu_specs.qtd_instrucoes);

  lista_emissao =
      (Instrucao *) malloc(sizeof(Instrucao) * _cpu_specs.qtd_instrucoes);

  lista_leitura =
      (Instrucao *) malloc(sizeof(Instrucao) * _cpu_specs.qtd_instrucoes);

  lista_executando =
      (Instrucao *) malloc(sizeof(Instrucao) * _cpu_specs.qtd_instrucoes);

  lista_escrita =
      (Instrucao *) malloc(sizeof(Instrucao) * _cpu_specs.qtd_instrucoes);

  for (uint i = 0; i < _cpu_specs.qtd_instrucoes; i++) {
    lista_emissao[i].instrucao        = 0;
    lista_emissao[i].uf               = -1;
    lista_emissao[i].index            = -1;
    lista_emissao[i].tipo             = -1;
    lista_emissao[i].clocks_restantes = -1;

    lista_leitura[i].instrucao        = 0;
    lista_leitura[i].uf               = -1;
    lista_leitura[i].index            = -1;
    lista_leitura[i].tipo             = -1;
    lista_leitura[i].clocks_restantes = -1;

    lista_executando[i].instrucao        = 0;
    lista_executando[i].uf               = -1;
    lista_executando[i].index            = -1;
    lista_executando[i].tipo             = -1;
    lista_executando[i].clocks_restantes = -1;

    lista_escrita[i].instrucao        = 0;
    lista_escrita[i].uf               = -1;
    lista_escrita[i].index            = -1;
    lista_escrita[i].tipo             = -1;
    lista_escrita[i].clocks_restantes = -1;
  }

  Status_Instrucoes *instrucao = status_instrucoes;

  for (uint i = 0; i < _cpu_specs.qtd_instrucoes; i++) {
    // Inicializa o status de cada instrução
    strncpy(instrucao->instrucao, decodificar_instrucao(i), 128);

    instrucao->busca    = '-';
    instrucao->emissao  = '-';
    instrucao->leitura  = '-';
    instrucao->execucao = '-';
    instrucao->escrita  = '-';

    instrucao++;
  }
}

// A função representa o ciclo de execução principal de um programa. Ele busca,
// executa, gerencia dependências entre instruções e move o PC para a próxima
// instrução em um loop contínuo até que a condição rodando seja falsa.
void
rodar_programa(char *nome_saida) {
  uint32_t instrucao;

  // Enquanto o programa estiver em execução
  // while (rodando) {
  for (int i = 0; i < 10; i++) {
    // Busca a próxima instrução na memória usando o valor atual de PC
    instrucao = barramento_buscar_instrucao(PC);

    // Estágio de escrita, onde são concluídas as instruções anteriores e seus
    // resultados são registrados
    escrever();

    // Estágio de execução, onde as instruções são executadas e os resultados
    // são calculados
    executar();

    // Estágio de leitura de operandos, onde são identificadas as dependências
    // entre instruções
    leitura_operandos();

    if (instrucao) {
      // Adiciona a instrução atual à lista de instruções em execução (parte da
      // fase de busca)
      adicionar_instrucao(instrucao);
    }

    // Estágio de emissão, onde os resultados calculados são transmitidos para
    // as unidades funcionais
    emitir();

    // TODO: Mudar local do PC
    PC += 4; // Incrementa o PC em 4 bytes (Avança para a próxima instrução)

    // Imprime o estado do scoreboard (pode ser uma representação do estado das
    // UFs e registradores)
    printar_scoreboard();
  }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                          PRIVATE FUNCTIONS                              *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// A função realiza uma série de tarefas, incluindo a verificação de
// dependências de dados entre instruções, a marcação de registradores usados, a
// execução de instruções e a configuração das unidades funcionais com base no
// tipo de instrução.
internal void
leitura_operandos(void) {
  // Encontra a instrução que está sendo executada
  UF *add =
      banco_uf.add; // Inicializa um ponteiro UF para a unidade funcional 'add'.
  UF *mul =
      banco_uf.mul; // Inicializa um ponteiro UF para a unidade funcional 'mul'.
  UF *inteiro = banco_uf.inteiro; // Inicializa um ponteiro UF para a unidade
                                  // funcional 'inteiro'.

  Banco_Registradores registradores_usados = {
    0
  }; // Inicializa uma estrutura de registro para rastrear registradores usados.

  // Itera pelas UFs do tipo 'add' e verifica quais registradores estão sendo
  // usados.
  for (uint i = 0; i < _cpu_specs.uf_add; i++) {
    if (add->Fi > -1) {
      registradores_usados[add->Fi] = 1; // Marca o registrador como usado
    }
    add++;
  }

  // Itera pelas UFs do tipo 'mul' e verifica quais registradores estão sendo
  // usados.
  for (uint i = 0; i < _cpu_specs.uf_mul; i++) {
    if (mul->Fi > -1) {
      registradores_usados[mul->Fi] = 1; // Marca o registrador como usado
    }
    mul++;
  }

  // Itera pelas UFs do tipo 'inteiro' e verifica quais registradores estão
  // sendo usados.
  for (uint i = 0; i < _cpu_specs.uf_int; i++) {
    if (inteiro->Fi > -1) {
      registradores_usados[inteiro->Fi] = 1; // Marca o registrador como usado
    }
    inteiro++;
  }

  // Enquanto houver instruções na lista
  for (uint i = 0; i < _cpu_specs.qtd_instrucoes; i++) {
    // Encontra a instrução atual
    Instrucao *instrucao = lista_leitura + i;

    uint8_t opcode = instrucao->instrucao >> 26; // Extrai o opcode da instrução
    int8_t rd = -1, rs = -1,
           rt   = -1; // Inicializa registradores de destino, fonte 1 e fonte 2
    int16_t imm = -1, extra = -1; // Inicializa valores imediatos e extras
    int32_t end = -1;             // Inicializa um destino

    // Determina o tipo da instrução com base no opcode
    switch (OpCodeTipo[opcode]) {
    case R:      // Instrução do tipo R (registrador)
      rd = (instrucao->instrucao >> 11)
         & 0x1F; // Extrai o registrador de destino
      rs = (instrucao->instrucao >> 21)
         & 0x1F; // Extrai o registrador de fonte 1
      rt = (instrucao->instrucao >> 16)
         & 0x1F; // Extrai o registrador de fonte 2
      extra = instrucao->instrucao & 0x7FF; // Extrai informações extras
      break;
    case I:                                 // Instrução do tipo I (imediato)
      rt = (instrucao->instrucao >> 16)
         & 0x1F;                            // Extrai o registrador de destino
      rs = (instrucao->instrucao >> 21) & 0x1F; // Extrai o registrador de fonte
      imm = instrucao->instrucao & 0xFFFF;      // Extrai o valor imediato
      break;
    case J:                                     // Instrução do tipo J (salto)
      end = instrucao->instrucao & 0x3FFFFFF;   // Extrai o destino do salto
      break;
    default:
      perror(
          "Tipo de instrução não reconhecido"); // Imprime um erro se o tipo de
                                                // instrução não for reconhecido
      exit(EXIT_FAILURE); // Sai do programa com falha
    }

    // Verifica se o registrador 'rs' está sendo usado e passa para a próxima
    // instrução.
    if (banco_registradores[rs]) {
      continue;
    }

    // Verifica se o registrador 'rt' está sendo usado e passa para a próxima
    // instrução.
    if (opcode != 0xD && banco_registradores[rt]) {
      continue;
    }

    // Não há dependência RAW, então a instrução pode ser executada.
    mandar_executar(instrucao);

    // Dependendo do opcode, atualiza o status das unidades funcionais.
    // Se o opcode estiver na faixa de 0 a 3
    if (opcode < 4) {
      add_usados++; // Aumenta a contagem de unidades funcionais 'add' usadas
      add = banco_uf.add; // Reinicia o ponteiro para unidades funcionais 'add'

      // Enquanto houver unidades funcionais 'add'
      while (add) {
        // Se a unidade funcional não estiver ocupada, configura a UF para
        // execução.
        if (!add->busy) {
          add->busy     = 1;
          add->operacao = opcode;
          add->Fi       = rd;
          add->Fj       = 0;
          add->Fk       = 0;
          add->Qj       = 0;
          add->Qk       = 0;
          add->Rj       = 1;
          add->Rk       = 1;
          break;
        }

        add++;
      }
    }
    // Se o opcode estiver na faixa de 4 a 5
    else if (opcode < 6)
    {
      mul_usados++; // Aumenta a contagem de unidades funcionais 'mul' usadas
      mul = banco_uf.mul; // Reinicia o ponteiro para unidades funcionais 'mul'

      // Enquanto houver unidades funcionais 'mul'
      while (mul) {
        // Se a unidade funcional não estiver ocupada, configura a UF para
        // execução.
        if (!mul->busy) {
          mul->busy     = 1;
          mul->operacao = opcode;
          mul->Fi       = rd;
          mul->Fj       = 0;
          mul->Fk       = 0;
          mul->Qj       = 0;
          mul->Qk       = 0;
          mul->Rj       = 1;
          mul->Rk       = 1;
          break;
        }

        mul++;
      }
    }
    // Se o opcode não estiver nas faixas anteriores
    else
    {
      int_usados++; // Aumenta a contagem de unidades funcionais 'inteiro'
                    // usadas
      inteiro = banco_uf.inteiro; // Reinicia o ponteiro para unidades
                                  // funcionais 'inteiro'

      // Enquanto houver unidades funcionais 'inteiro'
      while (inteiro) {
        // Se a unidade funcional não estiver ocupada, configura a UF para
        // execução.
        if (!inteiro->busy) {
          inteiro->busy     = 1;
          inteiro->operacao = opcode;
          inteiro->Fi       = rd;
          inteiro->Fj       = 0;
          inteiro->Fk       = 0;
          inteiro->Qj       = 0;
          inteiro->Qk       = 0;
          inteiro->Rj       = 1;
          inteiro->Rk       = 1;
          break;
        }

        inteiro++;
      }
    }
  }
}

// A função é responsável por adicionar uma nova instrução à lista de emissão,
// onde as instruções aguardam para serem emitidas para execução.
internal void
adicionar_instrucao(uint32_t instrucao) {
  local_persist uint32_t instrucoes_emitidas = 0;

  Status_Instrucoes *status = status_instrucoes + instrucoes_emitidas;

  status->busca    = 'X';
  status->emissao  = '-';
  status->leitura  = '-';
  status->execucao = '-';
  status->escrita  = '-';

  // Aloca memória para um novo nó de instrução na lista de emissão
  Instrucao no;

  no.index = instrucoes_emitidas;

  // Preenche os campos do novo nó com informações da instrução
  no.instrucao = instrucao; // Armazena a instrução
  // Define os clocks restantes com base no opcode da instrução
  no.clocks_restantes = ClockMap[instrucao >> 26];
  // Determina o tipo da instrução com base no opcode
  no.tipo = OpCodeTipo[instrucao >> 26];

  // Adiciona o nó à lista de emissão
  lista_emissao[instrucoes_emitidas] = no;

  instrucoes_emitidas++;
}

// A função é responsável por imprimir o estado do scoreboard, incluindo o
// número de relógio atual(clock), o estado das instruções em diferentes
// estágios, o estado das unidades funcionais e o estado dos registradores.
internal void
printar_scoreboard(void) {
  // Variável estática persistente para manter o rastreamento do relógio
  local_persist uint32_t clock = 0;

  // Imprime o número do relógio atual
  printf("Clock: %u\n", clock);

  // Imprime o estado das instruções
  printar_instrucoes();

  // Estado das unidades funcionais (UFs)
  UF *add     = banco_uf.add;
  UF *mul     = banco_uf.mul;
  UF *inteiro = banco_uf.inteiro;

  // Imprime o estado das unidades funcionais
  printar_ufs();

  // Imprime o estado dos registradores
  printar_status_registradores();

  printf(
      "---------------------------------------------------------------------"
      "-----------------\n");

  // Incrementa o contador de relógio
  clock++;
}

// A função é responsável por emitir instruções para execução, considerando a
// disponibilidade de Unidades Funcionais (UFs) e evitando conflitos Write After
// Write (WAW) com instruções já em execução
internal void
emitir(void) {
  Banco_Registradores registradores_usados = { 0 };

  // Verifica os registradores usados pelas instruções em execução
  for (uint i = 0; i < _cpu_specs.qtd_instrucoes; i++) {
    Instrucao *instrucao = lista_executando + i;
    switch (instrucao->tipo) {
    case R:
      registradores_usados[(instrucao->instrucao >> 11) & 0x1F] = 1;
      break;
    case I:
      if ((instrucao->instrucao >> 26) < 4) {
        registradores_usados[(instrucao->instrucao >> 16) & 0x1F] = 1;
      } else {
        registradores_usados[(instrucao->instrucao >> 21) & 0x1F] = 1;
      }
      break;
    case J:
    default: break;
    }
  }

  // Emite as instruções para execução, considerando disponibilidade de UFs e
  // evitando WAW
  for (uint i = 0; i < _cpu_specs.qtd_instrucoes; i++) {
    Instrucao *instrucao = lista_emissao + i;

    uint8_t opcode = instrucao->instrucao >> 26;

    // Checa se UF disponível para a emissão da instrução
    if (opcode < 4 && add_usados < _cpu_specs.uf_add) {
      // UF de adição disponível
    } else if (opcode >= 4 && opcode < 6 && mul_usados < _cpu_specs.uf_mul) {
      // UF de multiplicação disponível
    } else if (opcode >= 6 && opcode < 16 && int_usados < _cpu_specs.uf_int) {
      // UF de inteiro disponível
    } else {
      // UF não disponível para a instrução, passa para a próxima
      continue;
    }

    // Checa se há WAW (escrever após escrever) para evitar conflitos
    switch (instrucao->tipo) {
    case R:
      if (!registradores_usados[(instrucao->instrucao >> 11) & 0x1F]) {
        // Não há dependências de escrita, pode emitir a instrução para execução
        mandar_ler(instrucao);
      }
      break;
    case I:
      if ((instrucao->instrucao >> 26) < 4) {
        if (!registradores_usados[(instrucao->instrucao >> 16) & 0x1F]) {
          // Não há dependências de escrita, pode emitir a instrução para
          // execução
          mandar_ler(instrucao);
        }
      } else {
        if (!registradores_usados[(instrucao->instrucao >> 21) & 0x1F]) {
          // Não há dependências de escrita, pode emitir a instrução para
          // execução
          mandar_ler(instrucao);
        }
      }
      break;
    case J: break;
    }
  }
}

// A função é responsável por escrever o resultado das instruções na lista de
// escrita no barramento de dados e liberar as unidades funcionais
// correspondentes para uso futuro.
internal void
escrever(void) {
  UF *add     = banco_uf.add;
  UF *mul     = banco_uf.mul;
  UF *inteiro = banco_uf.inteiro;

  Banco_Registradores registradores_usados = { 0 };

  // Itera sobre as unidades funcionais (UFs) de adição
  for (uint i = 0; i < _cpu_specs.uf_add; i++) {
    // Verifica se as UFs têm dependências de leitura (Qj e Qk)
    if (add->Qj) {
      registradores_usados[add->Qj] = 1; // Registra o uso do registrador Qj
    }
    if (add->Qk) {
      registradores_usados[add->Qk] = 1; // Registra o uso do registrador Qk
    }

    add++;
  }

  // Itera sobre as unidades funcionais (UFs) de multiplicação
  for (uint i = 0; i < _cpu_specs.uf_mul; i++) {
    // Verifica se as UFs têm dependências de leitura (Qj e Qk)
    if (mul->Qj) {
      registradores_usados[mul->Qj] = 1; // Registra o uso do registrador Qj
    }
    if (mul->Qk) {
      registradores_usados[mul->Qk] = 1; // Registra o uso do registrador Qk
    }

    mul++;
  }

  // Itera sobre as unidades funcionais (UFs) de inteiro
  for (uint i = 0; i < _cpu_specs.uf_int; i++) {
    // Verifica se as UFs têm dependências de leitura (Qj e Qk)
    if (inteiro->Qj) {
      registradores_usados[inteiro->Qj] = 1; // Registra o uso do registrador Qj
    }
    if (inteiro->Qk) {
      registradores_usados[inteiro->Qk] = 1; // Registra o uso do registrador Qk
    }

    inteiro++;
  }

  // Itera sobre as instruções na lista de escrita
  for (uint i = 0; i < _cpu_specs.qtd_instrucoes; i++) {
    Instrucao *instrucao = lista_escrita + i;

    uint8_t opcode = instrucao->instrucao >> 26;

    // Verifica o opcode para determinar o tipo de operação
    if (opcode < 4) { // Instruções de adição
      add = banco_uf.add;
      for (int i = 0; i < instrucao->uf; i++) {
        add++;
      }

      // Verifica se o registrador Fi da UF está sendo usado por outras
      // instruções
      if (registradores_usados[add->Fi]) {
        // Move para a próxima instrução na lista de escrita
        continue;
      }

      add->busy = 0; // A UF está livre para ser utilizada
      add_usados--;  // Decrementa o contador de UFs de adição em uso

      Status_Instrucoes *status = status_instrucoes + instrucao->index;

      status->escrita = '-';

      // Escreve o resultado no barramento de dados
      barramento_escrever_dado(0, banco_registradores[add->Fi]);
    } else if (opcode < 6) { // Instruções de multiplicação
      mul = banco_uf.mul;
      for (int i = 0; i < instrucao->uf; i++) {
        mul++;
      }

      // Verifica se o registrador Fi da UF está sendo usado por outras
      // instruções
      if (registradores_usados[mul->Fi]) {
        // Move para a próxima instrução na lista de escrita
        continue;
      }

      mul->busy = 0; // A UF está livre para ser utilizada
      mul_usados--;  // Decrementa o contador de UFs de multiplicação em uso

      Status_Instrucoes *status = status_instrucoes + instrucao->index;

      status->escrita = '-';

      // Escreve o resultado no barramento de dados
      barramento_escrever_dado(0, banco_registradores[mul->Fi]);
    } else if (opcode < 16) { // Instruções de inteiro
      inteiro = banco_uf.inteiro;
      for (int i = 0; i < instrucao->uf; i++) {
        inteiro++;
      }

      // Verifica se o registrador Fi da UF está sendo usado por outras
      // instruções
      if (registradores_usados[inteiro->Fi]) {
        // Move para a próxima instrução na lista de escrita
        continue;
      }

      inteiro->busy = 0; // A UF está livre para ser utilizada
      int_usados--;      // Decrementa o contador de UFs de inteiro em uso

      Status_Instrucoes *status = status_instrucoes + instrucao->index;

      status->escrita = '-';

      // Escreve o resultado no barramento de dados
      barramento_escrever_dado(0, banco_registradores[inteiro->Fi]);
    }
  }
}

// A função é responsável por executar as instruções na lista de execução.
internal void
executar(void) {
  // Itera sobre as instruções na lista de execução
  for (uint i = 0; i < _cpu_specs.qtd_instrucoes; i++) {
    Instrucao *instrucao = lista_executando + i;
    // Verifica se a instrução ainda possui ciclos de clock a serem executados
    if (instrucao->clocks_restantes > 0) {
      // Decrementa o contador de ciclos restantes para a instrução
      Status_Instrucoes *status = status_instrucoes + instrucao->index;

      status->leitura  = '-';
      status->execucao = 'X';

      instrucao->clocks_restantes--;
    } else {
      // TODO: Atualizar UF

      // Após a execução dos ciclos, a instrução está pronta para escrita
      // Chama a função para enviar a instrução para a escrita
      mandar_escrever(instrucao);
    }
  }
}

// A função é responsável por mover uma instrução da lista de emissão para a
// lista de leitura, indicando que a instrução está pronta para ser buscada e
// executada.
internal void
mandar_ler(Instrucao *instrucao) {
  for (uint i = 0; i < _cpu_specs.qtd_instrucoes; i++) {
    Instrucao *emissao = lista_emissao + i;

    if (emissao == instrucao) {
      // Remove a instrução da lista de emissão, pois está pronta para leitura

      status_instrucoes[i].emissao = '-';
      status_instrucoes[i].leitura = 'X';

      lista_emissao[i] = (Instrucao){ 0 };
      lista_leitura[i] = *instrucao;
    }
  }
}

// A função é responsável por mover uma instrução da lista de leitura para a
// lista de execução, indicando que a instrução está pronta para ser executada.
internal void
mandar_executar(Instrucao *instrucao) {
  for (uint i = 0; i < _cpu_specs.qtd_instrucoes; i++) {
    Instrucao *leitura = lista_leitura + i;

    if (leitura == instrucao) {
      // Remove a instrução da lista de leitura, pois está pronta para execução

      status_instrucoes[i].leitura  = '-';
      status_instrucoes[i].execucao = 'X';

      lista_leitura[i]    = (Instrucao){ 0 };
      lista_executando[i] = *instrucao;
    }
  }
}

// A função é responsável por mover uma instrução da lista de execução para a
// lista de escrita, indicando que a instrução está pronta para escrever seu
// resultado.
internal void
mandar_escrever(Instrucao *instrucao) {
  for (uint i = 0; i < _cpu_specs.qtd_instrucoes; i++) {
    Instrucao *executando = lista_executando + i;

    if (executando == instrucao) {
      // Remove a instrução da lista de execução, pois está pronta para escrita

      status_instrucoes[i].execucao = '-';
      status_instrucoes[i].escrita  = 'X';

      lista_executando[i] = (Instrucao){ 0 };
      lista_escrita[i]    = *instrucao;
    }
  }
}

// A função é responsável por imprimir o status das Unidades Funcionais (UFs) no
// formato de uma tabela.
internal void
printar_ufs(void) {
  UF *add     = banco_uf.add;
  UF *mul     = banco_uf.mul;
  UF *inteiro = banco_uf.inteiro;

  printf("\nName\tBusy\tOperation\tFi\tFj\tFk\tQj\tQk\tRj\tRk\n");

  printf(
      "------------------------------------------------------------------------"
      "-----------------\n");

  for (uint i = 0; i < _cpu_specs.uf_add; i++) {
    printf("ADD %u:\t%u\t%u\t\t%u\t%u\t%d\t%d\t%d\t%d\t%d|\n", i, add->busy,
           add->operacao, add->Fi, add->Fj, add->Fk, add->Qj, add->Qk, add->Rj,
           add->Rk);
    add++;
  }

  printf(
      "------------------------------------------------------------------------"
      "-----------------\n");

  for (uint i = 0; i < _cpu_specs.uf_mul; i++) {
    printf("MUL %u:\t%u\t%u\t\t%u\t%u\t%d\t%d\t%d\t%d\t%d|\n", i, mul->busy,
           mul->operacao, mul->Fi, mul->Fj, mul->Fk, mul->Qj, mul->Qk, mul->Rj,
           mul->Rk);
    mul++;
  }

  printf(
      "------------------------------------------------------------------------"
      "-----------------\n");

  for (uint i = 0; i < _cpu_specs.uf_int; i++) {
    printf("INT %u:\t%u\t%u\t\t%u\t%u\t%d\t%d\t%d\t%d\t%d|\n", i, inteiro->busy,
           inteiro->operacao, inteiro->Fi, inteiro->Fj, inteiro->Fk,
           inteiro->Qj, inteiro->Qk, inteiro->Rj, inteiro->Rk);
    inteiro++;
  }

  printf(
      "------------------------------------------------------------------------"
      "-----------------\n");
}

// A função é responsável por imprimir o status dos registradores, mostrando a
// UF associada a cada registrador e sua posição atual.
internal void
printar_status_registradores(void) {
  printf("\nRegistradores:\n\n");

  // Itera sobre os registradores
  for (uint i = 0; i < 32; i++) {
    // Obtém o nome da UF associada ao registrador
    char *uf = Tipo_UF_Nome[status_registrador[i].uf];
    // Imprime o status do registrador
    printf("R%u:\t%s\t%u\n", i, uf, status_registrador[i].pos);
  }
}

// A função é responsável por imprimir o status das instruções em cada etapa do
// pipeline, incluindo busca, emissão, leitura, execução e escrita.
internal void
printar_instrucoes(void) {
  printf(
      "+----------------------------------+---------+--------+-------+---------"
      "+--------+\n");
  printf(
      "| Instrução                        | Busca   | Emissão| Leitura| "
      "Execução| Escrita|\n");
  printf(
      "+----------------------------------+---------+--------+-------+---------"
      "+--------+\n");
  Status_Instrucoes *instrucao = status_instrucoes;

  for (uint i = 0; i < _cpu_specs.qtd_instrucoes; i++) {
    printf("| %32s | %c       | %c      | %c     | %c       | %c      |\n",
           instrucao->instrucao, instrucao->busca, instrucao->emissao,
           instrucao->leitura, instrucao->execucao, instrucao->escrita);
    instrucao++;
  }
}

internal void
atualizar_instrucoes(void) {
  // Status_Instrucoes *instrucao = status_instrucoes;
  //
  // for (uint i = 0; i < _cpu_specs.qtd_instrucoes; i++) {
  //   instrucao->busca    = 0;
  //   instrucao->emissao  = 0;
  //   instrucao->leitura  = 0;
  //   instrucao->execucao = 0;
  //   instrucao->escrita  = 0;
  //   instrucao++;
  // }
}

internal const char *
decodificar_instrucao(uint32_t i) {
  uint32_t instrucao = barramento_buscar_instrucao(PC + i * 4);

  char *saida;

  uint8_t opcode = instrucao >> 26;

  switch (OpCodeTipo[opcode]) {
  case R:
    if (opcode == 0x8) {
      saida = (char *) malloc(32);
      sprintf(saida, "%s r%u, r%u", OpNome[instrucao >> 26],
              (instrucao >> 11) & 0x1F, (instrucao >> 21) & 0x1F);
    } else {
      saida = (char *) malloc(32);
      sprintf(saida, "%s r%u, r%u, r%u", OpNome[instrucao >> 26],
              (instrucao >> 11) & 0x1F, (instrucao >> 21) & 0x1F,
              (instrucao >> 16) & 0x1F);
    }
    break;
  case I:
    if (opcode == 0xE || opcode == 0xF) {
      saida = (char *) malloc(32);
      sprintf(saida, "%s %u(r%u)", OpNome[instrucao >> 26], instrucao & 0xFFFF,
              (instrucao >> 16) & 0x1F);
    } else {
      saida = (char *) malloc(32);
      sprintf(saida, "%s r%u, r%u, %d", OpNome[instrucao >> 26],
              (instrucao >> 16) & 0x1F, (instrucao >> 21) & 0x1F,
              instrucao & 0xFFFF);
    }
    break;
  case J:
    if (opcode == 0x10) {
      saida = (char *) malloc(32);
      sprintf(saida, "%s", OpNome[instrucao >> 26]);
    } else {
      saida = (char *) malloc(32);
      sprintf(saida, "%s %u", OpNome[instrucao >> 26], instrucao & 0x3FFFFFF);
    }
    break;
  }

  return saida;
}
