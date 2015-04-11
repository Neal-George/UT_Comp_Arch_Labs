/*
Name 1: George Neal
UTEID 1: gln276
*/

/***************************************************************/
/*                                                             */
/*   LC-3b Simulator                                           */
/*                                                             */
/*   EE 460N                                                   */
/*   The University of Texas at Austin                         */
/*                                                             */
/***************************************************************/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/***************************************************************/
/*                                                             */
/* Files:  ucode        Microprogram file                      */
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

#define WORDS_IN_MEM    0x08000 
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
  /* The ready bit is also latched as you dont want the memory system to
  assert it
  at a bad point in the cycle*/

  int REGS[LC_3b_REGS]; /* register file. */

  int MICROINSTRUCTION[CONTROL_STORE_BITS]; /* The microintruction */

  int STATE_NUMBER; /* Current State Number - Provided for debugging */
} System_Latches;

/* Data Structure for Latch */

System_Latches CURRENT_LATCHES, NEXT_LATCHES;

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

/************************************************************/
/*                                                          */
/* Procedure : init_memory                                  */
/*                                                          */
/* Purpose   : Zero out the memory array                    */
/*                                                          */
/************************************************************/
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
void load_program(char *program_filename) {
  FILE * prog;
  int ii, word, program_base;

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
    MEMORY[program_base + ii][1] = (word >> 8) & 0x00FF;
    ii++;
  }

  if (CURRENT_LATCHES.PC == 0) CURRENT_LATCHES.PC = (program_base << 1);

  printf("Read %d words from program into memory.\n\n", ii);
}

/***************************************************************/
/*                                                             */
/* Procedure : initialize                                      */
/*                                                             */
/* Purpose   : Load microprogram and machine language program  */
/*             and set up initial state of the machine.        */
/*                                                             */
/***************************************************************/
void initialize(char *ucode_filename, char *program_filename, int num_prog_files) {
  int i;
  init_control_store(ucode_filename);

  init_memory();
  for (i = 0; i < num_prog_files; i++) {
    load_program(program_filename);
    while (*program_filename++ != '\0');
  }
  CURRENT_LATCHES.Z = 1;
  CURRENT_LATCHES.STATE_NUMBER = INITIAL_STATE_NUMBER;
  memcpy(CURRENT_LATCHES.MICROINSTRUCTION, CONTROL_STORE[INITIAL_STATE_NUMBER], sizeof(int)*CONTROL_STORE_BITS);

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
  if (argc < 3) {
    printf("Error: usage: %s <micro_code_file> <program_file_1> <program_file_2> ...\n",
      argv[0]);
    exit(1);
  }

  printf("LC-3b Simulator\n\n");

  initialize(argv[1], argv[2], argc - 2);

  if ((dumpsim_file = fopen("dumpsim", "w")) == NULL) {
    printf("Error: Can't open dumpsim file\n");
    exit(-1);
  }

  while (1)
    get_command(dumpsim_file);

}

/***************************************************************/
/* Do not modify the above code.
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
#define DEBUG 0

/* Globals */
int MemToLatch;
int Gate_PC_Input;
int Gate_MDR_Input;
int Gate_ALU_Input;
int Gate_MARMUX_Input;
int Gate_SHF_Input;

int AssmInstr = 0; 

/* Assumes 1 indexed x and y */
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

/* outputs 3 bits */
int getSR1MuxOut()
{
  int returnVal = 0;
  if (GetSR1MUX(CURRENT_LATCHES.MICROINSTRUCTION) == 1)
  {
    returnVal = ((CURRENT_LATCHES.IR & 0x01C0) >> 6);
  }
  else
  {
    returnVal = ((CURRENT_LATCHES.IR & 0x0E00) >> 9);
  }
  if (DEBUG)
  {
    printf("SR1MUX with input 0x%x output 0x%x\n",
      GetSR1MUX(CURRENT_LATCHES.MICROINSTRUCTION), returnVal);
  }
  return returnVal & 0x0FFFF;
}

int getSR1Data()
{
  int returnVal;
  
  returnVal = CURRENT_LATCHES.REGS[getSR1MuxOut()];
  
  if (DEBUG)
  {
    printf("Data from SR1 is 0x%x\n",returnVal);
  }
  return returnVal & 0x0FFFF;
}

