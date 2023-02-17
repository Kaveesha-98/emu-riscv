#include <stdio.h>
#include <stdlib.h>
#include "elf.h"

/**
 * This is a riscv emulator(Incomplete)
 * Currently support for S priv is not planned
 * Address space of RAM 0x8000_0000 - a000_0000 (512MB)
 */

int main()
{
  int n;
  FILE *fptr;

  if ((fptr = fopen("../../riscv-tests/isa/rv64ui-p-add","rb")) == NULL){
    printf("Error! opening file");

    // Program exits if the file pointer returns NULL.
    exit(1);
  }

  struct elf_header header;

  // read header of elf file
  fread(&header, sizeof(struct elf_header), 1, fptr);
  
  // check the header for correct fields
  if (
    header.magic != 0x464c457f && // need to check if the elf file has the proper header
    header.bitsize != 2 && // emulator only supports 64-bit elf files
    header.endian != 1 && // emulator expects little endian elf files
    header.obj_type != 2 && // should be executable type
    header.machine != 0xf3 && // expects a RISCV machine code
    header.version != 0x02 && // should be original elf file(no idea what this means)
    (header.entry_addr < 0x80000000 || header.entry_addr > 0xa0000000) && // program needs to be in RAM address space
    header.phoff != 0x40 // this value is expected for program header of 64-bit
  ) {
    printf("Some header fields did not match what was expected\n");
    fclose(fptr);
    exit(1);
  }
  fclose(fptr); 

  return 0;
}

