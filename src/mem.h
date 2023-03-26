#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define RAM_BASE_ADDRESS 0x80000000
#define RAM_HIGH_ADDRESS 0xa0000000

unsigned char byte_size_lookup[] = {1, 2, 4, 8}; 

enum gpr{zero, ra, sp, gp, tp, t0, t1, t2, s0, s1, a0, a1, a2, a3, a4, a5, a6, a7, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, t3, t4, t5, t6};

static unsigned char ram[536870912];

bool mem_read(unsigned long address, unsigned char funct3, unsigned long* load_data) {
  *load_data = 0;
  
  for (int i = 0; i < byte_size_lookup[funct3 & 3]; i++) {
    *load_data = *load_data >> 8;
    if ((address + i) < RAM_BASE_ADDRESS || (address + i) > RAM_HIGH_ADDRESS) {
      printf("load address %lx out of range of RAM.\n", (address + i));
      return 0;
    }
    *load_data = *load_data | (((unsigned long)ram[address + i - RAM_BASE_ADDRESS]) << 56);
    // printf("%lx\n", *load_data);  
    //data = data | ram[address + i];
  }
  if (funct3 & 4) {
    *load_data = ((*load_data) >> 8*(8 - byte_size_lookup[funct3 & 3])) & ((1UL<<(8*byte_size_lookup[funct3 & 3])) - 1);
  } else {
    *load_data = ((signed long) *load_data) >> 8*(8 - byte_size_lookup[funct3 & 3]);
  }
  return 1;
}

bool mem_write(unsigned long address, unsigned char funct3, unsigned long data) {

  if (address == 0x80001000) {
    printf("program returned: %lx\n", data);
    return 0;
  }
  

  for (int i = 0; i < byte_size_lookup[funct3 & 3]; i++) {
    if ((address + i) < RAM_BASE_ADDRESS || (address + i) > RAM_HIGH_ADDRESS) {
      // printf("store address %lx out of range of RAM.\n", (address + i));
      return 0;
    }
  }

  for (int i = 0; i < byte_size_lookup[funct3 & 3]; i++) {
    ram[address + i - RAM_BASE_ADDRESS] = (data & 255);
    data = data >> 8;
  }
  return 1;
};
