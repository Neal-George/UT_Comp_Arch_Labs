/*
Name 1: George Neal
UTEID 1: gln276
*/

/***************************************************************/
/*                                                             */
/*   LC-3b Simulator                                           */
/*                                                             */
/*   EE 460N - Lab 5                                           */
/*   The University of Texas at Austin                         */
/*                                                             */
/***************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/***************************************************************/
/*                                                             */
/* Files:  ucode        Microprogram file                      */
/*         pagetable    page table in LC-3b machine language   */
/*         isaprogram   LC-3b machine language program file    */
/*                                                             */
/***************************************************************/

/***************************************************************/
/* These are the functions you'll have to write.               */
/***************************************************************/

void eval_micro_sequencer();
void cycle_memory();
void eval_bus_drivers();
void drive_bus();
void latch_datapath_values();

/***************************************************************/
/* A couple of useful definitions.                             */
/***************************************************************/
#define FALSE 0
#define TRUE  1

/***************************************************************/
/* Use this to avoid overflowing 16 bits on the bus.           */
/***************************************************************/
#define Low16bits(x) ((x) & 0xFFFF)

/***************************************************************/
/* Definition of the control store layout.                     */
/***************************************************************/
#define CONTROL_STORE_ROWS 64
#define INITIAL_STATE_NUMBER 18

/***************************************************************/
/* Definition of bit order in control store word.              */
/***************************************************************/
enum CS_BITS {
  IRD,
  COND1, COND0,
  J5, J4, J3, J2, J1, J0,
  LD_MAR,
  LD_MDR,
  LD_IR,
  LD_BEN,
  LD_REG,
  LD_CC,
  LD_PC,
  GATE_PC,
  GATE_MDR,
  GATE_ALU,
  GATE_MARMUX,
  GATE_SHF,
  PCMUX1, PCMUX0,
  DRMUX,
  SR1MUX,
  ADDR1MUX,
  ADDR2MUX1, ADDR2MUX0,
  MARMUX,
  ALUK1, ALUK0,
  MIO_EN,
  R_W,
  DATA_SIZE,
  LSHF1,
  /* MODIFY: you have to add all your new control signals */
  /* <Interrupt_Exception_Control_Bits>  */
  Gate_PSR,
  Gate_USP_RES,
  Gate_SSP_RES,
  Gate_SP,
  Gate_IE_V,
  LD_USP_RES,
  LD_SSP_RES,
  LD_PSR,
  LD_TEMP_V,
  SP_SEL,
  DI_SEL,
  VMUX1,
  VMUX0,
  TM_COMP_MUX,
  CLD_E1,
  CLD_E2,
  CLD_E3,
  CLD_MAR0,
  CLD_PSR,
  PSR_MODE,
  SP_SELD,
  GATE_DEC_PC,
  PSR_MDR_MODE,
  /* Virtual_Memory_Control_Bits */
  GATE_PTBR,
  GATE_VA_OFFSET,
  GATE_VA_IND_OFFSET_PN,
  GATE_PFN,
  LD_VA,
  SET_WRITE,
  CLR_WRITE,
  WRITE_ENABLE,
  SET_REFBIT_SEL,
  REF_BIT_ENABLE,
  WR_REF_MDR_SEL,
  TEMPJ_SEL2,
  TEMPJ_SEL1,
  TEMPJ_SEL0,
  LD_TEMPJ,
  VT_PSR,
  GOTO_TEMPJ,
  LD_WR,
  CONTROL_STORE_BITS
} CS_BITS;
/***************************************************************/
/* Functions to get at the control bits.                       */
/***************************************************************/
int GetIRD(int *x)           { return(x[IRD]); }
int GetCOND(int *x)          { return((x[COND1] << 1) + x[COND0]); }
int GetJ(int *x)             {
  return((x[J5] << 5) + (x[J4] << 4) +
    (x[J3] << 3) + (x[J2] << 2) +
    (x[J1] << 1) + x[J0]);
}
int GetLD_MAR(int *x)        { return(x[LD_MAR]); }
int GetLD_MDR(int *x)        { return(x[LD_MDR]); }
int GetLD_IR(int *x)         { return(x[LD_IR]); }
int GetLD_BEN(int *x)        { return(x[LD_BEN]); }
int GetLD_REG(int *x)        { return(x[LD_REG]); }
int GetLD_CC(int *x)         { return(x[LD_CC]); }
int GetLD_PC(int *x)         { return(x[LD_PC]); }
int GetGATE_PC(int *x)       { return(x[GATE_PC]); }
int GetGATE_MDR(int *x)      { return(x[GATE_MDR]); }
int GetGATE_ALU(int *x)      { return(x[GATE_ALU]); }
int GetGATE_MARMUX(int *x)   { return(x[GATE_MARMUX]); }
int GetGATE_SHF(int *x)      { return(x[GATE_SHF]); }
int GetPCMUX(int *x)         { return((x[PCMUX1] << 1) + x[PCMUX0]); }
int GetDRMUX(int *x)         { return(x[DRMUX]); }
int GetSR1MUX(int *x)        { return(x[SR1MUX]); }
int GetADDR1MUX(int *x)      { return(x[ADDR1MUX]); }
int GetADDR2MUX(int *x)      { return((x[ADDR2MUX1] << 1) + x[ADDR2MUX0]); }
int GetMARMUX(int *x)        { return(x[MARMUX]); }
int GetALUK(int *x)          { return((x[ALUK1] << 1) + x[ALUK0]); }
int GetMIO_EN(int *x)        { return(x[MIO_EN]); }
int GetR_W(int *x)           { return(x[R_W]); }
int GetDATA_SIZE(int *x)     { return(x[DATA_SIZE]); }
int GetLSHF1(int *x)         { return(x[LSHF1]); }
/* Interrupt Exception Control Bit Getters */
int GetGate_PSR(int* x)      { return (x[Gate_PSR]); }
int GetGate_USP_RES(int* x)  { return (x[Gate_USP_RES]); }
int GetGate_SSP_RES(int* x)  { return (x[Gate_SSP_RES]); }
int GetGate_SP(int* x)       { return (x[Gate_SP]); }
int GetGate_IE_V(int* x)     { return (x[Gate_IE_V]); }
int GetLD_USP_RES(int* x)    { return (x[LD_USP_RES]); }
int GetLD_SSP_RES(int* x)    { return (x[LD_SSP_RES]); }
int GetLD_PSR(int* x)        { return (x[LD_PSR]); }
int GetLD_TEMP_V(int* x)     { return (x[LD_TEMP_V]); }
int GetSP_SEL(int* x)        { return (x[SP_SEL]); }
int GetDI_SEL(int* x)        { return (x[DI_SEL]); }
int GetVMUX(int* x)          { return ((x[VMUX1] << 1) + x[VMUX0]); }
int GetTM_COMP_MUX(int* x)   { return (x[TM_COMP_MUX]); }
int GETCLD_E1(int* x)        { return (x[CLD_E1]); }
int GETCLD_E2(int* x)        { return (x[CLD_E2]); }
int GETCLD_E3(int* x)        { return (x[CLD_E3]); }
int GETCLD_MAR0(int* x)      { return (x[CLD_MAR0]); }
int GETCLD_PSR(int* x)       { return (x[CLD_PSR]); }
int GETPSR_MODE(int* x)      { return (x[PSR_MODE]); }
int GETSP_SELD(int* x)       { return (x[SP_SELD]); }
int GETGATE_DEC_PC(int* x)   { return (x[GATE_DEC_PC]); }
int GETPSR_MDR_MODE(int* x)  { return (x[PSR_MDR_MODE]); }
/* Virtual Memory Control Bit Getters */
int GetGATE_PTBR(int* x)             { return x[GATE_PTBR]; }
int GetGATE_VA_OFFSET(int* x)        { return x[GATE_VA_OFFSET]; }
int GetGATE_VA_IND_OFFSET_PN(int* x) { return x[GATE_VA_IND_OFFSET_PN]; }
int GetGATE_PFN(int* x)              { return x[GATE_PFN]; }
int GetLD_VA(int* x)                 { return x[LD_VA]; }
int GetSET_WRITE(int* x)             { return x[SET_WRITE]; }
int GetCLR_WRITE(int* x)             { return x[CLR_WRITE]; }
int GetWRITE_ENABLE(int* x)          { return x[WRITE_ENABLE]; }
int GetSET_REFBIT_SEL(int* x)        { return x[SET_REFBIT_SEL]; }
int GetREF_BIT_ENABLE(int* x)        { return x[REF_BIT_ENABLE]; }
int GetWR_REF_MDR_SEL(int* x)        { return x[WR_REF_MDR_SEL]; }
int GetTEMPJ_SEL(int* x)             {
  return ((x[TEMPJ_SEL2] << 2) +
    (x[TEMPJ_SEL1] << 1) +
    (x[TEMPJ_SEL0]));
}
int GetLD_TEMPJ(int* x)              { return x[LD_TEMPJ]; }
int GetVT_PSR(int* x)                { return x[VT_PSR]; }
int GetGOTO_TEMPJ(int* x)            { return x[GOTO_TEMPJ]; }
int GetLD_WR(int* x)                 { return x[LD_WR]; }
/* MODIFY: you can add more Get functions for your new control signals */

