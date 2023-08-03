/* ANSI-C code produced by gperf version 3.1 */
/* Command-line: gperf reghash  */
/* Computed positions: -k'2-3' */

#if !(                                                                         \
    (' ' == 32) && ('!' == 33) && ('"' == 34) && ('#' == 35) && ('%' == 37) && \
    ('&' == 38) && ('\'' == 39) && ('(' == 40) && (')' == 41) &&               \
    ('*' == 42) && ('+' == 43) && (',' == 44) && ('-' == 45) && ('.' == 46) && \
    ('/' == 47) && ('0' == 48) && ('1' == 49) && ('2' == 50) && ('3' == 51) && \
    ('4' == 52) && ('5' == 53) && ('6' == 54) && ('7' == 55) && ('8' == 56) && \
    ('9' == 57) && (':' == 58) && (';' == 59) && ('<' == 60) && ('=' == 61) && \
    ('>' == 62) && ('?' == 63) && ('A' == 65) && ('B' == 66) && ('C' == 67) && \
    ('D' == 68) && ('E' == 69) && ('F' == 70) && ('G' == 71) && ('H' == 72) && \
    ('I' == 73) && ('J' == 74) && ('K' == 75) && ('L' == 76) && ('M' == 77) && \
    ('N' == 78) && ('O' == 79) && ('P' == 80) && ('Q' == 81) && ('R' == 82) && \
    ('S' == 83) && ('T' == 84) && ('U' == 85) && ('V' == 86) && ('W' == 87) && \
    ('X' == 88) && ('Y' == 89) && ('Z' == 90) && ('[' == 91) &&                \
    ('\\' == 92) && (']' == 93) && ('^' == 94) && ('_' == 95) &&               \
    ('a' == 97) && ('b' == 98) && ('c' == 99) && ('d' == 100) &&               \
    ('e' == 101) && ('f' == 102) && ('g' == 103) && ('h' == 104) &&            \
    ('i' == 105) && ('j' == 106) && ('k' == 107) && ('l' == 108) &&            \
    ('m' == 109) && ('n' == 110) && ('o' == 111) && ('p' == 112) &&            \
    ('q' == 113) && ('r' == 114) && ('s' == 115) && ('t' == 116) &&            \
    ('u' == 117) && ('v' == 118) && ('w' == 119) && ('x' == 120) &&            \
    ('y' == 121) && ('z' == 122) && ('{' == 123) && ('|' == 124) &&            \
    ('}' == 125) && ('~' == 126))
/* The character set is not based on ISO-646.  */
#error                                                                         \
    "gperf generated tables don't work with this execution character set. Please report a bug to <bug-gperf@gnu.org>."
#endif

#line 1 "reghash"

#include <stdint.h>
#line 11 "reghash"
struct RegHashMap {
  char *reg;
  uint8_t identificador;
};
#include <string.h>

#define TOTAL_KEYWORDS 32
#define MIN_WORD_LENGTH 2
#define MAX_WORD_LENGTH 3
#define MIN_HASH_VALUE 2
#define MAX_HASH_VALUE 68
/* maximum key range = 67, duplicates = 0 */

#ifdef __GNUC__
__inline
#else
#ifdef __cplusplus
inline
#endif
#endif
    static unsigned int
    hash_reg(register const char *str, register size_t len) {
  static unsigned char asso_values[] = {
      69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69,
      69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69,
      69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 15, 5,  0,  10, 25,
      12, 1,  60, 50, 40, 36, 26, 16, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69,
      69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69,
      69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69,
      69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69,
      69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69,
      69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69,
      69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69,
      69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69,
      69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69,
      69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69,
      69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69,
      69, 69, 69, 69, 69, 69, 69, 69};
  register unsigned int hval = len;

  switch (hval) {
  default:
    hval += asso_values[(unsigned char)str[2] + 4];
  /*FALLTHROUGH*/
  case 2:
    hval += asso_values[(unsigned char)str[1] + 1];
    break;
  }
  return hval;
}

