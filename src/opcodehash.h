/* ANSI-C code produced by gperf version 3.1 */
/* Command-line: gperf opcodehash  */
/* Computed positions: -k'1-2' */

#if !((' ' == 32) && ('!' == 33) && ('"' == 34) && ('#' == 35) \
      && ('%' == 37) && ('&' == 38) && ('\'' == 39) && ('(' == 40) \
      && (')' == 41) && ('*' == 42) && ('+' == 43) && (',' == 44) \
      && ('-' == 45) && ('.' == 46) && ('/' == 47) && ('0' == 48) \
      && ('1' == 49) && ('2' == 50) && ('3' == 51) && ('4' == 52) \
      && ('5' == 53) && ('6' == 54) && ('7' == 55) && ('8' == 56) \
      && ('9' == 57) && (':' == 58) && (';' == 59) && ('<' == 60) \
      && ('=' == 61) && ('>' == 62) && ('?' == 63) && ('A' == 65) \
      && ('B' == 66) && ('C' == 67) && ('D' == 68) && ('E' == 69) \
      && ('F' == 70) && ('G' == 71) && ('H' == 72) && ('I' == 73) \
      && ('J' == 74) && ('K' == 75) && ('L' == 76) && ('M' == 77) \
      && ('N' == 78) && ('O' == 79) && ('P' == 80) && ('Q' == 81) \
      && ('R' == 82) && ('S' == 83) && ('T' == 84) && ('U' == 85) \
      && ('V' == 86) && ('W' == 87) && ('X' == 88) && ('Y' == 89) \
      && ('Z' == 90) && ('[' == 91) && ('\\' == 92) && (']' == 93) \
      && ('^' == 94) && ('_' == 95) && ('a' == 97) && ('b' == 98) \
      && ('c' == 99) && ('d' == 100) && ('e' == 101) && ('f' == 102) \
      && ('g' == 103) && ('h' == 104) && ('i' == 105) && ('j' == 106) \
      && ('k' == 107) && ('l' == 108) && ('m' == 109) && ('n' == 110) \
      && ('o' == 111) && ('p' == 112) && ('q' == 113) && ('r' == 114) \
      && ('s' == 115) && ('t' == 116) && ('u' == 117) && ('v' == 118) \
      && ('w' == 119) && ('x' == 120) && ('y' == 121) && ('z' == 122) \
      && ('{' == 123) && ('|' == 124) && ('}' == 125) && ('~' == 126))
/* The character set is not based on ISO-646.  */
#error "gperf generated tables don't work with this execution character set. Please report a bug to <bug-gperf@gnu.org>."
#endif

#line 1 "opcodehash"

#include "memoria.h"
#line 11 "opcodehash"
struct OpCodeMap { char *operacao; uint8_t opcode; tipo t; };
#include <string.h>

#define TOTAL_KEYWORDS 17
#define MIN_WORD_LENGTH 1
#define MAX_WORD_LENGTH 4
#define MIN_HASH_VALUE 1
#define MAX_HASH_VALUE 38
/* maximum key range = 38, duplicates = 0 */

#ifdef __GNUC__
__inline
#else
#ifdef __cplusplus
inline
#endif
#endif
static unsigned int
hash_operacao (register const char *str, register size_t len)
{
  static unsigned char asso_values[] =
    {
      39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
      39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
      39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
      39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
      39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
      39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
      39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
      39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
      39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
      39, 39, 39, 39, 39, 39, 39,  5, 10, 39,
       0, 15, 39, 25, 39,  8,  0, 39, 10,  3,
       5, 25, 39, 39,  0,  0, 39,  0, 39,  0,
       0, 39, 39, 39, 39, 39, 39, 39, 39, 39,
      39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
      39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
      39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
      39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
      39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
      39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
      39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
      39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
      39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
      39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
      39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
      39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
      39, 39, 39, 39, 39, 39
    };
  register unsigned int hval = len;

  switch (hval)
    {
      default:
        hval += asso_values[(unsigned char)str[1]];
      /*FALLTHROUGH*/
      case 1:
        hval += asso_values[(unsigned char)str[0]];
        break;
    }
  return hval;
}

struct OpCodeMap *
encontra_operacao (register const char *str, register size_t len)
{
  static struct OpCodeMap wordlist[] =
    {
#line 26 "opcodehash"
      {"j", 0x0D, J},
#line 28 "opcodehash"
      {"sw", 0x0F, I},
#line 15 "opcodehash"
      {"sub", 0x02, R},
#line 16 "opcodehash"
      {"subi", 0x03, I},
#line 17 "opcodehash"
      {"mul", 0x04, R},
#line 13 "opcodehash"
      {"add", 0x00, R},
#line 14 "opcodehash"
      {"addi", 0x01, I},
#line 18 "opcodehash"
      {"div", 0x05, R},
#line 27 "opcodehash"
      {"lw", 0x0E, I},
#line 19 "opcodehash"
      {"and", 0x06, R},
#line 25 "opcodehash"
      {"bne", 0x0C, I},
#line 29 "opcodehash"
      {"exit", 0x10, J},
#line 22 "opcodehash"
      {"blt", 0x09, I},
#line 20 "opcodehash"
      {"or", 0x07, R},
#line 24 "opcodehash"
      {"beq", 0x0B, I},
#line 21 "opcodehash"
      {"not", 0x08, R},
#line 23 "opcodehash"
      {"bgt", 0x0A, I}
    };

  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      register unsigned int key = hash_operacao (str, len);

      if (key <= MAX_HASH_VALUE && key >= MIN_HASH_VALUE)
        {
          register struct OpCodeMap *resword;

          switch (key - 1)
            {
              case 0:
                resword = &wordlist[0];
                goto compare;
              case 1:
                resword = &wordlist[1];
                goto compare;
              case 2:
                resword = &wordlist[2];
                goto compare;
              case 3:
                resword = &wordlist[3];
                goto compare;
              case 5:
                resword = &wordlist[4];
                goto compare;
              case 7:
                resword = &wordlist[5];
                goto compare;
              case 8:
                resword = &wordlist[6];
                goto compare;
              case 10:
                resword = &wordlist[7];
                goto compare;
              case 11:
                resword = &wordlist[8];
                goto compare;
              case 12:
                resword = &wordlist[9];
                goto compare;
              case 17:
                resword = &wordlist[10];
                goto compare;
              case 18:
                resword = &wordlist[11];
                goto compare;
              case 22:
                resword = &wordlist[12];
                goto compare;
              case 26:
                resword = &wordlist[13];
                goto compare;
              case 27:
                resword = &wordlist[14];
                goto compare;
              case 32:
                resword = &wordlist[15];
                goto compare;
              case 37:
                resword = &wordlist[16];
                goto compare;
            }
          return 0;
        compare:
          {
            register const char *s = resword->operacao;

            if (*str == *s && !strncmp (str + 1, s + 1, len - 1) && s[len] == '\0')
              return resword;
          }
        }
    }
  return 0;
}