/* outputs 16bits */
int getSR2MuxOut()
{
  int returnVal, reg;

  returnVal = reg = 0;

  if ((CURRENT_LATCHES.IR & 0x020) > 0)
  { 
    if (DEBUG) { printf("IR[5] = %d, imm mode\n",((CURRENT_LATCHES.IR & 0x020) > 0));}
    returnVal = SextXtoY((CURRENT_LATCHES.IR & 0x01F), 5, 16);
  }
  else
  {
    /* otherwise use sr2 (bottom 3 bits) */
    if (DEBUG) { printf("IR[5] = %d, reg mode\n",((CURRENT_LATCHES.IR & 0x020) > 0));}
    reg = (CURRENT_LATCHES.IR & 0x07);
    returnVal = CURRENT_LATCHES.REGS[reg];
  }

  if (DEBUG)
  {
    printf("Data from SR2MUX is 0x%x\n",returnVal); 
  }
  return returnVal & 0x0FFFF; 
}

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
  if (DEBUG)
  {
    printf("DRMUX with input 0x%x output 0x%x\n",
      GetDRMUX(CURRENT_LATCHES.MICROINSTRUCTION), returnVal);
  }
  return returnVal & 0x0FFFF;
}

int getLshftAddr1MuxOut()
{
  int lAddr2Mux, lAddr1Mux, lAddrOutput;
  lAddr2Mux = lAddr1Mux = lAddrOutput = 0;

  /* Emulate logic for addr2mux and lshft */
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
  if (GetLSHF1(CURRENT_LATCHES.MICROINSTRUCTION) == 1)
  {
    lAddr2Mux *= 2;
  }
  lAddr2Mux &= 0x0FFFF;

  /* Emulate logic for Addr1MUX */
  if (GetADDR1MUX(CURRENT_LATCHES.MICROINSTRUCTION) == 1)
  {
    lAddr1Mux = CURRENT_LATCHES.REGS[getSR1MuxOut()];
  }
  else
  {
    lAddr1Mux = CURRENT_LATCHES.PC;
  }

  /* Add both results and return */
  lAddrOutput = (lAddr2Mux + lAddr1Mux) & 0x0FFFF;

  if (DEBUG)
  {
    printf("_getLshftAddr1MuxOut_\nAstate %d: \n____________________\nlAddr2Mux is   x%x\nlAddr1Mux is   x%x\nlAddrOutput is x%x\n____________________\n", 
      CURRENT_LATCHES.STATE_NUMBER,
      lAddr2Mux,
      lAddr1Mux,
      lAddrOutput);
  }

  return lAddrOutput;
}

int getALUKOutput()
{
  int returnVal;

  returnVal = 0;

  switch (GetALUK(CURRENT_LATCHES.MICROINSTRUCTION))
  {
  case 0: /* ADD */
    returnVal = (getSR2MuxOut()) +
      (getSR1Data());
    if (DEBUG) { printf("_GETALUKOUTPUT_\nAdding 0x%x and 0x%x\nResult: 0x%x\n________________\n",
                 getSR2MuxOut(), getSR1Data(), returnVal); }
    break;
  case 1: /* AND */
    returnVal = (getSR2MuxOut()) &
      (getSR1Data());
    if (DEBUG) { printf("_GETALUKOUTPUT_\nAnding 0x%x and 0x%x\nResult: 0x%x\n_________________\n",
                 getSR2MuxOut(), getSR1Data(), returnVal); }
    break;
  case 2: /* XOR */
    returnVal = (getSR2MuxOut()) ^
      (getSR1Data());
        if (DEBUG) { printf("_GETALUKOUTPUT_\nXORing 0x%x and 0x%x\nResult: 0x%x\n_________________\n",
                     getSR2MuxOut(), getSR1Data(), returnVal); }
    break;
  case 3: /* PASSA (SR1's data) */
    returnVal = getSR1Data();
      if (DEBUG) { printf("_GETALUKOUTPUT_\nPassing 0x%x through\n_________________\n",
                   returnVal); }
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
    printf("SHF Output: 0x%x\n",returnVal);
  }

  return returnVal;
}