struct RegHashMap *encontra_reg(register const char *str, register size_t len) {
  static struct RegHashMap wordlist[] = {
#line 15 "reghash"
      {"r2", 0x02},
#line 19 "reghash"
      {"r6", 0x06},
#line 36 "reghash"
      {"r23", 0x17},
#line 14 "reghash"
      {"r1", 0x01},
#line 26 "reghash"
      {"r13", 0x0D},
#line 16 "reghash"
      {"r3", 0x03},
#line 33 "reghash"
      {"r20", 0x14},
#line 18 "reghash"
      {"r5", 0x05},
#line 35 "reghash"
      {"r22", 0x16},
#line 13 "reghash"
      {"r0", 0x00},
#line 23 "reghash"
      {"r10", 0x0A},
#line 42 "reghash"
      {"r29", 0x1D},
#line 25 "reghash"
      {"r12", 0x0C},
#line 43 "reghash"
      {"r30", 0x1E},
#line 32 "reghash"
      {"r19", 0x13},
#line 17 "reghash"
      {"r4", 0x04},
#line 34 "reghash"
      {"r21", 0x15},
#line 41 "reghash"
      {"r28", 0x1C},
#line 24 "reghash"
      {"r11", 0x0B},
#line 31 "reghash"
      {"r18", 0x12},
#line 44 "reghash"
      {"r31", 0x1F},
#line 40 "reghash"
      {"r27", 0x1B},
#line 22 "reghash"
      {"r9", 0x09},
#line 39 "reghash"
      {"r26", 0x1A},
#line 30 "reghash"
      {"r17", 0x11},
#line 29 "reghash"
      {"r16", 0x10},
#line 21 "reghash"
      {"r8", 0x08},
#line 38 "reghash"
      {"r25", 0x19},
#line 28 "reghash"
      {"r15", 0x0F},
#line 20 "reghash"
      {"r7", 0x07},
#line 37 "reghash"
      {"r24", 0x18},
#line 27 "reghash"
      {"r14", 0x0E}};

  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH) {
    register unsigned int key = hash_reg(str, len);

    if (key <= MAX_HASH_VALUE && key >= MIN_HASH_VALUE) {
      register struct RegHashMap *resword;

      switch (key - 2) {
      case 0:
        resword = &wordlist[0];
        goto compare;
      case 1:
        resword = &wordlist[1];
        goto compare;
      case 2:
        resword = &wordlist[2];
        goto compare;
      case 5:
        resword = &wordlist[3];
        goto compare;
      case 7:
        resword = &wordlist[4];
        goto compare;
      case 10:
        resword = &wordlist[5];
        goto compare;
      case 11:
        resword = &wordlist[6];
        goto compare;
      case 12:
        resword = &wordlist[7];
        goto compare;
      case 13:
        resword = &wordlist[8];
        goto compare;
      case 15:
        resword = &wordlist[9];
        goto compare;
      case 16:
        resword = &wordlist[10];
        goto compare;
      case 17:
        resword = &wordlist[11];
        goto compare;
      case 18:
        resword = &wordlist[12];
        goto compare;
      case 21:
        resword = &wordlist[13];
        goto compare;
      case 22:
        resword = &wordlist[14];
        goto compare;
      case 25:
        resword = &wordlist[15];
        goto compare;
      case 26:
        resword = &wordlist[16];
        goto compare;
      case 27:
        resword = &wordlist[17];
        goto compare;
      case 31:
        resword = &wordlist[18];
        goto compare;
      case 32:
        resword = &wordlist[19];
        goto compare;
      case 36:
        resword = &wordlist[20];
        goto compare;
      case 37:
        resword = &wordlist[21];
        goto compare;
      case 40:
        resword = &wordlist[22];
        goto compare;
      case 41:
        resword = &wordlist[23];
        goto compare;
      case 42:
        resword = &wordlist[24];
        goto compare;
      case 46:
        resword = &wordlist[25];
        goto compare;
      case 50:
        resword = &wordlist[26];
        goto compare;
      case 51:
        resword = &wordlist[27];
        goto compare;
      case 56:
        resword = &wordlist[28];
        goto compare;
      case 60:
        resword = &wordlist[29];
        goto compare;
      case 61:
        resword = &wordlist[30];
        goto compare;
      case 66:
        resword = &wordlist[31];
        goto compare;
      }
      return 0;
    compare : {
      register const char *s = resword->reg;

      if (*str == *s && !strncmp(str + 1, s + 1, len - 1) && s[len] == '\0')
        return resword;
    }
    }
  }
  return 0;
}
