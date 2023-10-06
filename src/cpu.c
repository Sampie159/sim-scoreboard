#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// Header dos módulos
#include "barramento.h"
#include "cpu.h"
#include "defs.h"
#include "hashtables.h"
#include "memoria.h"

// Variáveis globais para armazenar listas de instruções em diferentes etapas do
// pipeline.
global Lista_Instrucoes lista_emissao    = { 0 };
global Lista_Instrucoes lista_leitura    = { 0 };
global Lista_Instrucoes lista_executando = { 0 };
global Lista_Instrucoes lista_escrita    = { 0 };

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

// Variável estática persistente para manter o rastreamento do relógio
global uint32_t clock = 0;

// Protótipos de funções internas.
internal void        adicionar_instrucao(uint32_t instrucao);
internal void        printar_scoreboard(void);
internal void        emitir(void);
internal void        leitura_operandos(void);
internal void        executar(void);
internal void        escrever(void);
internal void        mandar_ler(Instrucao_No *instrucao);
internal void        mandar_executar(Instrucao_No *instrucao);
internal void        mandar_escrever(Instrucao_No *instrucao);
internal void        printar_ufs(void);
internal void        printar_status_registradores(void);
internal void        printar_instrucoes(void);
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

  Status_Instrucoes *instrucao = status_instrucoes;

  for (uint i = 0; i < _cpu_specs.qtd_instrucoes; i++) {
    // Inicializa o status de cada instrução
    strncpy(instrucao->instrucao, decodificar_instrucao(i), 128);

    instrucao->busca    = 0;
    instrucao->emissao  = 0;
    instrucao->leitura  = 0;
    instrucao->execucao = 0;
    instrucao->escrita  = 0;

    instrucao++;
  }

  for (uint i = 0; i < 32; i++) {
    status_registrador[i].uf = none;
  }
}

