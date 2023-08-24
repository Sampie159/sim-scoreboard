#include "hashtables.h"
#include "memoria.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
  uint32_t tamanho_memoria;
  char *nome_programa;
  char *nome_saida;
} Programa;

typedef struct {
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
} ArqAsb;

static ins_t codificar(char *instrucao);
static uint8_t get_registrador(const char *registrador);
static void decodificar(ins_t instrucao);
static void definir_programa(Programa *programa, int argc, char *argv[]);
static void print_ajuda(void);

int main(int argc, char *argv[]) {
  Programa programa = {0};
  definir_programa(&programa, argc, argv);

  uint32_t memoria[programa.tamanho_memoria];

  FILE *arq = fopen(programa.nome_programa, "r");
  if (arq == NULL) {
    fprintf(stderr, "Erro ao abrir o arquivo.\n");
    exit(1);
  }

  fseek(arq, 0, SEEK_END);
  unsigned long tamanho = ftell(arq);
  fseek(arq, 0, SEEK_SET);

  char buffer[tamanho];
  fread(buffer, sizeof(char), tamanho, arq);
  buffer[tamanho] = '\0';
  printf("%s\n", buffer);

  char instrucao[128] = "addi r1, r2, 10";
  ins_t t = codificar(instrucao);
  printf("%08X\n", t.tipo);
  printf("%08X\n", t.valor);

  decodificar(t);

  fclose(arq);

  return 0;
}

static ins_t codificar(char *instrucao) {
  ins_t ins = {0};

  char *token = strtok(instrucao, " ");

  struct OpCodeMap *op = encontra_operacao(token, strlen(token));
  ins.valor |= op->opcode << 26;
  ins.tipo = op->t;

  const char delim[] = ", ";

  switch (ins.tipo) {
  case R:
    ins.valor |= get_registrador(strtok(NULL, delim)) << 11; // rd
    ins.valor |= get_registrador(strtok(NULL, delim)) << 21; // rs
    ins.valor |= get_registrador(strtok(NULL, delim)) << 16; // rt

    break;
  case I:
    switch (op->opcode) {
    case 0x1:
    case 0x3:
      ins.valor |= get_registrador(strtok(NULL, delim)) << 16; // rt
      ins.valor |= get_registrador(strtok(NULL, delim)) << 21; // rs
      ins.valor |= atoi(strtok(NULL, delim));                  // extra
      break;
    case 0x9:
    case 0xA:
    case 0xB:
    case 0xC:
      ins.valor |= get_registrador(strtok(NULL, delim)) << 21; // rs
      ins.valor |= get_registrador(strtok(NULL, delim)) << 16; // rt
      ins.valor |= atoi(strtok(NULL, delim));                  // imm
      break;
    }
    break;
  case J:
    ins.valor |= atoi(strtok(NULL, delim)); // endereço
    break;
  default:
    perror("Tipo de instrução não reconhecido");
    exit(EXIT_FAILURE);
  }

  return ins;
}

static uint8_t get_registrador(const char *registrador) {
  const struct RegHashMap *reg = encontra_reg(registrador, strlen(registrador));

  return reg->identificador;
}

static void decodificar(ins_t instrucao) {
  uint8_t opcode = instrucao.valor >> 26;
  uint8_t rd = 0, rs = 0, rt = 0;
  uint16_t imm = 0, extra = 0;
  uint32_t end = 0;

  switch (OpCodeTipo[opcode]) {
  case R:
    rd = (instrucao.valor >> 11) & 0x1F;
    rs = (instrucao.valor >> 21) & 0x1F;
    rt = (instrucao.valor >> 16) & 0x1F;
    extra = instrucao.valor & 0x7FF;
    break;
  case I:
    rt = (instrucao.valor >> 16) & 0x1F;
    rs = (instrucao.valor >> 21) & 0x1F;
    imm = instrucao.valor & 0xFFFF;
    break;
  case J:
    end = instrucao.valor & 0x3FFFFFF;
    break;
  default:
    perror("Tipo de instrução não reconhecido");
    exit(EXIT_FAILURE);
  }

  printf("Opcode: %u\nRD: %u\nRS: %u\nRT: %u\nIMM: %u\nEXTRA: %u\n"
         "Endereço: %u\n",
         opcode, rd, rs, rt, imm, extra, end);
}

static void definir_programa(Programa *programa, int argc, char *argv[]) {
  int opt;
  while ((opt = getopt(argc, argv, "p:m:o:h")) != -1) {
    switch (opt) {
    case 'p':
      if (strncmp(optarg + strlen(optarg) - 4, ".asb", 4) == 0) {
        programa->nome_programa = optarg;
      } else {
        fprintf(stderr,
                "Extensão inválida, por favor use um arquivo \".asb\".\n");
        exit(1);
      }
      break;

    case 'm':
      programa->tamanho_memoria = atoi(optarg);
      if (programa->tamanho_memoria == 0) {
        fprintf(stderr, "Valor inválido para o tamanho da memória.\n");
        exit(1);
      }
      break;

    case 'o':
      programa->nome_saida = optarg;
      break;

    case 'h':
      print_ajuda();
      exit(0);
    }
  }

  if (programa->nome_saida == NULL) {
    programa->nome_saida = "scoreboarding";
  }
}

static void print_ajuda(void) { printf("Ajuda\n"); }
