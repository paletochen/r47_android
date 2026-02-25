/* go to the src/c47 directory
 * then run
 *    gcc reservedRegisterLookupGenerator.c
 *    ./a.out > reservedRegisterLookupGenerator
 *    rm a.out
 *    gperf -C -G --null-strings -m 1000 -E -n -L ANSI-C -N lookupReservedVariableName -t < reservedRegisterLookupGenerator > reservedRegisterLookup.h
 *    rm reservedRegisterLookupGenerator
 *
 * The TO_QSPI prefix is needed for asso_values and wordlist in the generated reservedRegisterLookup.h file
 */
#include <stdio.h>

struct {
  unsigned char name[7];
  const char *reg;
} names[] = {
#if 0
  {1, 'X',  0,   0,   0,   0,   0},
  {1, 'Y',  0,   0,   0,   0,   0},
  {1, 'Z',  0,   0,   0,   0,   0},
  {1, 'T',  0,   0,   0,   0,   0},
  {1, 'A',  0,   0,   0,   0,   0},
  {1, 'B',  0,   0,   0,   0,   0},
  {1, 'C',  0,   0,   0,   0,   0},
  {1, 'D',  0,   0,   0,   0,   0},
  {1, 'L',  0,   0,   0,   0,   0},
  {1, 'I',  0,   0,   0,   0,   0},
  {1, 'J',  0,   0,   0,   0,   0},
  {1, 'K',  0,   0,   0,   0,   0},
  {1, 'M',  0,   0,   0,   0,   0},
  {1, 'N',  0,   0,   0,   0,   0},
  {1, 'P',  0,   0,   0,   0,   0},
  {1, 'Q',  0,   0,   0,   0,   0},
  {1, 'R',  0,   0,   0,   0,   0},
  {1, 'S',  0,   0,   0,   0,   0},
  {1, 'E',  0,   0,   0,   0,   0},
  {1, 'F',  0,   0,   0,   0,   0},
  {1, 'G',  0,   0,   0,   0,   0},
  {1, 'H',  0,   0,   0,   0,   0},
  {1, 'O',  0,   0,   0,   0,   0},
  {1, 'U',  0,   0,   0,   0,   0},
  {1, 'V',  0,   0,   0,   0,   0},
  {1, 'W',  0,   0,   0,   0,   0},
#endif
  {{3, 'A', 'D', 'M',  0,   0,   0 }, "RESERVED_VARIABLE_ADM" },
  {{5, 'D', '.', 'M', 'A', 'X',  0 }, "RESERVED_VARIABLE_DENMAX" },
  {{3, 'I', 'S', 'M',  0,   0,   0 }, "RESERVED_VARIABLE_ISM" },
  {{6, 'R', 'E', 'A', 'L', 'D', 'F'}, "RESERVED_VARIABLE_REALDF" },
  {{4, '#', 'D', 'E', 'C',  0,   0 }, "RESERVED_VARIABLE_NDEC" },
  {{3, 'A', 'C', 'C',  0,   0,   0 }, "RESERVED_VARIABLE_ACC" },
  {{5, 161, 145, 'L', 'i', 'm',  0 }, "RESERVED_VARIABLE_ULIM" },
  {{5, 161, 147, 'L', 'i', 'm',  0 }, "RESERVED_VARIABLE_LLIM" },
  {{2, 'F', 'V',  0,   0,   0,   0 }, "RESERVED_VARIABLE_FV" },
  {{4, 'I', '%', '/', 'a',  0,   0 }, "RESERVED_VARIABLE_IPONA" },
  {{5, 'N', 'P', 'P', 'E', 'R',  0 }, "RESERVED_VARIABLE_NPPER" },
  {{6, 'P', 'P', 'E', 'R', '/', 'a'}, "RESERVED_VARIABLE_PPERONA" },
  {{3, 'P', 'M', 'T',  0,   0,   0 }, "RESERVED_VARIABLE_PMT" },
  {{2, 'P', 'V',  0,   0,   0,   0 }, "RESERVED_VARIABLE_PV" },
  {{6, 'G', 'R', 'A', 'M', 'O', 'D'}, "RESERVED_VARIABLE_GRAMOD" },
  {{3, 161, 145, 'X',  0,   0,   0 }, "RESERVED_VARIABLE_UX" },
  {{3, 161, 147, 'X',  0,   0,   0 }, "RESERVED_VARIABLE_LX" },
  {{6, 'C', 'P', 'E', 'R', '/', 'a'}, "RESERVED_VARIABLE_CPERONA" },
  {{5, 161, 145, 'E', 'S', 'T',  0 }, "RESERVED_VARIABLE_UEST" },
  {{5, 161, 147, 'E', 'S', 'T',  0 }, "RESERVED_VARIABLE_LEST" },
  {{3, 161, 145, 'Y',  0,   0,   0 }, "RESERVED_VARIABLE_UY" },
  {{3, 161, 147, 'Y',  0,   0,   0 }, "RESERVED_VARIABLE_LY" },
};

int main() {
  int i, j;

  puts("struct reservedRegister {");
  puts("  char name[7];");
  puts("  calcRegister_t reg;");
  puts("};\n");
  puts("%%");
  for(i=0; i < sizeof(names)/sizeof(*names); i++) {
    int n = names[i].name[0];
    putchar('"');
    for(j=1; j<= n; j++) {
      if(names[i].name[j] == '\\' || names[i].name[j] == '"') { // Escape \ and " with \.
        putchar('\\');
      }
      putchar(names[i].name[j]);
    }
    putchar('"');
    printf(",%s\n", names[i].reg);
  }
  return 0;
}