void eval_micro_sequencer()
{

  /*
  * Evaluate the address of the next state according to the
  * micro sequencer logic. Latch the next microinstruction.
  */

  int lIR15to12, lIR11, lCurrentState, lCOND10, lBEN, lRBit, lIRD, lJBits;
  int x, nextState, lCOND1, lCOND0, testBit;

  /* acquire all necessary bits of from current microinstruction */
  x = CURRENT_LATCHES.IR;
  if (DEBUG) { printf("IR: 0x%x\n",x); }
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

  /* for debugging */ 
  if (CURRENT_LATCHES.STATE_NUMBER == 32)
  {
    AssmInstr += 1; 
  }
  if (DEBUG)
  {
    printf("Current Instruction Count: %d\n",AssmInstr);
  }

  /*  Calculate low 3 J bits for Branch, Mem Ready, and Addr Mode */
  lJBits |= ( ((lCOND1 != 0) && (lCOND0 == 0) && (lBEN != 0) ) << 2);
  lJBits |= ( ((lCOND1 == 0) && (lCOND0 != 0) && (lRBit != 0)) << 1);
  lJBits |= ( ((lCOND1 != 0) && (lCOND0 != 0) && (lIR11 != 0)     ));

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
    if (DEBUG) { printf("%d",(CONTROL_STORE[nextState][x])); }
  }
  if (DEBUG) {printf("\n");} 

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
}

