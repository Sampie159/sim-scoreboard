#include "memoria.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define QTD_OPCODES 17
#define QTD_REGS 32

typedef struct {
  char nome[5];
  uint8_t opcode;
} CodeMap;

typedef struct {
  char nome[5];
  tipo t;
} CodeTipo;

// Constantes de Instrução
const CodeMap OpCodeMap[QTD_OPCODES] = {
    {"ADD", 0x0},   {"ADDI", 0x1},  {"SUB", 0x2}, {"SUBI", 0x3}, {"MUL", 0x4},
    {"DIV", 0x5},   {"AND", 0x6},   {"OR", 0x7},  {"NOT", 0x8},  {"BLT", 0x9},
    {"BGT", 0xA},   {"BEQ", 0xB},   {"BNE", 0xC}, {"JUMP", 0xD}, {"LOAD", 0xE},
    {"STORE", 0xF}, {"EXIT", 0x10},
};

const CodeTipo OpCodeTipo[QTD_OPCODES] = {
    {"ADD", R}, {"ADDI", I}, {"SUB", R},  {"SUBI", I},  {"MUL", R},  {"DIV", R},
    {"AND", R}, {"OR", R},   {"NOT", R},  {"BLT", I},   {"BGT", I},  {"BEQ", I},
    {"BNE", I}, {"JUMP", J}, {"LOAD", I}, {"STORE", I}, {"EXIT", J},
};

const CodeMap RegCodeMap[QTD_REGS] = {
    {"R0", 0x0},   {"R1", 0x1},   {"R2", 0x2},   {"R3", 0x3},   {"R4", 0x4},
    {"R5", 0x5},   {"R6", 0x6},   {"R7", 0x7},   {"R8", 0x8},   {"R9", 0x9},
    {"R10", 0xA},  {"R11", 0xB},  {"R12", 0xC},  {"R13", 0xD},  {"R14", 0xE},
    {"R15", 0xF},  {"R16", 0x10}, {"R17", 0x11}, {"R18", 0x12}, {"R19", 0x13},
    {"R20", 0x14}, {"R21", 0x15}, {"R22", 0x16}, {"R23", 0x17}, {"R24", 0x18},
    {"R25", 0x19}, {"R26", 0x1A}, {"R27", 0x1B}, {"R28", 0x1C}, {"R29", 0x1D},
    {"R30", 0x1E}, {"R31", 0x1F},
};

static ins_t codificar(char *instrucao);
static uint8_t get_registrador(const char *registrador);
static void decodificar(ins_t instrucao);

int main(void) {
  char instrucao[128] = "ADD R1, R2, R3";
  ins_t t = codificar(instrucao);
  printf("%08X\n", t.tipo);
  printf("%08X\n", t.valor);

  decodificar(t);

  return 0;
}

static ins_t codificar(char *instrucao) {
  ins_t ins = {0};

  uint8_t opcode = 0;
  char *token = strtok(instrucao, " ");
  for (int i = 0; i < QTD_OPCODES; i++) {
    if (strncmp(OpCodeMap[i].nome, token, 5) == 0) {
      opcode = OpCodeMap[i].opcode;
      ins.valor |= OpCodeMap[i].opcode << 26;
      ins.tipo = OpCodeTipo[i].t;
      break;
    }
  }

  const char delim[] = ", ";

  switch (ins.tipo) {
  case R:
    ins.valor |= get_registrador(strtok(NULL, delim)) << 11; // rd
    ins.valor |= get_registrador(strtok(NULL, delim)) << 21; // rs
    ins.valor |= get_registrador(strtok(NULL, delim)) << 16; // rt

    break;
  case I:
    switch (opcode) {
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
  for (int i = 0; i < QTD_REGS; i++) {
    if (strncmp(RegCodeMap[i].nome, registrador, 4) == 0) {
      return RegCodeMap[i].opcode;
    }
  }
  return 0xFF; // Registrador não encontrado
}

static void decodificar(ins_t instrucao) {
  uint8_t opcode = instrucao.valor >> 26;
  uint8_t rd = 0, rs = 0, rt = 0;
  uint16_t imm = 0, extra = 0;
  uint32_t end = 0;

  switch (instrucao.tipo) {
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
