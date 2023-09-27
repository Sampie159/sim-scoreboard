#include "cpu.h"
#include "hashtables.h"
#include "leitor.h"
#include "memoria.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
  uint32_t tamanho_memoria;
  char    *nome_programa;
  char    *nome_saida;
} Programa;

static void decodificar(uint32_t instrucao);
static void definir_programa(Programa *programa, int argc, char *argv[]);
static void print_ajuda(void);

int
main(int argc, char *argv[]) {
  Programa programa = { 0 };
  definir_programa(&programa, argc, argv);

  char     *memoria   = (char *) malloc(programa.tamanho_memoria);
  uint32_t  PC        = 100; // 100 primeiros bytes da memória reservados
  CPU_Specs cpu_specs = { 0 };

  FILE *arq = fopen(programa.nome_programa, "r");
  if (arq == NULL) {
    fprintf(stderr, "Erro ao abrir o arquivo.\n");
    exit(1);
  }

  fseek(arq, 0, SEEK_END);
  unsigned long tamanho = ftell(arq);
  rewind(arq);

  char buffer[tamanho];
  fread(buffer, sizeof(char), tamanho, arq);
  buffer[tamanho] = '\0';
  fclose(arq);

  printf("%s\n", buffer);

  leitor_ler_arquivo(buffer, memoria, &cpu_specs);
  printf("%d\n", *((int *) &memoria[00]));

  for (size_t i = 100; i < programa.tamanho_memoria; i += 4) {
    uint32_t instrucao = *((uint32_t *) &memoria[i]);
    decodificar(instrucao);
  }

  return 0;
}

static void
decodificar(uint32_t instrucao) {
  uint8_t  opcode = instrucao >> 26;
  uint8_t  rd = 0, rs = 0, rt = 0;
  uint16_t imm = 0, extra = 0;
  uint32_t end = 0;

  switch (OpCodeTipo[opcode]) {
  case R:
    rd    = (instrucao >> 11) & 0x1F;
    rs    = (instrucao >> 21) & 0x1F;
    rt    = (instrucao >> 16) & 0x1F;
    extra = instrucao & 0x7FF;
    break;
  case I:
    rt  = (instrucao >> 16) & 0x1F;
    rs  = (instrucao >> 21) & 0x1F;
    imm = instrucao & 0xFFFF;
    break;
  case J: end = instrucao & 0x3FFFFFF; break;
  default: perror("Tipo de instrução não reconhecido"); exit(EXIT_FAILURE);
  }

  printf(
      "Opcode: %u\nRD: %u\nRS: %u\nRT: %u\nIMM: %u\nEXTRA: %u\n"
      "Endereço: %u\n",
      opcode, rd, rs, rt, imm, extra, end);
}

static void
definir_programa(Programa *programa, int argc, char *argv[]) {
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
      if (programa->tamanho_memoria < 256) {
        fprintf(stderr, "Tamanho de memória muito pequeno. Minímo 256.\n");
        exit(1);
      }
      break;

    case 'o': programa->nome_saida = optarg; break;

    case 'h': print_ajuda(); exit(0);
    }
  }

  if (programa->nome_saida == NULL) {
    programa->nome_saida = "scoreboarding";
  }
}

static void
print_ajuda(void) {
  printf(
      "Uso: ./scoreboarding -p <arquivo> -m <tamanho> [-o <nome>]\n"
      "  -p <arquivo>   Arquivo com o código em assembly.\n"
      "  -m <tamanho>   Tamanho da memória em bytes.\n"
      "  -o <nome>      Nome do arquivo de saída.\n");
}
