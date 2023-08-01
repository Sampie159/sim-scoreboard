#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "memoria.h"
#include <stdint.h>

typedef struct {
  char *nome_programa;
  uint64_t tam_memoria;
  char *nome_saida;
} Programa;

static void ler_argumentos(Programa *programa, int argc, char *argv[]);
static void print_ajuda(void);

int main(int argc, char *argv[]) {
  for (int i = 0; i < argc; i++) {
    printf("%s\n", argv[i]);
  }

  return 0;
}

static void ler_argumentos(Programa *programa, int argc, char *argv[]) {
  int opt;
  while ((opt = getopt(argc, argv, "p:m:o:h")) != -1) {
    switch (opt) {
      case 'p':
        programa->nome_programa = optarg;
        break;
      case 'm':
        programa->tam_memoria = atoi(optarg);
        break;
      case 'o':
        programa->nome_saida = optarg;
        break;
      case 'h':
        print_ajuda();
        exit(0);
    }
  }
}

static void print_ajuda(void) {
  printf("AJUDA!\n");
}
