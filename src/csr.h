#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// machine information registers
// #define MVENDORID   0xF11;
// #define MARCHID     0xF12;
// #define MIMPID      0xF13;
#define MHARTID     0xF14;
//#define MCONFIGPTR  0xF15;

long unsigned mhartid;

// machine trap setup
#define MSTATUS     0x300
// #define MISA        0x301
#define MEDELEG     0x302
#define MIDELEG     0x303
#define MIE         0x304
#define MTVEC       0x305
// #define MCOUNTEREN  0x306

// machine trap handling
/* #define MSCRATCH  0x340 */
#define MEPC      0x341
#define MCAUSE    0x342
/* #define MTVAL     0x343
#define MIP       0x344
#define MTINST    0x34A
#define MTVAL2    0x34B */

// machine configuration
// #define MENVCFG 0x30A
// #define MSECCFG 0x747

// machine memory protection
#define PMPCFGBASE  0x3A0
#define PMPCFGHIGH  0x3AF
#define PMPADDRBASE 0x3B0
#define PMPADDRHIGH 0x3EF

// machine counter/timers
/* #define MCYCLE          0xB00
#define MINSTRET        0xB02
#define MHPMCOUNTERBASE 0xB03
#define MHPMCOUNTERHIGH 0xB1F */
// other machine counter/timers are for 32-bit machines

// machine counter setup
/* #define MCOUNTINHIBIT 0x320
#define MHPMEVENTBASE 0x323
#define MHPMEVENTHIGH 0x33F */

#define SATP  0x180
#define STVEC 0x105

bool isInstIllegal(unsigned int instruction) {
  unsigned char opcode = instruction & 127;
  unsigned char funct3 = (instruction >> 12) & 7;
  unsigned char funct7 = (instruction >> 25) & 127;
  unsigned char funct6 = (instruction >> 26) & 63;
  // only checks opcode
  return (
    (opcode != 0b0110111) & // lui
    (opcode != 0b0010111) & // auipc
    (opcode != 0b1101111) & // jal
    (opcode != 0b1100111) & // jalr
    (opcode != 0b1100011) & // conditional branches
    (opcode != 0b0000011) & // loads
    (opcode != 0b0100011) & // stores
    (opcode != 0b0010011) & // op-imm
    (opcode != 0b0110011) & // op
    (opcode != 0b0011011) & // op32-imm
    (opcode != 0b0111011) & // op32
    (opcode != 0b1110011) & // Zicsr
    (opcode != 0b0001111)
  );
};