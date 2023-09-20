#pragma once

#include "cpu.h"

#include <stddef.h>
#include <stdint.h>

typedef struct _leitor Leitor;

struct _leitor {
  char *input;
  char ch;
  size_t pos;
  size_t prox_pos;
};

// clang-format off

__attribute__((nonnull))
void leitor_ler_arquivo(char *input,
                        uint32_t *memoria,
                        uint32_t *PC,
                        CPU_Specs *cpu_specs);