// A função representa o ciclo de execução principal de um programa. Ele busca,
// executa, gerencia dependências entre instruções e move o PC para a próxima
// instrução em um loop contínuo até que a condição rodando seja falsa.
void
rodar_programa(char *nome_saida) {
  uint32_t instrucao;

  // Enquanto o programa estiver em execução
  // while (rodando)
  for (int i = 0; i < 20; i++) {
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

  // Checa a dependência RAW (Read-After-Write) nas instruções.
  Instrucao_No *instrucao = lista_leitura.cabeca;

  // Enquanto houver instruções na lista
  while (instrucao != NULL) {
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
      instrucao = instrucao->proximo;
      continue;
    }

    // Verifica se o registrador 'rt' está sendo usado e passa para a próxima
    // instrução.
    if (opcode != 0xD && banco_registradores[rt]) {
      instrucao = instrucao->proximo;
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

    // Move para a próxima instrução.
    instrucao = instrucao->proximo;
  }
}

// A função é responsável por adicionar uma nova instrução à lista de emissão,
// onde as instruções aguardam para serem emitidas para execução.
internal void
adicionar_instrucao(uint32_t instrucao) {
  local_persist uint32_t instrucoes_emitidas = 0;

  Status_Instrucoes *status = status_instrucoes + instrucoes_emitidas;

  status->busca = clock;

  // Aloca memória para um novo nó de instrução na lista de emissão
  Instrucao_No *no = (Instrucao_No *) malloc(sizeof(Instrucao_No));

  no->index = instrucoes_emitidas++;

  // Preenche os campos do novo nó com informações da instrução
  no->instrucao = instrucao;     // Armazena a instrução
  no->proximo   = NULL;          // Inicializa o próximo nó como NULL
  no->clocks_restantes =
      ClockMap[instrucao >> 26]; // Define os clocks restantes com base no
                                 // opcode da instrução
  // Determina o tipo da instrução com base no opcode
  no->tipo    = OpCodeTipo[instrucao >> 26];
  no->tipo_uf = Tipos_UF[instrucao >> 26];

  // Verifica se a lista de emissão está vazia
  if (lista_emissao.cabeca == NULL) {
    // Se estiver vazia, o novo nó se torna a cabeça e o fim da lista
    lista_emissao.cabeca = no;
    lista_emissao.fim    = no;
  } else {
    // Se não estiver vazia, adiciona o novo nó no final da lista
    lista_emissao.fim->proximo = no;
    lista_emissao.fim          = no; // Atualiza o fim da lista
  }
}

// A função é responsável por imprimir o estado do scoreboard, incluindo o
// número de relógio atual(clock), o estado das instruções em diferentes
// estágios, o estado das unidades funcionais e o estado dos registradores.
internal void
printar_scoreboard(void) {
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
  Instrucao_No *instrucao_emitida = lista_emissao.cabeca;

  // Emite as instruções para execução, considerando disponibilidade de UFs e
  // evitando WAW
  while (instrucao_emitida != NULL) {
    uint8_t opcode = instrucao_emitida->instrucao >> 26;

    // Checa disponibilidade de UFs
    int livre = (opcode < 4 && add_usados < _cpu_specs.uf_add)
              + (opcode >= 4 && opcode < 6 && mul_usados < _cpu_specs.uf_mul)
              + (opcode >= 6 && opcode < 16 && int_usados < _cpu_specs.uf_int);

    if (!livre) {
      // UF não disponível para a instrução, passa para a próxima
      instrucao_emitida = instrucao_emitida->proximo;
      continue;
    }

    // Checa se há WAW (escrever após escrever) para evitar conflitos
    switch (instrucao_emitida->tipo) {
    case R: {
      int destino = (instrucao_emitida->instrucao >> 11) & 0x1F;
      if (status_registrador[destino].uf == none) {
        // Não há dependências de escrita, pode emitir a instrução para
        // execução

        status_registrador[destino].uf  = instrucao_emitida->tipo_uf;
        status_registrador[destino].pos = instrucao_emitida->uf;

        mandar_ler(instrucao_emitida);
      }
    } break;
    case I:
      if ((instrucao_emitida->instrucao >> 26) < 4) {
        int destino = (instrucao_emitida->instrucao >> 16) & 0x1F;
        if (status_registrador[destino].uf == none) {
          // Não há dependências de escrita, pode emitir a instrução para
          // execução

          status_registrador[destino].uf  = instrucao_emitida->tipo_uf;
          status_registrador[destino].pos = instrucao_emitida->uf;

          mandar_ler(instrucao_emitida);
        }
      } else {
        mandar_ler(instrucao_emitida);
      }
      break;
    case J: break;
    }

    instrucao_emitida = instrucao_emitida->proximo;
  }
}

// A função é responsável por escrever o resultado das instruções na lista de
// escrita no barramento de dados e liberar as unidades funcionais
// correspondentes para uso futuro.
internal void
escrever(void) {
  Instrucao_No *instrucao = lista_escrita.cabeca;

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
  while (instrucao != NULL) {
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
        instrucao = instrucao->proximo; // Move para a próxima instrução na
                                        // lista de escrita
        continue;
      }

      add->busy = 0; // A UF está livre para ser utilizada
      add_usados--;  // Decrementa o contador de UFs de adição em uso

      // Status_Instrucoes *status = status_instrucoes + instrucao->index;

      // Escreve o resultado no barramento de dados
      status_registrador[add->Fi].uf  = none;
      status_registrador[add->Fi].pos = -1;

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
        instrucao = instrucao->proximo;
        continue;
      }

      mul->busy = 0; // A UF está livre para ser utilizada
      mul_usados--;  // Decrementa o contador de UFs de multiplicação em uso

      // Status_Instrucoes *status = status_instrucoes + instrucao->index;

      // Escreve o resultado no barramento de dados
      status_registrador[mul->Fi].uf  = none;
      status_registrador[mul->Fi].pos = -1;

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
        instrucao = instrucao->proximo;
        continue;
      }

      inteiro->busy = 0; // A UF está livre para ser utilizada
      int_usados--;      // Decrementa o contador de UFs de inteiro em uso

      // Status_Instrucoes *status = status_instrucoes + instrucao->index;

      // status->escrita = '-';

      // Escreve o resultado no barramento de dados
      status_registrador[inteiro->Fi].uf  = none;
      status_registrador[inteiro->Fi].pos = -1;

      barramento_escrever_dado(0, banco_registradores[inteiro->Fi]);
    }

    // Move para a próxima instrução na lista de escrita
    instrucao = instrucao->proximo;
  }
}