/***************************************************************/
/* The control store rom.                                      */
/***************************************************************/
int CONTROL_STORE[CONTROL_STORE_ROWS][CONTROL_STORE_BITS];

/***************************************************************/
/* Main memory.                                                */
/***************************************************************/
/* MEMORY[A][0] stores the least significant byte of word at word address A
MEMORY[A][1] stores the most significant byte of word at word address A
There are two write enable signals, one for each byte. WE0 is used for
the least significant byte of a word. WE1 is used for the most significant
byte of a word. */

#define WORDS_IN_MEM    0x2000 /* 32 frames */ 
#define MEM_CYCLES      5
int MEMORY[WORDS_IN_MEM][2];

/***************************************************************/

/***************************************************************/

/***************************************************************/
/* LC-3b State info.                                           */
/***************************************************************/
#define LC_3b_REGS 8

int RUN_BIT;  /* run bit */
int BUS;  /* value of the bus */

typedef struct System_Latches_Struct{

  int PC,   /* program counter */
    MDR,  /* memory data register */
    MAR,  /* memory address register */
    IR,   /* instruction register */
    N,    /* n condition bit */
    Z,    /* z condition bit */
    P,    /* p condition bit */
    BEN;        /* ben register */

  int READY;  /* ready bit */
  /* The ready bit is also latched as you dont want the memory system to assert it
  at a bad point in the cycle*/

  int REGS[LC_3b_REGS]; /* register file. */

  int MICROINSTRUCTION[CONTROL_STORE_BITS]; /* The microintruction */

  int STATE_NUMBER; /* Current State Number - Provided for debugging */

  /* For lab 4 */
  int INTV; /* Interrupt vector register */
  int EXCV; /* Exception vector register */
  int SSP; /* Initial value of system stack pointer */
  /* MODIFY: you should add here any other registers you need to implement interrupts and exceptions */
  int Temp_V;
  int USP_RES;
  int PSR;

  /* For lab 5 */
  int PTBR; /* This is initialized when we load the page table */
  int VA;   /* Temporary VA register */
  /* MODIFY: you should add here any other registers you need to implement virtual memory */
  int TempJ; 
  int Write; 

} System_Latches;

/* Data Structure for Latch */

System_Latches CURRENT_LATCHES, NEXT_LATCHES;

/* For lab 5 */
#define PAGE_NUM_BITS 9
#define PTE_PFN_MASK 0x3E00
#define PTE_VALID_MASK 0x0004
#define PAGE_OFFSET_MASK 0x1FF

/***************************************************************/
/* A cycle counter.                                            */
/***************************************************************/
int CYCLE_COUNT;

