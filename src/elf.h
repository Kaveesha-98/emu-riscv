#include <stdio.h>
#include <stdlib.h>
// source: https://osblog.stephenmarz.com/ch11.html 

struct elf_header {
  unsigned int magic;
  unsigned char bitsize;
  unsigned char endian;
  unsigned char ident_abi_version;
  unsigned char target_platform;
  unsigned char abi_version;
  unsigned char padding[7];
  unsigned short obj_type;
  unsigned short machine; // 0xf3 for RISC-V
  unsigned int version;
  unsigned long entry_addr;
  unsigned long phoff;
  unsigned long shoff;
  unsigned int flags;
  unsigned short ehsize;
  unsigned short phentsize;
  unsigned short phnum;
  unsigned short shentsize;
  unsigned short shnum;
  unsigned short shstrndx;
};

struct program_header {
  unsigned int seg_type;
  unsigned int flags;
  unsigned long off;
  unsigned long vaddr;
  unsigned long paddr;
  unsigned long filesz;
  unsigned long memsz;
  unsigned long align;
};

void sort_program_header_table(struct program_header program_header_table[], unsigned int length) {
  // uses insertion sort
  struct program_header temp;
  for (int i = 1; i < length; i++) {
    for (int j = i-1; j >= 0; j--) {
      if (program_header_table[j].off > program_header_table[j+1].off) {
        //printf("%d %d\n", i, j);
        temp = program_header_table[j];
        program_header_table[j] = program_header_table[j+1];
        program_header_table[j+1] = temp;
      }
    }
  }
  
};