// A função é responsável por executar as instruções na lista de execução.
internal void
executar(void) {
  Instrucao_No *instrucao = lista_executando.cabeca;

  // Itera sobre as instruções na lista de execução
  while (instrucao != NULL) {
    // Verifica se a instrução ainda possui ciclos de clock a serem executados
    if (instrucao->clocks_restantes > 0) {
      // Decrementa o contador de ciclos restantes para a instrução
      Status_Instrucoes *status = status_instrucoes + instrucao->index;

      // status->leitura  = '-';
      status->execucao = clock;

      instrucao->clocks_restantes--;
    } else {
      // TODO: Atualizar UF

      // Após a execução dos ciclos, a instrução está pronta para escrita
      // Chama a função para enviar a instrução para a escrita
      mandar_escrever(instrucao);
    }

    // Avança para a próxima instrução na lista de execução
    instrucao = instrucao->proximo;
  }
}

// A função é responsável por mover uma instrução da lista de emissão para a
// lista de leitura, indicando que a instrução está pronta para ser buscada e
// executada.
internal void
mandar_ler(Instrucao_No *instrucao) {
  Instrucao_No *emitida = lista_emissao.cabeca;

  Status_Instrucoes *status = status_instrucoes + instrucao->index;
  status->emissao           = clock;

  // Verifica se a instrução a ser lida é a primeira na lista de emissão
  if (emitida == instrucao) {
    // Remove a instrução da lista de emissão, pois está pronta para leitura
    lista_emissao.cabeca = instrucao->proximo;

    instrucao->proximo = NULL;

    // Verifica se a lista de leitura está vazia
    if (lista_leitura.cabeca == NULL) {
      // Se estiver vazia, a instrução se torna a cabeça e o fim da lista de
      // leitura
      lista_leitura.cabeca = instrucao;
      lista_leitura.fim    = instrucao;
    } else {
      // Se não estiver vazia, adiciona a instrução no final da lista de
      // leitura
      lista_leitura.fim->proximo = instrucao;
      lista_leitura.fim          = instrucao;
    }
  } else {
    // Caso a instrução não seja a primeira na lista de emissão
    while (emitida && emitida->proximo != instrucao) {
      emitida = emitida->proximo;
    }

    // Verifica se a instrução foi encontrada na lista de emissão
    if (!emitida) {
      return;
    }

    // Remove a instrução da lista de emissão, pois está pronta para leitura
    emitida->proximo = instrucao->proximo;

    instrucao->proximo = NULL;

    // Verifica se a lista de leitura está vazia
    if (lista_leitura.cabeca == NULL) {
      // Se estiver vazia, a instrução se torna a cabeça e o fim da lista de
      // leitura
      lista_leitura.cabeca = instrucao;
      lista_leitura.fim    = instrucao;
    } else {
      // Se não estiver vazia, adiciona a instrução no final da lista de
      // leitura
      lista_leitura.fim->proximo = instrucao;
      lista_leitura.fim          = instrucao;
    }
  }
}