void cycle_memory()
{

  /*
  * This function emulates memory and the WE logic.
  * Keep track of which cycle of MEMEN we are dealing with.
  * If fourth, we need to latch Ready bit at the end of
  * cycle to prepare microsequencer for the fifth cycle.
  */

  int lWE, lMemToWrite, lMAR0Bit;
  lWE = lMemToWrite = 0;

  /* If memory is being accessed this cycle, increment R_BitMemCtr */
  if (GetMIO_EN(CURRENT_LATCHES.MICROINSTRUCTION) > 0)
  {
    NEXT_LATCHES.READY = (CURRENT_LATCHES.READY + 1) % MEM_CYCLES;
    
    if (DEBUG) { printf("Attempting to access memory...\n"); }

    /* If it is the (MEM_CYCLES-1)th cycle, output mem */
    /* TODO: try this with MEM_CYCLES -2 if there are problems */
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
            printf("MAR 0 bit is %d\n",lMAR0Bit);
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
        if(DEBUG)
        {
          printf("MemToLatch is driving 0x%x\n",MemToLatch);  
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
          MEMORY[CURRENT_LATCHES.MAR >> 1][0] = lMemToWrite & 0x00FF;
          if (DEBUG)
          {
            printf("Memory[%x][0] <= 0x%x\n", (CURRENT_LATCHES.MAR >> 1), (lMemToWrite & 0x00FF)); 
          }
        }
        if (lWE & 0x02)
        {
          MEMORY[CURRENT_LATCHES.MAR >> 1][1] = (lMemToWrite & 0x0FF00) >> 8;
          if (DEBUG)
          {
            printf("Memory[%x][1] <= 0x%x\n", (CURRENT_LATCHES.MAR >> 1), ((lMemToWrite & 0x0FF00) >> 8)); 
          }
        }
        MemToLatch = 0;
      }
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

void eval_bus_drivers()
{

  /*
  * Datapath routine emulating operations before driving the bus.
  * Evaluate the input of tristate drivers
  *     Gate_PC_Input
  *     Gate_MDR_Input
  *     Gate_ALU_Input
  *     Gate_MARMUX_Input
  *     Gate_SHF_Input
  */

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
    Gate_MARMUX_Input = getLshftAddr1MuxOut();
  }

  /* Evaluate input for GateSHF */
  Gate_SHF_Input = getSHFOutput();

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

void drive_bus()
{

  /*
  * Datapath routine for driving the bus from one of the 5 possible
  * tristate drivers.
  */
  int i, lDriveCount;

  i = lDriveCount = 0;
  BUS &= 0x0FFFF; 

  /* make sure only 1 gate tristate buffer is driving bus */
  for (i = GATE_PC; i <= GATE_SHF; i += 1)
  {
    lDriveCount += CURRENT_LATCHES.MICROINSTRUCTION[i];
    if (lDriveCount == 1)
    {
      switch (i)
      {
      case GATE_PC:
        if (DEBUG) { printf("GATE_PC driven to bus, value %x\n", Gate_PC_Input); }
        BUS = Gate_PC_Input;
        break;
      case GATE_MDR:
        if (DEBUG) { printf("GATE_MDR driven to bus, value %x\n", Gate_MDR_Input); }
        BUS = Gate_MDR_Input;
        break;
      case GATE_ALU:
        if (DEBUG) { printf("GATE_ALU driven to bus, value %x\n", Gate_ALU_Input); }
        BUS = Gate_ALU_Input;
        break;
      case GATE_MARMUX:
        if (DEBUG) { printf("GATE_MARMUX driven to bus, value %x\n", Gate_MARMUX_Input); }
        BUS = Gate_MARMUX_Input;
        break;
      case GATE_SHF:
        if (DEBUG) { printf("GATE_SHF driven to bus, value %x\n", Gate_SHF_Input); }
        BUS = Gate_SHF_Input;
        break;
      }
      lDriveCount += 1; 
    }
  }

  if (lDriveCount == 0)
  {
    BUS = 0;
    if (DEBUG) { printf("Bus driven to 0\n"); }
  }

  /* Only one output can be gated to bus at a time */
  if (lDriveCount  > 2)
  {
    printf("Error: Multiple gate tristate buffers driving bus.\nExiting...\n"); 
      exit(-1);
  }

}

void latch_datapath_values()
{
  /*
  * Datapath routine for computing all functions that need to latch
  * values in the data path at the end of this cycle.  Some values
  * require sourcing the bus; therefore, this routine has to come
  * after drive_bus.
  *
  * PC
  * MDR
  * MAR
  * IR?
  * N?
  * Z?
  * P?
  * BEN
  * REGS
  * MICROINSTRUCTION
  *
  */

  int temp, ln, lz, lp;

  if(DEBUG)
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
      NEXT_LATCHES.PC = getLshftAddr1MuxOut();
      break;
    }
    if (DEBUG) { printf("NEXT_LATCHES.PC <- 0x%x\n",NEXT_LATCHES.PC); }
  }
  else 
    if (DEBUG) { printf("\n"); }

  /* Latch MDR */
  if (GetLD_MDR(CURRENT_LATCHES.MICROINSTRUCTION) == 1)
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
    if (DEBUG) { printf("NEXT_LATCHES.MDR <- 0x%x\n",NEXT_LATCHES.MDR); }
  }
  else 
    if (DEBUG) { printf("\n"); }

  /* Latch MAR */
  if (GetLD_MAR(CURRENT_LATCHES.MICROINSTRUCTION) == 1)
  {
    NEXT_LATCHES.MAR = BUS;
    if (DEBUG) { printf("NEXT_LATCHES.MAR <- 0x%x\n",NEXT_LATCHES.MAR); }
  }
  else 
    if (DEBUG) { printf("\n"); }


  /* Latch IR */
  if (GetLD_IR(CURRENT_LATCHES.MICROINSTRUCTION) == 1)
  {
    NEXT_LATCHES.IR = BUS;
    if (DEBUG) { printf("NEXT_LATCHES.IR <- 0x%x\n",NEXT_LATCHES.IR); }
  }
  else 
    if (DEBUG) { printf("\n"); }


  /* Latch NZP */
  if (GetLD_CC(CURRENT_LATCHES.MICROINSTRUCTION) == 1)
  {
    if ((BUS & 0x8000) == 0x8000)
    {
      NEXT_LATCHES.N = 1;
    } else { NEXT_LATCHES.N = 0; }
    if (BUS == 0)
    {
      NEXT_LATCHES.Z = 1;
    } else { NEXT_LATCHES.Z = 0; }
    if (((BUS & 0x8000) == 0) && (BUS != 0))
    {
      NEXT_LATCHES.P = 1;
    } else { NEXT_LATCHES.P = 0; }
    if (DEBUG) { printf("NZP <- %x%x%x", NEXT_LATCHES.N, NEXT_LATCHES.Z, NEXT_LATCHES.P ); }
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
    if (ln  || lz || lp)
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
    if (DEBUG) { printf("NEXT_LATCHES.BEN <- 0x%x\n",NEXT_LATCHES.BEN); }
  }
  else 
    if (DEBUG) { printf("\n"); }

  /* Latch Destination Register */
  if (GetLD_REG(CURRENT_LATCHES.MICROINSTRUCTION) == 1)
  {
    NEXT_LATCHES.REGS[getDRMuxOut()] = BUS;
    if (DEBUG) { printf("NEXT_LATCHES.REG[%d] <- 0x%x\n",getDRMuxOut(), BUS); }
  }
  else 
    if (DEBUG) { printf("\n"); }
  if (DEBUG) 
  {
    printf("_______________________\n");
  }
}





