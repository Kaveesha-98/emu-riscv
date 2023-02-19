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

    // Program exits if the file pointer returns NULL
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

  // this is assuming program header table is immediately after elf_header
  struct program_header program_header_table[header.phentsize]; // phentsize contains the number of entries in program header

  fread(&program_header_table, sizeof(struct program_header), header.phnum, fptr);

  printf("PROGRAM HEADER TABLE\n");
  for (int i = 0; i < header.phnum; i++){
    printf("\
header entry %d \
Type: 0x%x  \
Offset: 0x%lx \
VirtAddr: 0x%lx \
PhysAddr: 0x%lx \
Filesiz: 0x%lx  \
MemSiz: 0x%lx \
Flags: 0x%x \
Align: 0x%lx  \n", i, 
    program_header_table[i].seg_type, program_header_table[i].off, program_header_table[i].vaddr, program_header_table[i].paddr,
    program_header_table[i].filesz, program_header_table[i].memsz, program_header_table[i].flags, program_header_table[i].align);

    // executable should not require MMU
    if (program_header_table[i].paddr != program_header_table[i].vaddr) {
      printf("Emulator does not support MMU\n");
      fclose(fptr);
      exit(1);
    }
    
  }
  
  fclose(fptr); 

  return 0;
}
  // please don't delete might be needed in future
    //Xil_Out32(SLCR_UNLOCK_ADDR, UNLOCK_KEY);

    //Xil_Out32(FPGA_RST_CTRL, PL_CLR_MASK);
    //Xil_Out32(FPGA_RST_CTRL, PL_RST_MASK);
    //Xil_Out32(FPGA_RST_CTRL, PL_CLR_MASK);

    //Xil_Out32(SLCR_LOCK_ADDR, LOCK_KEY);