// A função é responsável por mover uma instrução da lista de leitura para a
// lista de execução, indicando que a instrução está pronta para ser executada.
internal void
mandar_executar(Instrucao_No *instrucao) {
  Instrucao_No *leitura = lista_leitura.cabeca;

  Status_Instrucoes *status = status_instrucoes + instrucao->index;
  // status->emissao           = '-';
  status->leitura = clock;

  // Verifica se a instrução a ser executada é a primeira na lista de leitura
  if (leitura == instrucao) {
    // Remove a instrução da lista de leitura, pois está pronta para execução
    lista_leitura.cabeca = instrucao->proximo;

    instrucao->proximo = NULL;

    // Verifica se a lista de execução está vazia
    if (lista_executando.cabeca == NULL) {
      // Se estiver vazia, a instrução se torna a cabeça e o fim da lista de
      // execução
      lista_executando.cabeca = instrucao;
      lista_executando.fim    = instrucao;
    } else {
      // Se não estiver vazia, adiciona a instrução no final da lista de
      // execução
      lista_executando.fim->proximo = instrucao;
      lista_executando.fim          = instrucao;
    }
  } else {
    // Caso a instrução não seja a primeira na lista de leitura
    while (leitura && leitura->proximo != instrucao) {
      leitura = leitura->proximo;
    }

    // Verifica se a instrução foi encontrada na lista de leitura
    if (!leitura) {
      return;
    }

    // Remove a instrução da lista de leitura, pois está pronta para
    // execução
    leitura->proximo = instrucao->proximo;

    instrucao->proximo = NULL;

    // Verifica se a lista de execução está vazia
    if (lista_executando.cabeca == NULL) {
      // Se estiver vazia, a instrução se torna a cabeça e o fim da lista de
      // execução
      lista_executando.cabeca = instrucao;
      lista_executando.fim    = instrucao;
    } else {
      // Se não estiver vazia, adiciona a instrução no final da lista de
      // execução
      lista_executando.fim->proximo = instrucao;
      lista_executando.fim          = instrucao;
    }
  }
}

// A função é responsável por mover uma instrução da lista de execução para a
// lista de escrita, indicando que a instrução está pronta para escrever seu
// resultado.
internal void
mandar_escrever(Instrucao_No *instrucao) {
  Instrucao_No *executando = lista_executando.cabeca;

  Status_Instrucoes *status = status_instrucoes + instrucao->index;
  // status->execucao          = '-';
  status->escrita = clock;

  // Verifica se a instrução a ser escrita é a primeira na lista de execução
  if (executando == instrucao) {
    // Remove a instrução da lista de execução, pois está pronta para escrita
    lista_executando.cabeca = instrucao->proximo;

    instrucao->proximo = NULL;

    // Verifica se a lista de escrita está vazia
    if (lista_escrita.cabeca == NULL) {
      // Se estiver vazia, a instrução se torna a cabeça e o fim da lista de
      // escrita
      lista_escrita.cabeca = instrucao;
      lista_escrita.fim    = instrucao;
    } else {
      // Se não estiver vazia, adiciona a instrução no final da lista de
      // escrita
      lista_escrita.fim->proximo = instrucao;
      lista_escrita.fim          = instrucao;
    }
  } else {
    // Caso a instrução não seja a primeira na lista de execução
    while (executando && executando->proximo != instrucao) {
      executando = executando->proximo;
    }

    // Verifica se a instrução foi encontrada na lista de execução
    if (!executando) {
      return;
    }

    // Remove a instrução da lista de execução, pois está pronta para
    // escrita
    executando->proximo = instrucao->proximo;

    instrucao->proximo = NULL;

    // Verifica se a lista de escrita está vazia
    if (lista_escrita.cabeca == NULL) {
      // Se estiver vazia, a instrução se torna a cabeça e o fim da lista de
      // escrita
      lista_escrita.cabeca = instrucao;
      lista_escrita.fim    = instrucao;
    } else {
      // Se não estiver vazia, adiciona a instrução no final da lista de
      // escrita
      lista_escrita.fim->proximo = instrucao;
      lista_escrita.fim          = instrucao;
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
    printf("| %32s | %d       | %d      | %d     | %d       | %d      |\n",
           instrucao->instrucao, instrucao->busca, instrucao->emissao,
           instrucao->leitura, instrucao->execucao, instrucao->escrita);
    instrucao++;
  }
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