/***************************************************************/
/*                                                             */
/* Procedure : help                                            */
/*                                                             */
/* Purpose   : Print out a list of commands.                   */
/*                                                             */
/***************************************************************/
void help() {
  printf("----------------LC-3bSIM Help-------------------------\n");
  printf("go               -  run program to completion       \n");
  printf("run n            -  execute program for n cycles    \n");
  printf("mdump low high   -  dump memory from low to high    \n");
  printf("rdump            -  dump the register & bus values  \n");
  printf("?                -  display this help menu          \n");
  printf("quit             -  exit the program                \n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : cycle                                           */
/*                                                             */
/* Purpose   : Execute a cycle                                 */
/*                                                             */
/***************************************************************/
void cycle() {

  eval_micro_sequencer();
  cycle_memory();
  eval_bus_drivers();
  drive_bus();
  latch_datapath_values();

  CURRENT_LATCHES = NEXT_LATCHES;

  CYCLE_COUNT++;
}

/***************************************************************/
/*                                                             */
/* Procedure : run n                                           */
/*                                                             */
/* Purpose   : Simulate the LC-3b for n cycles.                 */
/*                                                             */
/***************************************************************/
void run(int num_cycles) {
  int i;

  if (RUN_BIT == FALSE) {
    printf("Can't simulate, Simulator is halted\n\n");
    return;
  }

  printf("Simulating for %d cycles...\n\n", num_cycles);
  for (i = 0; i < num_cycles; i++) {
    if (CURRENT_LATCHES.PC == 0x0000) {
      RUN_BIT = FALSE;
      printf("Simulator halted\n\n");
      break;
    }
    cycle();
  }
}

/***************************************************************/
/*                                                             */
/* Procedure : go                                              */
/*                                                             */
/* Purpose   : Simulate the LC-3b until HALTed.                 */
/*                                                             */
/***************************************************************/
void go() {
  if (RUN_BIT == FALSE) {
    printf("Can't simulate, Simulator is halted\n\n");
    return;
  }

  printf("Simulating...\n\n");
  while (CURRENT_LATCHES.PC != 0x0000)
    cycle();
  RUN_BIT = FALSE;
  printf("Simulator halted\n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : mdump                                           */
/*                                                             */
/* Purpose   : Dump a word-aligned region of memory to the     */
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void mdump(FILE * dumpsim_file, int start, int stop) {
  int address; /* this is a byte address */

  printf("\nMemory content [0x%0.4x..0x%0.4x] :\n", start, stop);
  printf("-------------------------------------\n");
  for (address = (start >> 1); address <= (stop >> 1); address++)
    printf("  0x%0.4x (%d) : 0x%0.2x%0.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
  printf("\n");

  /* dump the memory contents into the dumpsim file */
  fprintf(dumpsim_file, "\nMemory content [0x%0.4x..0x%0.4x] :\n", start, stop);
  fprintf(dumpsim_file, "-------------------------------------\n");
  for (address = (start >> 1); address <= (stop >> 1); address++)
    fprintf(dumpsim_file, " 0x%0.4x (%d) : 0x%0.2x%0.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
  fprintf(dumpsim_file, "\n");
  fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : rdump                                           */
/*                                                             */
/* Purpose   : Dump current register and bus values to the     */
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void rdump(FILE * dumpsim_file) {
  int k;

  printf("\nCurrent register/bus values :\n");
  printf("-------------------------------------\n");
  printf("Cycle Count  : %d\n", CYCLE_COUNT);
  printf("PC           : 0x%0.4x\n", CURRENT_LATCHES.PC);
  printf("IR           : 0x%0.4x\n", CURRENT_LATCHES.IR);
  printf("STATE_NUMBER : 0x%0.4x\n\n", CURRENT_LATCHES.STATE_NUMBER);
  printf("BUS          : 0x%0.4x\n", BUS);
  printf("MDR          : 0x%0.4x\n", CURRENT_LATCHES.MDR);
  printf("MAR          : 0x%0.4x\n", CURRENT_LATCHES.MAR);
  printf("CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
  printf("Registers:\n");
  for (k = 0; k < LC_3b_REGS; k++)
    printf("%d: 0x%0.4x\n", k, CURRENT_LATCHES.REGS[k]);
  printf("\n");

  /* dump the state information into the dumpsim file */
  fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
  fprintf(dumpsim_file, "-------------------------------------\n");
  fprintf(dumpsim_file, "Cycle Count  : %d\n", CYCLE_COUNT);
  fprintf(dumpsim_file, "PC           : 0x%0.4x\n", CURRENT_LATCHES.PC);
  fprintf(dumpsim_file, "IR           : 0x%0.4x\n", CURRENT_LATCHES.IR);
  fprintf(dumpsim_file, "STATE_NUMBER : 0x%0.4x\n\n", CURRENT_LATCHES.STATE_NUMBER);
  fprintf(dumpsim_file, "BUS          : 0x%0.4x\n", BUS);
  fprintf(dumpsim_file, "MDR          : 0x%0.4x\n", CURRENT_LATCHES.MDR);
  fprintf(dumpsim_file, "MAR          : 0x%0.4x\n", CURRENT_LATCHES.MAR);
  fprintf(dumpsim_file, "CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
  fprintf(dumpsim_file, "Registers:\n");
  for (k = 0; k < LC_3b_REGS; k++)
    fprintf(dumpsim_file, "%d: 0x%0.4x\n", k, CURRENT_LATCHES.REGS[k]);
  fprintf(dumpsim_file, "\n");
  fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : get_command                                     */
/*                                                             */
/* Purpose   : Read a command from standard input.             */
/*                                                             */
/***************************************************************/
void get_command(FILE * dumpsim_file) {
  char buffer[20];
  int start, stop, cycles;

  printf("LC-3b-SIM> ");

  scanf("%s", buffer);
  printf("\n");

  switch (buffer[0]) {
  case 'G':
  case 'g':
    go();
    break;

  case 'M':
  case 'm':
    scanf("%i %i", &start, &stop);
    mdump(dumpsim_file, start, stop);
    break;

  case '?':
    help();
    break;
  case 'Q':
  case 'q':
    printf("Bye.\n");
    exit(0);

  case 'R':
  case 'r':
    if (buffer[1] == 'd' || buffer[1] == 'D')
      rdump(dumpsim_file);
    else {
      scanf("%d", &cycles);
      run(cycles);
    }
    break;

  default:
    printf("Invalid Command\n");
    break;
  }
}

/***************************************************************/
/*                                                             */
/* Procedure : init_control_store                              */
/*                                                             */
/* Purpose   : Load microprogram into control store ROM        */
/*                                                             */
/***************************************************************/
void init_control_store(char *ucode_filename) {
  FILE *ucode;
  int i, j, index;
  char line[200];

  printf("Loading Control Store from file: %s\n", ucode_filename);

  /* Open the micro-code file. */
  if ((ucode = fopen(ucode_filename, "r")) == NULL) {
    printf("Error: Can't open micro-code file %s\n", ucode_filename);
    exit(-1);
  }

  /* Read a line for each row in the control store. */
  for (i = 0; i < CONTROL_STORE_ROWS; i++) {
    if (fscanf(ucode, "%[^\n]\n", line) == EOF) {
      printf("Error: Too few lines (%d) in micro-code file: %s\n",
        i, ucode_filename);
      exit(-1);
    }

    /* Put in bits one at a time. */
    index = 0;

    for (j = 0; j < CONTROL_STORE_BITS; j++) {
      /* Needs to find enough bits in line. */
      if (line[index] == '\0') {
        printf("Error: Too few control bits in micro-code file: %s\nLine: %d\n",
          ucode_filename, i);
        exit(-1);
      }
      if (line[index] != '0' && line[index] != '1') {
        printf("Error: Unknown value in micro-code file: %s\nLine: %d, Bit: %d\n",
          ucode_filename, i, j);
        exit(-1);
      }

      /* Set the bit in the Control Store. */
      CONTROL_STORE[i][j] = (line[index] == '0') ? 0 : 1;
      index++;
    }

    /* Warn about extra bits in line. */
    if (line[index] != '\0')
      printf("Warning: Extra bit(s) in control store file %s. Line: %d\n",
      ucode_filename, i);
  }
  printf("\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : init_memory                                     */
/*                                                             */
/* Purpose   : Zero out the memory array                       */
/*                                                             */
/***************************************************************/
void init_memory() {
  int i;

  for (i = 0; i < WORDS_IN_MEM; i++) {
    MEMORY[i][0] = 0;
    MEMORY[i][1] = 0;
  }
}

/**************************************************************/
/*                                                            */
/* Procedure : load_program                                   */
/*                                                            */
/* Purpose   : Load program and service routines into mem.    */
/*                                                            */
/**************************************************************/
void load_program(char *program_filename, int is_virtual_base) {
  FILE * prog;
  int ii, word, program_base, pte, virtual_pc;

  /* Open program file. */
  prog = fopen(program_filename, "r");
  if (prog == NULL) {
    printf("Error: Can't open program file %s\n", program_filename);
    exit(-1);
  }

  /* Read in the program. */
  if (fscanf(prog, "%x\n", &word) != EOF)
    program_base = word >> 1;
  else {
    printf("Error: Program file is empty\n");
    exit(-1);
  }

  if (is_virtual_base) {
    if (CURRENT_LATCHES.PTBR == 0) {
      printf("Error: Page table base not loaded %s\n", program_filename);
      exit(-1);
    }

    /* convert virtual_base to physical_base */
    virtual_pc = program_base << 1;
    pte = (MEMORY[(CURRENT_LATCHES.PTBR + (((program_base << 1) >> PAGE_NUM_BITS) << 1)) >> 1][1] << 8) |
      MEMORY[(CURRENT_LATCHES.PTBR + (((program_base << 1) >> PAGE_NUM_BITS) << 1)) >> 1][0];

    printf("virtual base of program: %04x\npte: %04x\n", program_base << 1, pte);
    if ((pte & PTE_VALID_MASK) == PTE_VALID_MASK) {
      program_base = (pte & PTE_PFN_MASK) | ((program_base << 1) & PAGE_OFFSET_MASK);
      printf("physical base of program: %x\n\n", program_base);
      program_base = program_base >> 1;
    }
    else {
      printf("attempting to load a program into an invalid (non-resident) page\n\n");
      exit(-1);
    }
  }
  else {
    /* is page table */
    CURRENT_LATCHES.PTBR = program_base << 1;
  }

  ii = 0;
  while (fscanf(prog, "%x\n", &word) != EOF) {
    /* Make sure it fits. */
    if (program_base + ii >= WORDS_IN_MEM) {
      printf("Error: Program file %s is too long to fit in memory. %x\n",
        program_filename, ii);
      exit(-1);
    }

    /* Write the word to memory array. */
    MEMORY[program_base + ii][0] = word & 0x00FF;
    MEMORY[program_base + ii][1] = (word >> 8) & 0x00FF;;
    ii++;
  }

  if (CURRENT_LATCHES.PC == 0 && is_virtual_base)
    CURRENT_LATCHES.PC = virtual_pc;

  printf("Read %d words from program into memory.\n\n", ii);
}

/***************************************************************/
/*                                                             */
/* Procedure : initialize                                      */
/*                                                             */
/* Purpose   : Load microprogram and machine language program  */
/*             and set up initial state of the machine         */
/*                                                             */
/***************************************************************/
void initialize(char *ucode_filename, char *pagetable_filename, char *program_filename, int num_prog_files) {
  int i;
  init_control_store(ucode_filename);

  init_memory();
  load_program(pagetable_filename, 0);
  for (i = 0; i < num_prog_files; i++) {
    load_program(program_filename, 1);
    while (*program_filename++ != '\0');
  }
  CURRENT_LATCHES.Z = 1;
  CURRENT_LATCHES.STATE_NUMBER = INITIAL_STATE_NUMBER;
  memcpy(CURRENT_LATCHES.MICROINSTRUCTION, CONTROL_STORE[INITIAL_STATE_NUMBER], sizeof(int)*CONTROL_STORE_BITS);
  CURRENT_LATCHES.SSP = 0x3000; /* Initial value of system stack pointer */


  /* MODIFY: you can add more initialization code HERE */
  CURRENT_LATCHES.PSR |= 0x8000; 

  NEXT_LATCHES = CURRENT_LATCHES;

  RUN_BIT = TRUE;
}

/***************************************************************/
/*                                                             */
/* Procedure : main                                            */
/*                                                             */
/***************************************************************/
int main(int argc, char *argv[]) {
  FILE * dumpsim_file;

  /* Error Checking */
  if (argc < 4) {
    printf("Error: usage: %s <micro_code_file> <page table file> <program_file_1> <program_file_2> ...\n",
      argv[0]);
    exit(1);
  }

  printf("LC-3b Simulator\n\n");

  initialize(argv[1], argv[2], argv[3], argc - 3);

  if ((dumpsim_file = fopen("dumpsim", "w")) == NULL) {
    printf("Error: Can't open dumpsim file\n");
    exit(-1);
  }

  while (1)
    get_command(dumpsim_file);

}

/***************************************************************/
/* Do not modify the above code, except for the places indicated
with a "MODIFY:" comment.
You are allowed to use the following global variables in your
code. These are defined above.

CONTROL_STORE
MEMORY
BUS

CURRENT_LATCHES
NEXT_LATCHES

You may define your own local/global variables and functions.
You may use the functions to get at the control bits defined
above.

Begin your code here                  */
/***************************************************************/


/* Macros */
#define DEBUG     0
#define DEBUG_INT 0
#define DEBUG_VM  1
#define DEBUG_TIM 1
#define PBit      8
#define VBit      4
#define MBit      2
#define RBit      1
#define notMBit   0xFFFD
#define notRBit   0xFFFE


/* Globals */
int MemToLatch;
int Gate_PC_Input;
int Gate_MDR_Input;
int Gate_ALU_Input;
int Gate_MARMUX_Input;
int Gate_SHF_Input;
/* Gates Added for Interrupts */
int Gate_PSR_Input;
int Gate_USP_RES_Input;
int GATE_SSP_RES_Input;
int GATE_SP_Input;
int GATE_IE_V_Input;
int Gate_DEC_PC_Input;
int TIE = 0;
/* Gates Added for Virtual Mem */
int GatePageOffset; 
int GateIndexedPageNo; 
int GatePTBR;
int GatePFN;
/* Registers with constant values */
const int SP_REGNUM = 0x0006;
const int INT_V_Time = 0x01;
const int PME_V = 0x04;
const int UAE_V = 0x03;
const int UOE_V = 0x05;
const int PFE_V = 0x02;
const int EO_PRIV_SPACE = 0x2FFF;
const int EO_TRAP_SPACE = 0x01FF;
const int Int_Cycle = 300;
/* for uSequencer logic */
const int E1Bits = 0x01;
const int E2Bits = 0x04;
const int E3Bits = 0x3D;
const int MAR0Bits = 0x2E;
const int PSR15Bits = 0x04;
const int PSR15VTBits = 0x01; 
/* TempJ bit values */
const int TJ2Bits = 0x1D; 
const int TJ3Bits = 0x18;
const int TJ1819Bits = 0x21; 
const int TJ55Bits = 0x17; 
const int TJ57Bits = 0x19; 


int AssmInstr = 0;

/**
* returns sign extended value of x-bit input to y-bits
* Assumes 1 indexed x and y
* (value, 5bit 2's comp value, 16bit 2's comp value)
*/
int SextXtoY(int value, int x, int y)
{
  int returnVal, sign, i;
  returnVal = 0;

  sign = ((value&(1 << (x - 1))) > 0);
  returnVal |= value;

  for (i = x; i < y; i += 1)
  {
    returnVal |= (sign << i);
  }
  if (DEBUG)
  {
    printf("SEXTXtoY(0x%x,%d,%d) == 0x%x\n",
      value, x, y, returnVal);
  }
  return returnVal & 0x0FFFF;
}

/**
* returns WE bits (two low bits) for memory
*/
int we_logic()
{
  int lRW, lDataSize, lBit0MAR, lRetWE;

  lRW = GetR_W(CURRENT_LATCHES.MICROINSTRUCTION);
  lDataSize = GetDATA_SIZE(CURRENT_LATCHES.MICROINSTRUCTION);
  lBit0MAR = (0x0001 & CURRENT_LATCHES.MAR);
  lRetWE = 0;

  if (lRW != 0)
  {
    /* check write mode */
    if (lDataSize != 0)
    { /* word write mode */
      lRetWE = 0x03;
    }
    else
    { /* byte write mode */
      if (lBit0MAR == 0)
      { /* little end: MDR[7:0] */
        lRetWE = 0x01;
      }
      else
      { /* big end: MDR[15:8] */
        lRetWE = 0x02;
      }
    }
  }
  if (DEBUG)
  {
    printf("WE_LOGIC_\n At state %d:\n _______________\n lDataSize is  x%x\n lRW is        x%x\n lBit0Mar is   x%x\n lRetWE is     x%x\n _______________\n",
      CURRENT_LATCHES.STATE_NUMBER,
      lDataSize,
      lRW,
      lBit0MAR,
      lRetWE);
  }
  return lRetWE;
}



/* TODO */
void updatePSR()
{
  int latchVal = 0;
  if (GetLD_PSR(CURRENT_LATCHES.MICROINSTRUCTION) > 0)
  {
    if (GETPSR_MODE(CURRENT_LATCHES.MICROINSTRUCTION) > 0)
    {
      NEXT_LATCHES.PSR |= (BUS & 0xFFFF);
      if (DEBUG_VM) { printf("Cycle: %d\nNext PSR <- 0x%x\n", CYCLE_COUNT, (NEXT_LATCHES.PSR)); }
    }
    else
    {
      /* want to load mode */
      if (GETPSR_MODE(CURRENT_LATCHES.MICROINSTRUCTION) > 0)
      {
        NEXT_LATCHES.PSR |= 0x8000;
        if (DEBUG_VM) { printf("Cycle: %d\nNext PSR <- 0x%x\n", CYCLE_COUNT, (NEXT_LATCHES.PSR)); }
      }
      else
      {
        NEXT_LATCHES.PSR &= 0x7FFF;
        if (DEBUG_VM) { printf("Cycle: %d\nNext PSR <- 0x%x\n", CYCLE_COUNT, (NEXT_LATCHES.PSR)); }
      }
    }
  }
}

/* included sp mux */
/**
* returns register number required for SR1
* outputs 3 bits
*/
int getSR1MuxOut()
{
  int returnVal = 0;
  if (GetSR1MUX(CURRENT_LATCHES.MICROINSTRUCTION) == 1)
  { /* return IR[8:6] */
    returnVal = ((CURRENT_LATCHES.IR & 0x01C0) >> 6);
  }
  else
  { /* return IR[11:9] */
    returnVal = ((CURRENT_LATCHES.IR & 0x0E00) >> 9);
  }
  if (DEBUG)
  {
    printf("SR1MUX with input 0x%x output 0x%x\n",
      GetSR1MUX(CURRENT_LATCHES.MICROINSTRUCTION), returnVal);
  }
  if (GetSP_SEL(CURRENT_LATCHES.MICROINSTRUCTION) > 0)
  {
    returnVal = 0x0006;
  }
  return returnVal & 0x0FFFF;
}

/**
* return register value for SR1 for current instr
*/
int getSR1Data()
{
  int returnVal;

  returnVal = CURRENT_LATCHES.REGS[getSR1MuxOut()];

  if (DEBUG)
  {
    printf("Data from SR1 is 0x%x\n", returnVal);
  }
  return returnVal & 0x0FFFF;
}

/**
* returns output of SR2 MUX
* either SR2 or immediate 5 (16bits)
*/
int getSR2MuxOut()
{
  int returnVal, reg;

  returnVal = reg = 0;

  /* check bit 5: imm5 or SR2 */
  if ((CURRENT_LATCHES.IR & 0x020) != 0)
  {
    /* return immediate 5 bits sign extended to 16 bits */
    if (DEBUG) { printf("IR[5] = %d, imm mode\n", ((CURRENT_LATCHES.IR & 0x020) > 0)); }
    returnVal = SextXtoY((CURRENT_LATCHES.IR & 0x01F), 5, 16);
  }
  else
  {
    /* otherwise use sr2 (bottom 3 bits) */
    if (DEBUG) { printf("IR[5] = %d, reg mode\n", ((CURRENT_LATCHES.IR & 0x020) > 0)); }
    reg = (CURRENT_LATCHES.IR & 0x07);
    returnVal = CURRENT_LATCHES.REGS[reg];
  }

  if (DEBUG)
  {
    printf("Data from SR2MUX is 0x%x\n", returnVal);
  }
  return returnVal & 0x0FFFF;
}

/* included sp mux */
/* outputs 3 bits */
int getDRMuxOut()
{
  int returnVal = 0;
  if (GetDRMUX(CURRENT_LATCHES.MICROINSTRUCTION) == 1)
  {
    returnVal = 0x07;
  }
  else
  {
    returnVal = ((CURRENT_LATCHES.IR & 0x0E00) >> 9);
  }
  if (GETSP_SELD(CURRENT_LATCHES.MICROINSTRUCTION) > 0)
  {
    returnVal = SP_REGNUM;
  }
  if (DEBUG)
  {
    printf("DRMUX with input 0x%x output 0x%x\n",
      GetDRMUX(CURRENT_LATCHES.MICROINSTRUCTION), returnVal);
  }
  return returnVal & 0x0FFFF;
}

/* TODO */
int getDISPout()
{
  int returnVal = 0;
  if (GetDI_SEL(CURRENT_LATCHES.MICROINSTRUCTION) > 0)
  {
    /* increment */
    returnVal = getSR1Data() + 2;
  }
  else
  {
    returnVal = getSR1Data() - 2;
  }
  return returnVal;
}


/**
* get output from Addr2Mux
*/
int getAddr2MuxOut()
{
  int lAddr2Mux = 0;
  switch (GetADDR2MUX(CURRENT_LATCHES.MICROINSTRUCTION))
  {
  case 0:
    lAddr2Mux = 0;
    break;
  case 1: /* SEXT(IR[5:0]) */
    lAddr2Mux = SextXtoY((CURRENT_LATCHES.IR & 0x03F), 6, 16);
    break;
  case 2: /* SEXT(IR[8:0]) */
    lAddr2Mux = SextXtoY((CURRENT_LATCHES.IR & 0x01FF), 9, 16);
    break;
  case 3:
    lAddr2Mux = SextXtoY((CURRENT_LATCHES.IR & 0x7FF), 11, 16);
    break;
  }
  if (DEBUG) { printf("getAddr2MuxOut output: x%x\nstate: %d", lAddr2Mux, CURRENT_LATCHES.STATE_NUMBER); }
  return lAddr2Mux;
}

/**
* return Addr1 Mux output
* either PC value, or SR1
*/
int getAddr1MuxOut()
{
  int lAddr1Mux = 0;
  if (GetADDR1MUX(CURRENT_LATCHES.MICROINSTRUCTION) == 1)
  {
    lAddr1Mux = CURRENT_LATCHES.REGS[getSR1MuxOut()];
  }
  else
  {
    lAddr1Mux = CURRENT_LATCHES.PC;
  }
  return lAddr1Mux; 
}

/**
* Returns output of MARMUX
* Uses:
*   getAddr2MuxOut
*     LSHFT (hardware serially after addr2mux)
*   getAddr1MuxOut
* Adds the output of the above 2 outputs
*/
int getMARMUXOut()
{
  int lAddr2Mux, lAddr1Mux, lAddrOutput;
  lAddr2Mux = lAddr1Mux = lAddrOutput = 0;

  /* Emulate logic for addr2mux */
  lAddr2Mux = getAddr2MuxOut();

  /* Emulate LSHFT after Addr2Mux */
  if (GetLSHF1(CURRENT_LATCHES.MICROINSTRUCTION) == 1)
  {
    lAddr2Mux *= 2;
  }
  lAddr2Mux &= 0x0FFFF;

  /* Emulate logic for Addr1MUX */
  lAddr1Mux = getAddr1MuxOut();

  /* Add both results and return */
  lAddrOutput = (lAddr2Mux + lAddr1Mux) & 0x0FFFF;

  if (DEBUG)
  {
    printf("_getMARMUXOut_\nAstate %d: \n____________________\nlAddr2Mux is   x%x\nlAddr1Mux is   x%x\nlAddrOutput is x%x\n____________________\n",
      CURRENT_LATCHES.STATE_NUMBER,
      lAddr2Mux,
      lAddr1Mux,
      lAddrOutput);
  }

  return lAddrOutput;
}

/**
* Returns ALUK output
* Uses:
*   SR1MuxOut
*   SR2MuxOut
* Determines how to operate on inputs based on sel lines (uCode)
*/
int getALUKOutput()
{
  int returnVal;

  returnVal = 0;

  switch (GetALUK(CURRENT_LATCHES.MICROINSTRUCTION))
  {
  case 0: /* ADD */
    returnVal = (getSR2MuxOut()) +
      (getSR1Data());
    if (DEBUG) {
      printf("_GETALUKOUTPUT_\nAdding 0x%x and 0x%x\nResult: 0x%x\n________________\n",
        getSR2MuxOut(), getSR1Data(), returnVal);
    }
    break;
  case 1: /* AND */
    returnVal = (getSR2MuxOut()) &
      (getSR1Data());
    if (DEBUG) {
      printf("_GETALUKOUTPUT_\nAnding 0x%x and 0x%x\nResult: 0x%x\n_________________\n",
        getSR2MuxOut(), getSR1Data(), returnVal);
    }
    break;
  case 2: /* XOR */
    returnVal = (getSR2MuxOut()) ^
      (getSR1Data());
    if (DEBUG) {
      printf("_GETALUKOUTPUT_\nXORing 0x%x and 0x%x\nResult: 0x%x\n_________________\n",
        getSR2MuxOut(), getSR1Data(), returnVal);
    }
    break;
  case 3: /* PASSA (SR1's data) */
    returnVal = getSR1Data();
    if (DEBUG) {
      printf("_GETALUKOUTPUT_\nPassing 0x%x through\n_________________\n",
        returnVal);
    }
    break;
  }
  returnVal &= 0x0FFFF;

  if (DEBUG)
  {
    printf("_getALUKOutput_\nAt state %d: \n____________________\nreturnVal is   x%x\n____________________\n",
      CURRENT_LATCHES.STATE_NUMBER,
      returnVal);
  }
  return returnVal;
}

/**
* Returns output of SHF unit
* Uses:
*   getSR1MuxOut
* Shifts according to IR[5:0]
*/
int getSHFOutput()
{
  int operand, returnVal;
  operand = CURRENT_LATCHES.IR & 0x03F;
  returnVal = CURRENT_LATCHES.REGS[getSR1MuxOut()];
  switch (operand & 0x30)
  {
  case 0x00: /* LSH */
    returnVal = returnVal << (operand & 0x0F);
    break;
  case 0x010: /* RSHFL */
    returnVal = returnVal >> (operand & 0x0F);
    break;
  case 0x030: /* RSHFA */
    returnVal = returnVal >> (operand & 0x0F);
    returnVal = SextXtoY(returnVal,
      (16 - (operand & 0x0F)),
      16);
    break;
  }
  returnVal &= 0x0FFFF;

  if (DEBUG)
  {
    printf("SHF Output: 0x%x\n", returnVal);
  }

  return returnVal;
}

/* interrupt updates occur in updateUSequencer() */
/* they require the bus is written to */
/**
* Emulates the micro sequencer
* Gets bits from IR and uCode
* Calculates next micro instruction
*/
void eval_micro_sequencer()
{

  /*
  * Evaluate the address of the next state according to the
  * micro sequencer logic. Latch the next microinstruction.
  */

  int lIR15to12, lIR11, lCurrentState, lCOND10, lBEN, lRBit, lIRD, lJBits;
  int x, nextState, lCOND1, lCOND0, testBit, lE1, lE2, lE3, lPSR15;

  /* acquire all necessary bits of from current microinstruction */
  x = CURRENT_LATCHES.IR;
  if (DEBUG) { printf("IR: 0x%x\n", x); }
  Low16bits(x);
  lIR15to12 = (x >> 12);
  lIR11 = (x & 0x0800) >> 11;
  lCurrentState = CURRENT_LATCHES.STATE_NUMBER;
  lCOND10 = GetCOND(CURRENT_LATCHES.MICROINSTRUCTION);
  lCOND1 = (lCOND10 & 0x2) >> 1;
  lCOND0 = lCOND10 & 0x1;
  lBEN = CURRENT_LATCHES.BEN;
  lRBit = (CURRENT_LATCHES.READY == (MEM_CYCLES - 1));
  lIRD = GetIRD(CURRENT_LATCHES.MICROINSTRUCTION);
  lJBits = GetJ(CURRENT_LATCHES.MICROINSTRUCTION);
  lPSR15 = ((CURRENT_LATCHES.PSR & 0x8000) >> 15);

  if (DEBUG_VM)
  {
    printf("Current Cycle: %d\nCurrent State: %d\n", CYCLE_COUNT, CURRENT_LATCHES.STATE_NUMBER);
    if (CURRENT_LATCHES.STATE_NUMBER == 26)
      printf("Beginning translation\n"); 
  }

  if ((DEBUG_TIM == 1) && 
      (CURRENT_LATCHES.STATE_NUMBER == 37))
      printf("Exception taken at cycle: %d\n", CYCLE_COUNT); 

  if ((DEBUG_TIM == 1) && 
      (CURRENT_LATCHES.STATE_NUMBER == 51))
      printf("RTI finished at cycle: %d\n", CYCLE_COUNT);

  if (DEBUG_VM)
    printf("Current PC: x%x\n", CURRENT_LATCHES.PC); 

  if (DEBUG_VM)
    printf("PSR: x%x\n", CURRENT_LATCHES.PSR); 

  /* for debugging */
  if (CURRENT_LATCHES.STATE_NUMBER == 32)
  {
    AssmInstr += 1;
  }
  if (DEBUG)
  {
    printf("Current Instruction Count: %d\n", AssmInstr);
  }

  /*  Calculate low 3 J bits for Branch, Mem Ready, and Addr Mode */
  lJBits |= (((lCOND1 != 0) && (lCOND0 == 0) && (lBEN != 0)) << 2);
  lJBits |= (((lCOND1 == 0) && (lCOND0 != 0) && (lRBit != 0)) << 1);
  lJBits |= (((lCOND1 != 0) && (lCOND0 != 0) && (lIR11 != 0)));



  /* Latch next state and microinstruction information */
  if (lIRD == 0)
  {
    nextState = lJBits;
  }
  else
  {
    nextState = lIR15to12;
  }

  NEXT_LATCHES.STATE_NUMBER = nextState;


  /* NEXT_LATCHES.MICROINSTRUCTION = CONTROL_STORE[nextState]; */
  if (DEBUG) { printf("Next Microinstruction: "); }
  for (x = 0; x < CONTROL_STORE_BITS; x += 1)
  {
    NEXT_LATCHES.MICROINSTRUCTION[x] = CONTROL_STORE[nextState][x];
    if (DEBUG) { printf("%d", (CONTROL_STORE[nextState][x])); }
  }
  if (DEBUG) { printf("\n"); }

  if (DEBUG)
  {
    printf("_MICROSEQUENCER_\nAt state %d:\n________________\nIR15to12 is  x%x\nIR11 is      x%x\nCOND 1 is    x%x\nCOND 0 is    x%x\nBEN is       x%x\nR is         x%x\nIR[11] is    x%x\nJ bits are   x%x\nnextState is x%x\n________________\n",
      lCurrentState,
      lIR15to12,
      lIR11,
      lCOND1,
      lCOND0,
      lBEN,
      lRBit,
      lIR11,
      lJBits,
      nextState);
  }
  if (DEBUG_INT) {
    printf("____________________\nCURRENT CYCLE: %d\nCURRENT STATE: %d\n____________________\n",
      CYCLE_COUNT,
      CURRENT_LATCHES.STATE_NUMBER);
  }
}

/**
* Emulate Memory access
* Uses:
*   we_logic
* Once accessed, begins incrementing a counter every cycle.
* Once 4th cycle is reached, latch memory to global bus
*   If write, get Write Enable bits
*/
void cycle_memory()
{
  int lWE, lMemToWrite, lMAR0Bit;
  lWE = lMemToWrite = lMAR0Bit = 0;

  /* If memory is being accessed this cycle, increment R_BitMemCtr */
  if (GetMIO_EN(CURRENT_LATCHES.MICROINSTRUCTION) > 0)
  {
    NEXT_LATCHES.READY = (CURRENT_LATCHES.READY + 1) % MEM_CYCLES;

    if (DEBUG) { printf("Attempting to access memory...\n"); }

    /* If it is the (MEM_CYCLES-1)th cycle, output mem */
    if (CURRENT_LATCHES.READY == (MEM_CYCLES - 1))
    {
      if (GetR_W(CURRENT_LATCHES.MICROINSTRUCTION) == 0)
      {

        /* LOAD */
        if (DEBUG) { printf("Loading...\n"); }

        /* TODO: rather than datasize, use mar[0]*/
        if (GetDATA_SIZE(CURRENT_LATCHES.MICROINSTRUCTION) == 0)
        { /* Byte read */
          /* even address: Little End of Word Address */
          lMAR0Bit = CURRENT_LATCHES.MAR & 0x0001;
          if (DEBUG)
          {
            printf("MAR 0 bit is %d\n", lMAR0Bit);
          }
          MemToLatch = MEMORY[CURRENT_LATCHES.MAR >> 1][lMAR0Bit];
          MemToLatch = SextXtoY(MemToLatch, 8, 16);
        }
        else
        {
          /* Word Read */
          MemToLatch = 0;
          /* Little End appears first in memory */
          MemToLatch |= 0x0FF & (MEMORY[CURRENT_LATCHES.MAR >> 1][0]);
          MemToLatch |= ((0x0FF & (MEMORY[CURRENT_LATCHES.MAR >> 1][1])) << 8);
        }
        MemToLatch &= 0x0FFFF;
        if (DEBUG)
        {
          printf("MemToLatch is driving 0x%x\n", MemToLatch);
        }
      }

      /* STORE */
      else
      {
        /* mdr has been formatted for byte/word by logic block from bus */
        lMemToWrite = CURRENT_LATCHES.MDR & 0x0FFFF;

        /* Write to appropriate location(s) */
        lWE = we_logic();
        if (DEBUG) { printf("WE is %d\n", lWE); }
        if (lWE & 0x01)
        {
          MEMORY[CURRENT_LATCHES.MAR >> 1][0] = (lMemToWrite & 0x00FF);
          if (DEBUG)
          {
            printf("Memory[%x][0] <= 0x%x\n", (CURRENT_LATCHES.MAR >> 1), (lMemToWrite & 0x00FF));
          }
        }
        if (lWE & 0x02)
        {
          MEMORY[CURRENT_LATCHES.MAR >> 1][1] = ((lMemToWrite & 0x0FF00) >> 8);
          if (DEBUG)
          {
            printf("Memory[%x][1] <= 0x%x\n", (CURRENT_LATCHES.MAR >> 1), ((lMemToWrite & 0x0FF00) >> 8));
          }
        }
        MemToLatch = 0;
      }
      if (DEBUG_VM)
        printf("Memory to Latch = %x\n", MemToLatch); 
    }
  }
  if (DEBUG)
  {
    printf("_CYCLE_MEMORY_\nAt state %d: \n_____________________\nMemToLatch is  x%x\nlMemToWrite is x%x\nlWE is         x%x\nR(next) bit is x%x\n_____________________\n",
      CURRENT_LATCHES.STATE_NUMBER,
      MemToLatch,
      lMemToWrite,
      lWE,
      NEXT_LATCHES.READY);
  }
}

/**
* Emulate all logic that drive input to tristate buffers to BUS
*   As such, info is saved in global variables
* Uses:
*   getALUKOutput
*   getMARMUXOut
*   getSHFOutput
*/
void eval_bus_drivers()
{
  /* Evaluate input for GatePC */
  Gate_PC_Input = CURRENT_LATCHES.PC;

  /* Evaluate input for GateMDR */
  if (GetDATA_SIZE(CURRENT_LATCHES.MICROINSTRUCTION) == 0)
  { /* byte case */
    Gate_MDR_Input = SextXtoY((CURRENT_LATCHES.MDR & 0x0FF), 8, 16);
  }
  else
  { /* word case */
    Gate_MDR_Input = CURRENT_LATCHES.MDR;
  }

  /* Evaluate input for GateALU */
  Gate_ALU_Input = getALUKOutput();

  /* Evaluate input for GateMARMUX */
  if (GetMARMUX(CURRENT_LATCHES.MICROINSTRUCTION) == 0)
  { /* ZEXT and LSHFT(1) IR[7:0] */
    Gate_MARMUX_Input = ((CURRENT_LATCHES.IR & 0x00FF) << 1);
  }
  else
  {
    Gate_MARMUX_Input = getMARMUXOut();
  }

  /* Evaluate input for GateSHF */
  Gate_SHF_Input = getSHFOutput();


  /* Gate PSR tristate input */
  Gate_PSR_Input = 0;
  Gate_PSR_Input |= (CURRENT_LATCHES.PSR & 0x8000);
  Gate_PSR_Input |= ((CURRENT_LATCHES.N << 2) +
    (CURRENT_LATCHES.Z << 1) +
    (CURRENT_LATCHES.P));

  /* gate USP_RES tristate input */
  Gate_USP_RES_Input = CURRENT_LATCHES.USP_RES;

  /* gate SSP tristate input */
  GATE_SSP_RES_Input = CURRENT_LATCHES.SSP;

  /* gate SP tristate input */
  GATE_SP_Input = getDISPout();

  /* gate IE_V Input */
  GATE_IE_V_Input = (0x200 + (CURRENT_LATCHES.Temp_V << 1));

  /* gate DEC_PC Input */
  Gate_DEC_PC_Input = CURRENT_LATCHES.PC - 2;

  /* TODO */
  /* gate page offset (low 9 bits of VA) */
  GatePageOffset = CURRENT_LATCHES.VA & 0x1FF; 

  /* gate indexed page number from VA (high 7 bits * 2) */
  GateIndexedPageNo = (CURRENT_LATCHES.VA & 0xFE00) >> 8; 

  /* gate PTBR (High 8 bits) */
  GatePTBR = CURRENT_LATCHES.PTBR & 0xFF00; 
  
  /* gate PFN (PTE[13:9]) */
  GatePFN = CURRENT_LATCHES.MDR & 0x3E00; 
  
  if (DEBUG)
  {
    printf("_EVAL_BUS_DRIVERS_\nAt state %d: \n__________________________\nGate_PC_Input is     x%x\nGate_MDR_Input is    x%x\nGate_ALU_Input is    x%x\nGate_MARMUX_Input is x%x\nGate_SHF_Input is    x%x\n__________________________\n",
      CURRENT_LATCHES.STATE_NUMBER,
      Gate_PC_Input,
      Gate_MDR_Input,
      Gate_ALU_Input,
      Gate_MARMUX_Input,
      Gate_SHF_Input);
  }
}

/**
* Drive appropriate tristate buffers to bus based on uCode
*/
void drive_bus()
{

  /*
  * Datapath routine for driving the bus from one of the 5 possible
  * tristate drivers.
  */
  int i, lDriveCount, debugvm, alreadyWritten;

  i = 0; 
  lDriveCount = 0; 
  debugvm = 0;  
  alreadyWritten = 0;
  
  BUS &= 0x0FFFF;
  BUS = 0; 
  

  /* This loop drives BUS, and makes sure only 1 buffer is driving bus */
  for (i = GATE_PC; i <= GATE_SHF; i += 1)
  {
    lDriveCount += CURRENT_LATCHES.MICROINSTRUCTION[i];
    if (lDriveCount == 1)
    {
      switch (i)
      {
      case GATE_PC:
        if (DEBUG_VM) { printf("GATE_PC driven to bus, value %x\n", Gate_PC_Input); }
        BUS = Gate_PC_Input;
        break;
      case GATE_MDR:
        if (DEBUG_VM) { printf("GATE_MDR driven to bus, value %x\n", Gate_MDR_Input); }
        BUS = Gate_MDR_Input;
        break;
      case GATE_ALU:
        if (DEBUG_VM) { printf("GATE_ALU driven to bus, value %x\n", Gate_ALU_Input); }
        BUS = Gate_ALU_Input;
        break;
      case GATE_MARMUX:
        if (DEBUG_VM) { printf("GATE_MARMUX driven to bus, value %x\n", Gate_MARMUX_Input); }
        BUS = Gate_MARMUX_Input;
        break;
      case GATE_SHF:
        if (DEBUG_VM) { printf("GATE_SHF driven to bus, value %x\n", Gate_SHF_Input); }
        BUS = Gate_SHF_Input;
        break;
      }
      lDriveCount += 1;
    }
  }

  for (i = Gate_PSR; i <= Gate_IE_V; i += 1)
  {
    lDriveCount += CURRENT_LATCHES.MICROINSTRUCTION[i];
    if (lDriveCount == 1)
    {
      switch (i)
      {
      case Gate_PSR:
        if (DEBUG_VM) { printf("Gate_PSR driven to bus, value %x\n", Gate_PSR_Input); }
        BUS = Gate_PSR_Input;
        break;
      case Gate_USP_RES:
        if (DEBUG_VM) { printf("Gate_USP_RES driven to bus, value %x\n", Gate_USP_RES_Input); }
        BUS = Gate_USP_RES_Input;
        break;
      case Gate_SSP_RES:
        if (DEBUG_VM) { printf("Gate_SSP_RES driven to bus, value %x\n", GATE_SSP_RES_Input); }
        BUS = GATE_SSP_RES_Input;
        break;
      case Gate_SP:
        if (DEBUG_VM) { printf("Gate_SP driven to bus, value %x\n", GATE_SP_Input); }
        BUS = GATE_SP_Input;
        break;
      case Gate_IE_V:
        if (DEBUG_VM) { printf("Gate_IE_V driven to bus, value %x\n", GATE_IE_V_Input); }
        BUS = GATE_IE_V_Input;
        break;
      }
      lDriveCount += 1;
    }
  }

  lDriveCount += CURRENT_LATCHES.MICROINSTRUCTION[GATE_DEC_PC];
  /* TODO sloppy */
  if (CURRENT_LATCHES.MICROINSTRUCTION[GATE_DEC_PC] > 0)
  {
    if (DEBUG_VM) { printf("GATE_DEC_PC driven to bus, value %x\n", Gate_DEC_PC_Input); }
    BUS = Gate_DEC_PC_Input;
  }

  /* Only one output can be gated to bus at a time */
  if (lDriveCount  > 2)
  {
    printf("Error: Multiple gate tristate buffers driving bus.\nExiting...\n");
    exit(-1);
  }
  alreadyWritten = lDriveCount; 
  lDriveCount = 0; 
  for (i = GATE_PTBR; i <= GATE_PFN; i += 1)
  {
    lDriveCount += CURRENT_LATCHES.MICROINSTRUCTION[i];
    if ((lDriveCount >= 1) && (alreadyWritten > 0))
    {
      printf("Too many writes to BUS. Exiting..."); 
      exit (-1); 
    }  
    if (lDriveCount >= 1)
    {
      switch (i)
      {
      case GATE_PTBR: 
        if (DEBUG_VM) { printf("Gating PTBR, ORing onto bus: x%x\n", GatePTBR); }
        BUS |= GatePTBR;
        debugvm = 1;  
        break; 
      case GATE_VA_OFFSET:
        if (DEBUG_VM) { printf("Gating VA_Offset, ORing onto bus: x%x\n", GatePageOffset); } 
        BUS |= GatePageOffset; 
        debugvm = 1; 
        break; 
      case GATE_VA_IND_OFFSET_PN: 
        if (DEBUG_VM) { printf("Gating Indexed Page Number, ORing onto bus: x%x\n", GateIndexedPageNo); }         
        BUS |= GateIndexedPageNo; 
        debugvm = 1; 
        break; 
      case GATE_PFN:
        if (DEBUG_VM) { printf("Gating PFN, ORing onto bus: x%x\n", GatePFN); } 
        BUS |= GatePFN; 
        debugvm = 1; 
        break; 
      }
    }
    lDriveCount = 0; 
  }
  
  BUS &= 0xFFFF; 
  if ((debugvm == 1) && (DEBUG_VM == 1))
    printf("BUS = %x\n", BUS); 
  
  return;
}

void updateNextMicroinstruction()
{
  int x;
  for (x = 0; x < CONTROL_STORE_BITS; x += 1)
    NEXT_LATCHES.MICROINSTRUCTION[x] = CONTROL_STORE[NEXT_LATCHES.STATE_NUMBER][x];
}

/* TODO: shouldn't depend on NEXT_LATCHES.STATE_NUMBER */
void updateUSequencer()
{
  /* Deprecated from lab4
  if (GETCLD_E1(CURRENT_LATCHES.MICROINSTRUCTION) > 0)
  {
    if (((BUS & 0xFFFF) <= EO_PRIV_SPACE) && ((CURRENT_LATCHES.PSR & 0x8000) > 0))
    {
      if (DEBUG_INT) { printf("Taking E1 microbranch at cycle %d\n", CYCLE_COUNT); }
      NEXT_LATCHES.STATE_NUMBER |= E1Bits;
      updateNextMicroinstruction();
    }
  }
  */ 

  if (GETCLD_E2(CURRENT_LATCHES.MICROINSTRUCTION) > 0)
  {
    if ((CYCLE_COUNT >= Int_Cycle) && (TIE == 0))
    {
      if (DEBUG_INT) { printf("Taking E2 microbranch at cycle %d\n", CYCLE_COUNT); }
      NEXT_LATCHES.STATE_NUMBER = 37; /* TODO */
      updateNextMicroinstruction();
      TIE = 1;
    }
  }
  
  if (GETCLD_E3(CURRENT_LATCHES.MICROINSTRUCTION) > 0)
  {
    if (((BUS & 0xFFFF) > EO_TRAP_SPACE) && ((BUS <= EO_PRIV_SPACE)))
    {
      if (DEBUG_INT) { printf("Taking E3 microbranch at cycle %d\n", CYCLE_COUNT); }
      NEXT_LATCHES.STATE_NUMBER |= E3Bits;
      updateNextMicroinstruction();
    }
  }

  if (GETCLD_MAR0(CURRENT_LATCHES.MICROINSTRUCTION) > 0)
  {
    if ((CURRENT_LATCHES.MAR & 0x0001) > 0)
    {
      if (DEBUG_INT) { printf("Taking MAR[0] microbranch at cycle %d\n", CYCLE_COUNT); }
      NEXT_LATCHES.STATE_NUMBER |= MAR0Bits;
      updateNextMicroinstruction();
    }
  }

  if (GETCLD_PSR(CURRENT_LATCHES.MICROINSTRUCTION) > 0)
  {
    if ((CURRENT_LATCHES.PSR & 0x8000) == 0)
    {
      if (DEBUG_INT) { printf("Taking PSR microbranch at cycle %d\n", CYCLE_COUNT); }
      NEXT_LATCHES.STATE_NUMBER |= PSR15Bits;
      updateNextMicroinstruction();
    }
  }
  /* Virtual memory additions */
  if (GetVT_PSR(CURRENT_LATCHES.MICROINSTRUCTION) > 0)
  {
    if ((CURRENT_LATCHES.PSR & 0x8000) == 0)
    {
      if (DEBUG_VM) { printf("Taking PSR microbranch, skipping translation: cycle %d\n", CYCLE_COUNT); }
      NEXT_LATCHES.STATE_NUMBER |= 0x03; /* TODO */
      updateNextMicroinstruction(); 
    }
  }
  /* these must happen at the end of the function */
  if (GetGOTO_TEMPJ(CURRENT_LATCHES.MICROINSTRUCTION))
  {
    if (DEBUG_VM) { printf("Going to TempJ, cycle %d\n", CYCLE_COUNT); }
    NEXT_LATCHES.STATE_NUMBER = CURRENT_LATCHES.TempJ; 
    updateNextMicroinstruction(); 
  }
  /* final MUX of uSequencer */
  if (GETCLD_E1(CURRENT_LATCHES.MICROINSTRUCTION))
  {
    if ( ((CURRENT_LATCHES.MDR & VBit) == 0) || 
       ( ((CURRENT_LATCHES.PSR & 0x8000) > 0) && ((CURRENT_LATCHES.MDR & PBit) == 0)) ) 
    {
      if (DEBUG_VM) 
      { 
        printf("Taking Protected Exception E1, cycle %d\n", CYCLE_COUNT); 
        printf("MDR (PTE) is: x%x\n", CURRENT_LATCHES.MDR); 
        printf("VBit is: x%x\n", (CURRENT_LATCHES.MDR & VBit)); 
        printf("PSR priv is: x%x\n", (CURRENT_LATCHES.PSR & 0x8000)); 
        printf("Protect bit: x%x\n", (CURRENT_LATCHES.MDR & PBit)); 
      }
      NEXT_LATCHES.STATE_NUMBER = 0x3D; 
      updateNextMicroinstruction(); 
    }
  }
}


/**
* Latches all values at the end of the clock cycle
*   emulates all logic that latches into data path
* Latches:
*   PC, MDR, MAR, IR, CC's, BEN, Destination Register
*/
void latch_datapath_values()
{
  int temp, ln, lz, lp;

  if (DEBUG)
  {
    printf("_LATCH_DATAPATH_VALUES_\n");
    printf("_______________________\n");
  }

  /* Latch PC */
  if (GetLD_PC(CURRENT_LATCHES.MICROINSTRUCTION) == 1)
  {
    switch (GetPCMUX(CURRENT_LATCHES.MICROINSTRUCTION))
    {
    case 0x00: /* pc += 2 */
      NEXT_LATCHES.PC = CURRENT_LATCHES.PC + 2;
      break;
    case 0x01: /* pc <- gate */
      NEXT_LATCHES.PC = BUS;
      break;
    case 0x02:
      NEXT_LATCHES.PC = getMARMUXOut();
      break;
    }
    if (DEBUG) { printf("NEXT_LATCHES.PC <- 0x%x\n", NEXT_LATCHES.PC); }
  }
  else
    if (DEBUG) { printf("\n"); }

  /* Latch MDR */
  if (GetLD_MDR(CURRENT_LATCHES.MICROINSTRUCTION) == 1)
  {
    if (GetWR_REF_MDR_SEL(CURRENT_LATCHES.MICROINSTRUCTION) == 0)
    {
      /* Emulate MIO.EN selected MUX */
      if (GetMIO_EN(CURRENT_LATCHES.MICROINSTRUCTION) == 1)
      { /* Data from Memory */
        NEXT_LATCHES.MDR = MemToLatch;
      }
      else
      { /* Data from Bus */
        if (GetDATA_SIZE(CURRENT_LATCHES.MICROINSTRUCTION) == 1)
        { /* Word Size */
          NEXT_LATCHES.MDR = BUS;
        }
        else
        { /* Byte Size */
          NEXT_LATCHES.MDR = 0;
          NEXT_LATCHES.MDR |= (BUS & 0x0FF);
          NEXT_LATCHES.MDR |= (BUS & 0x0FF) << 8;
        }
      }

    }
    else
    {
      /* MDR has PTE, set/clr Modify and Reference Bits */
      if (DEBUG_VM)
        printf("Loading MDR with Write and REF bits\n");

      if (GetWRITE_ENABLE(CURRENT_LATCHES.MICROINSTRUCTION) > 0)
      {
        if (CURRENT_LATCHES.Write > 0)
        {
          NEXT_LATCHES.MDR |= MBit; 
        }
        else
          NEXT_LATCHES.MDR &= notMBit;
        NEXT_LATCHES.MDR &= 0xFFFF;  
      }
      if (GetREF_BIT_ENABLE(CURRENT_LATCHES.MICROINSTRUCTION) > 0)
      {
        if (GetSET_REFBIT_SEL(CURRENT_LATCHES.MICROINSTRUCTION) > 0)
        {
          NEXT_LATCHES.MDR |= RBit; 
        }
        else 
          NEXT_LATCHES.MDR &= notRBit; 
        NEXT_LATCHES.MDR &= 0xFFFF; 
      }
      if (DEBUG_VM)
      {
        printf("****************************************************\n"); 
        printf("MDR was: x%x\n", CURRENT_LATCHES.MDR); 
        printf("MDR set to: x%x\n", NEXT_LATCHES.MDR);
        printf("Write was: x%x\n", CURRENT_LATCHES.Write);
        printf("RefBit is: x%x\n", GetREF_BIT_ENABLE(CURRENT_LATCHES.MICROINSTRUCTION)); 
      }
    }
    if (DEBUG) { printf("NEXT_LATCHES.MDR <- 0x%x\n", NEXT_LATCHES.MDR); }
  }
  else
    if (DEBUG) { printf("\n"); }

  /* Latch MAR */
  if (GetLD_MAR(CURRENT_LATCHES.MICROINSTRUCTION) == 1)
  {
    NEXT_LATCHES.MAR = BUS;
    if (DEBUG_INT) { printf("NEXT_LATCHES.MAR <- 0x%x\n", NEXT_LATCHES.MAR); }
  }
  else
    if (DEBUG_INT) { printf("\n"); }


  /* Latch IR */
  if (GetLD_IR(CURRENT_LATCHES.MICROINSTRUCTION) == 1)
  {
    NEXT_LATCHES.IR = BUS;
    if (DEBUG) { printf("NEXT_LATCHES.IR <- 0x%x\n", NEXT_LATCHES.IR); }
  }
  else
    if (DEBUG) { printf("\n"); }


  /* Latch NZP */
  if (GetLD_CC(CURRENT_LATCHES.MICROINSTRUCTION) == 1)
  {
    if ((BUS & 0x8000) == 0x8000)
    {
      NEXT_LATCHES.N = 1;
    }
    else { NEXT_LATCHES.N = 0; }
    if (BUS == 0)
    {
      NEXT_LATCHES.Z = 1;
    }
    else { NEXT_LATCHES.Z = 0; }
    if (((BUS & 0x8000) == 0) && (BUS != 0))
    {
      NEXT_LATCHES.P = 1;
    }
    else { NEXT_LATCHES.P = 0; }
    if (DEBUG) { printf("NZP <- %x%x%x", NEXT_LATCHES.N, NEXT_LATCHES.Z, NEXT_LATCHES.P); }
  }
  else
    if (DEBUG) { printf("\n"); }

  /* Latch BEN */
  if (GetLD_BEN(CURRENT_LATCHES.MICROINSTRUCTION) == 1)
  {
    ln = (((CURRENT_LATCHES.IR & 0x0800) > 0) && (NEXT_LATCHES.N > 0));
    lz = (((CURRENT_LATCHES.IR & 0x0400) > 0) && (NEXT_LATCHES.Z > 0));
    lp = (((CURRENT_LATCHES.IR & 0x0200) > 0) && (NEXT_LATCHES.P > 0));
    if (DEBUG) { printf("In calculating BEN, nzp = %d%d%d\n", ln, lz, lp); }
    if (ln || lz || lp)
    {
      if (DEBUG)
      {
        printf("BEN was set to 1: (nzp & IR) = (%d%d%d)\n", ln, lz, lp);
      }
      NEXT_LATCHES.BEN = 1;
    }
    else
    {
      NEXT_LATCHES.BEN = 0;
    }
    if (DEBUG) { printf("NEXT_LATCHES.BEN <- 0x%x\n", NEXT_LATCHES.BEN); }
  }
  else
    if (DEBUG) { printf("\n"); }

  /* Latch Destination Register */
  if (GetLD_REG(CURRENT_LATCHES.MICROINSTRUCTION) == 1)
  {
    NEXT_LATCHES.REGS[getDRMuxOut()] = BUS;
    if (DEBUG) { printf("NEXT_LATCHES.REG[%d] <- 0x%x\n", getDRMuxOut(), BUS); }
  }
  else
  {
    if (DEBUG) { printf("\n"); }
  }
  if (DEBUG)
  {
    printf("_______________________\n");
  }


  /***************** new interrupt logic *****************/
  /* update next state bits with current bus information */
  updateUSequencer();

  /* LD_USP_RES */
  if (GetLD_USP_RES(CURRENT_LATCHES.MICROINSTRUCTION) > 0)
  {
    NEXT_LATCHES.USP_RES = getSR1Data();
    if (DEBUG_INT) { printf("USP_RES = 0x%x\n", getSR1Data()); }
  }

  /* LD_SSP_RES */
  if (GetLD_SSP_RES(CURRENT_LATCHES.MICROINSTRUCTION) > 0)
  {
    NEXT_LATCHES.SSP = getSR1Data();
    if (DEBUG_INT) { printf("SSP = 0x%x\n", getSR1Data()); }
  }

  /* LD_PSR */
  if (GetLD_PSR(CURRENT_LATCHES.MICROINSTRUCTION) > 0)
  {
    updatePSR();
  }

  /* LD_TEMP_V */
  if (GetLD_TEMP_V(CURRENT_LATCHES.MICROINSTRUCTION) > 0)
  {
    if (DEBUG_INT) { printf("VMUX = 0x%x\n", GetVMUX(CURRENT_LATCHES.MICROINSTRUCTION)); }
    switch (GetVMUX(CURRENT_LATCHES.MICROINSTRUCTION))
    {
    case 0x00:
      NEXT_LATCHES.Temp_V = INT_V_Time;
      break;
    case 0x01:
      if (((CURRENT_LATCHES.PSR & 0x8000) > 0) && ((CURRENT_LATCHES.MDR & PBit) == 0))
        NEXT_LATCHES.Temp_V = PME_V; 
      NEXT_LATCHES.Temp_V = PFE_V;
      break;
    case 0x02:
      NEXT_LATCHES.Temp_V = UAE_V;
      break;
    case 0x03:
      NEXT_LATCHES.Temp_V = UOE_V;
      break;
    }
    if (DEBUG_INT) { printf("tempv loaded with : x%x\n", NEXT_LATCHES.Temp_V); }
  }

  /********************** Virtual Mem Logic ***********************/
  /* set/clear next.write */
  if (GetLD_WR(CURRENT_LATCHES.MICROINSTRUCTION) > 0)
  {
    if (GetSET_WRITE(CURRENT_LATCHES.MICROINSTRUCTION) > 0)
      NEXT_LATCHES.Write = 1; 
    if (GetCLR_WRITE(CURRENT_LATCHES.MICROINSTRUCTION) > 0)
      NEXT_LATCHES.Write = 0; 
    if (DEBUG_VM)
      printf("Write set to: x%x\n", NEXT_LATCHES.Write); 
  }
  /* ld.va */
  if (GetLD_VA(CURRENT_LATCHES.MICROINSTRUCTION) > 0)
  {
    NEXT_LATCHES.VA = BUS; 
    if (DEBUG_VM)
      printf("VA set to: x%x\n", NEXT_LATCHES.VA); 
  }
  /* TempJ logic */
  if (GetLD_TEMPJ(CURRENT_LATCHES.MICROINSTRUCTION) > 0)
  {
    switch (GetTEMPJ_SEL(CURRENT_LATCHES.MICROINSTRUCTION))
    {
    case 0x00:
      NEXT_LATCHES.TempJ = TJ2Bits; 
      break; 
    case 0x01: 
      NEXT_LATCHES.TempJ = TJ3Bits; 
      break; 
    case 0x02: 
      NEXT_LATCHES.TempJ = TJ1819Bits; 
      break; 
    case 0x03: 
      NEXT_LATCHES.TempJ = TJ55Bits; 
      break; 
    case 0x04: 
      NEXT_LATCHES.TempJ = TJ57Bits; 
      break;
    }
    if (DEBUG_VM)
      printf("TempJ set to: x%x\n", NEXT_LATCHES.TempJ); 
  }
  if (DEBUG_VM)
        printf("_______________________\n");
}
