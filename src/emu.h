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

  if (address == 0xFFFFFFFFFFFFFFFF) {
    printf("program returned: %lx\n", data);
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

unsigned long sign_extend(unsigned long data, unsigned char size) {
  // 2*size bytes load
  switch (size) {
  case 0:
    return (0 - (data & (1<<7))) | (data & ((1<<8) - 1));

  case 1:
    return (0 - (data & (1<<15))) | (data & ((1<<16) - 1));
    
  case 2:
    //printf(" %lx ", (0 - (data & (1<<31))) | (data & ((1UL<<32) - 1)));
    return (0UL - (data & (1UL<<31))) | (data & ((1UL<<32) - 1));
  
  default:
    printf("Unexpected sign extend size.\n");
    return 0;
  }
}

int emulate(unsigned char mem[], unsigned long entry_point) {

  // initializing general purpose registers
  unsigned long gprs[32];
  for (int i = 0; i < 32; i++) {
    gprs[i] = 0;
  }  

  unsigned long pc = entry_point;
  unsigned int instruction;
  unsigned long immediate = 0;
  unsigned long op[8], opimm[8], op32[8], opimm32[8], wb_res[8], branch_pcs[4]; 
  unsigned long rs1, rs2, link_address, mem_access_address;
  unsigned long imm_u, imm_j, imm_i, imm_b, imm_s, load_data;
  bool reg_compare[8];
  enum gpr r1, r2, r3;

  while (1) {
    // get instruction
    if (!mem_read(pc, 0b010, &load_data)) {
      printf("Error loading pc %lx", pc);
      break;;
    };
    instruction = (unsigned int) load_data;

    // immediate generation
    imm_u = (0UL-(instruction & 0x80000000)) | (instruction & (0xFFFFF000));
    imm_j = (0UL-((instruction >> 11) & 0x00100000)) | ((instruction >> 11) & 0x00100000) | ((instruction >> 20) & 0x000007FE) | ((instruction >> 9) & 0x00000800) | (instruction & 0x000FF000);
    imm_i = (0UL-((instruction >> 20) & 0x00000800)) | ((instruction >> 20) & 0x00000FFF);
    imm_b = (0UL-((instruction >> 19) & 0x00001000)) | ((instruction >> 19) & 0x00001000) | ((instruction >> 20) & 0x000007E0) | ((instruction << 4) & 0x00000800) | ((instruction >> 7) & 0x0000001E);
    imm_s = (0UL-((instruction >> 20) & 0x00000800)) | ((instruction >> 20) & 0x00000FE0) | ((instruction >> 7) & 0x0000001F);

    //execution
    rs1 = gprs[(instruction >> 15) & 0x1F];
    rs2 = gprs[(instruction >> 20) & 0x1F];
    link_address = pc + 4;

    // 64 bit arithmetic operations between registers
    op[0] = (instruction & 0x40000000) ? (rs1 - rs2) : (rs1 + rs2);
    op[1] = rs1 << (rs2 & 63);
    op[2] = ((signed) rs1) < ((signed) rs2);
    op[3] = rs1 < rs2;
    op[4] = rs1 ^ rs2;
    op[5] = (instruction & 0x40000000) ? (((signed) rs1) >> (rs2 & 63)) : (rs1 >> (rs2 & 63));
    op[6] = rs1 | rs2;
    op[7] = rs1 & rs2;

    // 64 bit operations between registers and immediate
    opimm[0] = rs1 + imm_i;
    opimm[1] = rs1 << (imm_i & 63);
    opimm[2] = ((signed) rs1) < ((signed) imm_i);
    opimm[3] = rs1 < imm_i;
    opimm[4] = rs1 ^ imm_i;
    opimm[5] = (instruction & 0x40000000) ? (((signed long) rs1) >> (imm_i & 63)) : (rs1 >> (imm_i & 63));
    opimm[6] = rs1 | imm_i;
    opimm[7] = rs1 & imm_i;

    // 32 bit arithmetic operations between registers
    op32[0] = sign_extend((instruction & 0x40000000) ? (rs1 - rs2) : (rs1 + rs2), 0b10);
    op32[1] = sign_extend(rs1 << (rs2 & 31), 0b010);
    op32[2] = 0;
    op32[3] = 0;
    op32[4] = 0;
    op32[5] = sign_extend((instruction & 0x40000000) ? (((signed int) rs1) >> (rs2 & 31)) : ((rs1 & 0xFFFFFFFF) >> (rs2 & 31)), 0b010);
    op32[6] = 0;
    op32[7] = 0;

    // 32 bit arithmetic operations between registers
    opimm32[0] = sign_extend(rs1 + imm_i, 0b10);
    opimm32[1] = sign_extend(rs1 << (imm_i & 31), 0b010);
    opimm32[2] = 0;
    opimm32[3] = 0;
    opimm32[4] = 0;
    opimm32[5] = sign_extend((instruction & 0x40000000) ? (((signed int) rs1) >> (imm_i & 31)) : ((rs1 & 0xFFFFFFFF) >> (imm_i & 31)), 0b010);
    opimm32[6] = 0;
    opimm32[7] = 0;

    if(((instruction & 127) == 0b0000011) && !mem_read(rs1 + imm_i, (instruction >> 12) & 7, &load_data)){
      printf("Error loading address %lx at pc: %lx\n", rs1 + imm_i, pc);
      break;
    };

    if(((instruction & 127) == 0b0100011) && !mem_write(rs1 + imm_s, (instruction >> 12) & 7, rs2)) {
      // printf("Error storing address %lx at pc: %lx\n", rs1 + imm_s, pc);
      break;
    };

    // write back of result
    wb_res[0] = load_data;
    wb_res[1] = pc + 4;
    wb_res[2] = 0;
    wb_res[3] = pc + 4;
    wb_res[4] = (instruction & 32) ? op[(instruction >> 12) & 7] : opimm[(instruction >> 12) & 7];
    wb_res[5] = (instruction & 32) ? (imm_u) : (pc + imm_u);
    wb_res[6] = (instruction & 32) ? op32[(instruction >> 12) & 7] : opimm32[(instruction >> 12) & 7];
    wb_res[7] = 0;

    if(
      ((instruction & 127) == 0b0110111) || 
      ((instruction & 127) == 0b0010111) || 
      ((instruction & 127) == 0b1101111) || 
      ((instruction & 127) == 0b1100111) || 
      ((instruction & 127) == 0b0000011) || 
      ((instruction & 127) == 0b0010011) || 
      ((instruction & 127) == 0b0110011) || 
      ((instruction & 127) == 0b0011011) || 
      ((instruction & 127) == 0b0111011) 
    ) {
      gprs[(instruction >> 7) & 31] = wb_res[(instruction >> 2) & 7];
    };
    gprs[0] = 0;

    // next pc calc
    reg_compare[0] = (rs1 == rs2);
    reg_compare[1] = (rs1 != rs2);
    reg_compare[2] = 0;
    reg_compare[3] = 0;
    reg_compare[4] = ((signed) rs1) < ((signed) rs2);
    reg_compare[5] = ((signed) rs1) >= ((signed) rs2);
    reg_compare[6] = rs1 < rs2;
    reg_compare[7] = rs1 >= rs2;

    branch_pcs[0] = reg_compare[(instruction >> 12) & 7] ? pc + imm_b : pc + 4;
    branch_pcs[1] = rs1 + imm_i;
    branch_pcs[2] = 0;
    branch_pcs[3] = pc + imm_j;

    pc = (((instruction >> 4) & 7) == 0b110 )? branch_pcs[(instruction >> 2) & 3] : pc + 4;
    r1 = a4;
    r2 = t2;
    r3 = gp;
    // printf("%lx %lx %lx %lx\n", pc, gprs[ra], imm_u, gprs[t2]);
    //x = getchar();
  }
  
}