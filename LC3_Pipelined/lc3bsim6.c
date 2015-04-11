/*
Name 1: George Neal
UTEID 1: gln276
*/

/*                                                             */
/*   LC-3b Simulator - Lab 6                                   */
/*                                                             */
/*   EE 460N -- Spring 2013                                    */
/*   The University of Texas at Austin                         */
/*                                                             */
/***************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/***************************************************************/
/*                                                             */
/* Files:  ucode        Microprogram file                      */
/*         isaprogram   LC-3b machine language program file    */
/*                                                             */
/***************************************************************/

/***************************************************************/
/* These are the functions you'll have to write.               */
/***************************************************************/
void FETCH_stage();
void DE_stage();
void AGEX_stage();
void MEM_stage();
void SR_stage();
/***************************************************************/
/* A couple of useful definitions.                             */
/***************************************************************/
#define TRUE  1
#define FALSE 0

/***************************************************************/
/* Use this to avoid overflowing 16 bits on the bus.           */
/***************************************************************/
#define Low16bits(x) ((x) & 0xFFFF)

/***************************************************************/
/* Definition of the control store layout.                     */
/***************************************************************/
#define CONTROL_STORE_ROWS 64

/***************************************************************/
/* Definition of bit order in control store word.              */
/***************************************************************/
/* control signals from the control store */
enum CS_BITS {
  SR1_NEEDED,
  SR2_NEEDED,
  DRMUX,
  
  ADDR1MUX,
  ADDR2MUX1, ADDR2MUX0, 
  LSHF1,
  ADDRESSMUX,
  SR2MUX, 
  ALUK1, ALUK0,
  ALU_RESULTMUX,

  BR_OP,
  UNCOND_OP,
  TRAP_OP,
  BR_STALL,
  
  DCACHE_EN,
  DCACHE_RW,
  DATA_SIZE,

  DR_VALUEMUX1, DR_VALUEMUX0,
  LD_REG,
  LD_CC,
  NUM_CONTROL_STORE_BITS

} CS_BITS;


enum AGEX_CS_BITS {

  AGEX_ADDR1MUX,
  AGEX_ADDR2MUX1, AGEX_ADDR2MUX0, 
  AGEX_LSHF1,
  AGEX_ADDRESSMUX,
  AGEX_SR2MUX,
  AGEX_ALUK1, AGEX_ALUK0, 
  AGEX_ALU_RESULTMUX,

  AGEX_BR_OP,
  AGEX_UNCOND_OP,
  AGEX_TRAP_OP,
  AGEX_BR_STALL,
  AGEX_DCACHE_EN,
  AGEX_DCACHE_RW,
  AGEX_DATA_SIZE,
  
  AGEX_DR_VALUEMUX1, AGEX_DR_VALUEMUX0,
  AGEX_LD_REG,
  AGEX_LD_CC,
  NUM_AGEX_CS_BITS
} AGEX_CS_BITS;

enum MEM_CS_BITS {
  MEM_BR_OP,
  MEM_UNCOND_OP,
  MEM_TRAP_OP,
  MEM_BR_STALL,
  MEM_DCACHE_EN,
  MEM_DCACHE_RW,
  MEM_DATA_SIZE,

  MEM_DR_VALUEMUX1, MEM_DR_VALUEMUX0,
  MEM_LD_REG,
  MEM_LD_CC,
  NUM_MEM_CS_BITS
} MEM_CS_BITS;

enum SR_CS_BITS {
  SR_DR_VALUEMUX1, SR_DR_VALUEMUX0,
  SR_LD_REG,
  SR_LD_CC,
  NUM_SR_CS_BITS
} SR_CS_BITS;


/***************************************************************/
/* Functions to get at the control bits.                       */
/***************************************************************/
int Get_SR1_NEEDED(int *x)     { return (x[SR1_NEEDED]); }
int Get_SR2_NEEDED(int *x)     { return (x[SR2_NEEDED]); }
int Get_DRMUX(int *x)          { return (x[DRMUX]);}
int Get_DE_BR_OP(int *x)       { return (x[BR_OP]); } 
int Get_ADDR1MUX(int *x)       { return (x[AGEX_ADDR1MUX]); }
int Get_ADDR2MUX(int *x)       { return ((x[AGEX_ADDR2MUX1] << 1) + x[AGEX_ADDR2MUX0]); }
int Get_LSHF1(int *x)          { return (x[AGEX_LSHF1]); }
int Get_ADDRESSMUX(int *x)     { return (x[AGEX_ADDRESSMUX]); }
int Get_SR2MUX(int *x)          { return (x[AGEX_SR2MUX]); }
int Get_ALUK(int *x)           { return ((x[AGEX_ALUK1] << 1) + x[AGEX_ALUK0]); }
int Get_ALU_RESULTMUX(int *x)  { return (x[AGEX_ALU_RESULTMUX]); }
int Get_BR_OP(int *x)          { return (x[MEM_BR_OP]); }
int Get_UNCOND_OP(int *x)      { return (x[MEM_UNCOND_OP]); }
int Get_TRAP_OP(int *x)        { return (x[MEM_TRAP_OP]); }
int Get_DCACHE_EN(int *x)      { return (x[MEM_DCACHE_EN]); }
int Get_DCACHE_RW(int *x)      { return (x[MEM_DCACHE_RW]); }
int Get_DATA_SIZE(int *x)      { return (x[MEM_DATA_SIZE]); } 
int Get_DR_VALUEMUX1(int *x)   { return ((x[SR_DR_VALUEMUX1] << 1 ) + x[SR_DR_VALUEMUX0]); }
int Get_AGEX_LD_REG(int *x)    { return (x[AGEX_LD_REG]); }
int Get_AGEX_LD_CC(int *x)     { return (x[AGEX_LD_CC]); }
int Get_MEM_LD_REG(int *x)     { return (x[MEM_LD_REG]); }
int Get_MEM_LD_CC(int *x)      { return (x[MEM_LD_CC]); }
int Get_SR_LD_REG(int *x)      { return (x[SR_LD_REG]); }
int Get_SR_LD_CC(int *x)       { return (x[SR_LD_CC]); }
int Get_DE_BR_STALL(int *x)    { return (x[BR_STALL]); }
int Get_AGEX_BR_STALL(int *x)  { return (x[AGEX_BR_STALL]); }
int Get_MEM_BR_STALL(int *x)   { return (x[MEM_BR_STALL]); }



/***************************************************************/
/* The control store rom.                                      */
/***************************************************************/
int CONTROL_STORE[CONTROL_STORE_ROWS][NUM_CONTROL_STORE_BITS];

/***************************************************************/
/* Main memory.                                                */
/***************************************************************/
/* MEMORY[A][0] stores the least significant byte of word at word address A
   MEMORY[A][1] stores the most significant byte of word at word address A 
   There are two write enable signals, one for each byte. WE0 is used for 
   the least significant byte of a word. WE1 is used for the most significant 
   byte of a word. */

#define WORDS_IN_MEM    0x08000 
int MEMORY[WORDS_IN_MEM][2];

/***************************************************************/
/* The LC-3b register file.                                      */
/***************************************************************/
#define LC3b_REGS 8
int REGS[LC3b_REGS];
/***************************************************************/
/* architectural state */
/***************************************************************/
int  PC,    /* program counter */
     N,   /* n condition bit */
     Z = 1, /* z condition bit */
     P;   /* p condition bit */ 
/***************************************************************/
/* LC-3b State info.                                             */
/***************************************************************/

typedef struct PipeState_Entry_Struct{
  
  /* DE latches */
int DE_NPC,
    DE_IR,
    DE_V,
    /* AGEX lateches */
    AGEX_NPC,
    AGEX_SR1, 
    AGEX_SR2,
    AGEX_CC,
    AGEX_IR,
    AGEX_DRID,
    AGEX_V,
    AGEX_CS[NUM_AGEX_CS_BITS],
    /* MEM latches */
    MEM_NPC,
    MEM_ALU_RESULT,
    MEM_ADDRESS,
    MEM_CC,
    MEM_IR,
    MEM_DRID,
    MEM_V,
    MEM_CS[NUM_MEM_CS_BITS],
    /* SR latches */
    SR_NPC, 
    SR_DATA,
    SR_ALU_RESULT, 
    SR_ADDRESS,
    SR_IR,
    SR_DRID,
    SR_V,
    SR_CS[NUM_SR_CS_BITS];
    
} PipeState_Entry;

/* data structure for latch */
PipeState_Entry PS, NEW_PS;

/* simulator signal */
int RUN_BIT;

/* Internal stall signals */ 
int   dep_stall,
      v_de_br_stall,
      v_agex_br_stall,
      v_mem_br_stall,
      mem_stall,
      icache_r;

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
    printf("rdump            -  dump the architectural state    \n");
    printf("idump            -  dump the internal state         \n");
    printf("?                -  display this help menu          \n");
    printf("quit             -  exit the program                \n\n");
}

void print_CS(int *CS, int num)
{
  int ii ;
  for ( ii = 0 ; ii < num; ii++) {
    printf("%d",CS[ii]);
  }
  printf("\n");
}
/***************************************************************/
/*                                                             */
/* Procedure : cycle                                           */
/*                                                             */
/* Purpose   : Execute a cycle                                 */
/*                                                             */
/***************************************************************/
void cycle() {
  NEW_PS = PS; 
  SR_stage();
  MEM_stage(); 
  AGEX_stage();
  DE_stage();
  FETCH_stage();
  PS = NEW_PS; 
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
  if (PC == 0x0000) {
      cycle();
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
    if ((RUN_BIT == FALSE) || (PC == 0x0000)) {
  printf("Can't simulate, Simulator is halted\n\n");
  return;
    }
    printf("Simulating...\n\n");
    /* initialization */
    while (PC != 0x0000)
      cycle();
    cycle();
      RUN_BIT = FALSE;
    printf("Simulator halted\n\n");
}

/***************************************************************/ 
/*                                                             */
/* Procedure : mdump                                           */
/*                                                             */
/* Purpose   : Dump a region of memory to the output file.     */
/*                                                             */
/***************************************************************/
void mdump(FILE * dumpsim_file, int start, int stop) {
    int address; /* this is a byte address */

    printf("\nMemory content [0x%04x..0x%04x] :\n", start, stop);
    printf("-------------------------------------\n");
    for (address = (start >> 1); address <= (stop >> 1); address++)
  printf("  0x%04x (%d) : 0x%02x%02x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
    printf("\n");

    /* dump the memory contents into the dumpsim file */
    fprintf(dumpsim_file, "\nMemory content [0x%04x..0x%04x] :\n", start, stop);
    fprintf(dumpsim_file, "-------------------------------------\n");
    for (address = (start >> 1); address <= (stop >> 1); address++)
  fprintf(dumpsim_file, " 0x%04x (%d) : 0x%02x%02x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
    fprintf(dumpsim_file, "\n");
    fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : rdump                                           */
/*                                                             */
/* Purpose   : Dump current architectural state  to the       */   
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void rdump(FILE * dumpsim_file) {
    int k; 

    printf("\nCurrent architectural state :\n");
    printf("-------------------------------------\n");
    printf("Cycle Count : %d\n", CYCLE_COUNT);
    printf("PC          : 0x%04x\n", PC);
    printf("CCs: N = %d  Z = %d  P = %d\n", N, Z, P);
    printf("Registers:\n");
    for (k = 0; k < LC3b_REGS; k++)
  printf("%d: 0x%04x\n", k, (REGS[k] & 0xFFFF));
    printf("\n");

    /* dump the state information into the dumpsim file */
    fprintf(dumpsim_file, "\nCurrent architectural state :\n");
    fprintf(dumpsim_file, "-------------------------------------\n");
    fprintf(dumpsim_file, "Cycle Count : %d\n", CYCLE_COUNT);
    fprintf(dumpsim_file, "PC          : 0x%04x\n", PC);
    fprintf(dumpsim_file, "CCs: N = %d  Z = %d  P = %d\n", N, Z, P);
    fprintf(dumpsim_file, "Registers:\n");
    for (k = 0; k < LC3b_REGS; k++)
  fprintf(dumpsim_file, "%d: 0x%04x\n", k, (REGS[k] & 0xFFFF));
    fprintf(dumpsim_file, "\n");
    fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : idump                                           */
/*                                                             */
/* Purpose   : Dump current internal state to the              */
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void idump(FILE * dumpsim_file) {
    int k; 

    printf("\nCurrent architectural state :\n");
    printf("-------------------------------------\n");
    printf("Cycle Count     : %d\n", CYCLE_COUNT);
    printf("PC              : 0x%04x\n", PC);
    printf("CCs: N = %d  Z = %d  P = %d\n", N, Z, P);
    printf("Registers:\n");
    for (k = 0; k < LC3b_REGS; k++)
  printf("%d: 0x%04x\n", k, (REGS[k] & 0xFFFF));
    printf("\n");
    
    printf("------------- Stall Signals -------------\n");
    printf("ICACHE_R        :  %d\n", icache_r);
    printf("DEP_STALL       :  %d\n", dep_stall);
    printf("V_DE_BR_STALL   :  %d\n", v_de_br_stall);
    printf("V_AGEX_BR_STALL :  %d\n", v_agex_br_stall);
    printf("MEM_STALL       :  %d\n", mem_stall);
    printf("V_MEM_BR_STALL  :  %d\n", v_mem_br_stall);    
    printf("\n");

    printf("------------- DE   Latches --------------\n");
    printf("DE_NPC          :  0x%04x\n", PS.DE_NPC );
    printf("DE_IR           :  0x%04x\n", PS.DE_IR );
    printf("DE_V            :  %d\n", PS.DE_V);
    printf("\n");
    
    printf("------------- AGEX Latches --------------\n");
    printf("AGEX_NPC        :  0x%04x\n", PS.AGEX_NPC );
    printf("AGEX_SR1        :  0x%04x\n", PS.AGEX_SR1 );
    printf("AGEX_SR2        :  0x%04x\n", PS.AGEX_SR2 );
    printf("AGEX_CC         :  %d\n", PS.AGEX_CC );
    printf("AGEX_IR         :  0x%04x\n", PS.AGEX_IR );
    printf("AGEX_DRID       :  %d\n", PS.AGEX_DRID);
    printf("AGEX_CS         :  ");
    for ( k = 0 ; k < NUM_AGEX_CS_BITS; k++) {
      printf("%d",PS.AGEX_CS[k]);
    }
    printf("\n");
    printf("AGEX_V          :  %d\n", PS.AGEX_V);
    printf("\n");

    printf("------------- MEM  Latches --------------\n");
    printf("MEM_NPC         :  0x%04x\n", PS.MEM_NPC );
    printf("MEM_ALU_RESULT  :  0x%04x\n", PS.MEM_ALU_RESULT );
    printf("MEM_ADDRESS     :  0x%04x\n", PS.MEM_ADDRESS ); 
    printf("MEM_CC          :  %d\n", PS.MEM_CC );
    printf("MEM_IR          :  0x%04x\n", PS.MEM_IR );
    printf("MEM_DRID        :  %d\n", PS.MEM_DRID);
    printf("MEM_CS          :  ");
    for ( k = 0 ; k < NUM_MEM_CS_BITS; k++) {
      printf("%d",PS.MEM_CS[k]);
    }
    printf("\n");
    printf("MEM_V           :  %d\n", PS.MEM_V);
    printf("\n");

    printf("------------- SR   Latches --------------\n");
    printf("SR_NPC          :  0x%04x\n", PS.SR_NPC );
    printf("SR_DATA         :  0x%04x\n", PS.SR_DATA );
    printf("SR_ALU_RESULT   :  0x%04x\n", PS.SR_ALU_RESULT );
    printf("SR_ADDRESS      :  0x%04x\n", PS.SR_ADDRESS );
    printf("SR_IR           :  0x%04x\n", PS.SR_IR );
    printf("SR_DRID         :  %d\n", PS.SR_DRID);
    printf("SR_CS           :  ");
    for ( k = 0 ; k < NUM_SR_CS_BITS; k++) {
      printf("%d",PS.SR_CS[k]);
    }
    printf("\n");
    printf("SR_V            :  %d\n", PS.SR_V);
    
    printf("\n");

    /* dump the state information into the dumpsim file */
    fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
    fprintf(dumpsim_file,"\nCurrent architectural state :\n");
    fprintf(dumpsim_file,"-------------------------------------\n");
    fprintf(dumpsim_file,"Cycle Count     : %d\n", CYCLE_COUNT);
    fprintf(dumpsim_file,"PC              : 0x%04x\n", PC);
    fprintf(dumpsim_file,"CCs: N = %d  Z = %d  P = %d\n", N, Z, P);
    fprintf(dumpsim_file,"Registers:\n");
    for (k = 0; k < LC3b_REGS; k++)
      fprintf(dumpsim_file,"%d: 0x%04x\n", k, (REGS[k] & 0xFFFF));
    fprintf(dumpsim_file,"\n");
    
    fprintf(dumpsim_file,"------------- Stall Signals -------------\n");
    fprintf(dumpsim_file,"ICACHE_R        :  %d\n", icache_r);
    fprintf(dumpsim_file,"DEP_STALL       :  %d\n", dep_stall);
    fprintf(dumpsim_file,"V_DE_BR_STALL   :  %d\n", v_de_br_stall);
    fprintf(dumpsim_file,"V_AGEX_BR_STALL :  %d\n", v_agex_br_stall);
    fprintf(dumpsim_file,"MEM_STALL       :  %d\n", mem_stall);
    fprintf(dumpsim_file,"V_MEM_BR_STALL  :  %d\n", v_mem_br_stall);
    fprintf(dumpsim_file,"\n");

    fprintf(dumpsim_file,"------------- DE   Latches --------------\n");
    fprintf(dumpsim_file,"DE_NPC          :  0x%04x\n", PS.DE_NPC );
    fprintf(dumpsim_file,"DE_IR           :  0x%04x\n", PS.DE_IR );
    fprintf(dumpsim_file,"DE_V            :  %d\n", PS.DE_V);
    fprintf(dumpsim_file,"\n");
    
    fprintf(dumpsim_file,"------------- AGEX Latches --------------\n");
    fprintf(dumpsim_file,"AGEX_NPC        :  0x%04x\n", PS.AGEX_NPC );
    fprintf(dumpsim_file,"AGEX_SR1        :  0x%04x\n", PS.AGEX_SR1 );
    fprintf(dumpsim_file,"AGEX_SR2        :  0x%04x\n", PS.AGEX_SR2 );
    fprintf(dumpsim_file,"AGEX_CC         :  %d\n", PS.AGEX_CC );
    fprintf(dumpsim_file,"AGEX_IR         :  0x%04x\n", PS.AGEX_IR );
    fprintf(dumpsim_file,"AGEX_DRID       :  %d\n", PS.AGEX_DRID);
    fprintf(dumpsim_file,"AGEX_CS         :  ");
    for ( k = 0 ; k < NUM_AGEX_CS_BITS; k++) {
      fprintf(dumpsim_file,"%d",PS.AGEX_CS[k]);
    }
    fprintf(dumpsim_file,"\n");
    fprintf(dumpsim_file,"AGEX_V          :  %d\n", PS.AGEX_V);
    fprintf(dumpsim_file,"\n");

    fprintf(dumpsim_file,"------------- MEM  Latches --------------\n");
    fprintf(dumpsim_file,"MEM_NPC         :  0x%04x\n", PS.MEM_NPC );
    fprintf(dumpsim_file,"MEM_ALU_RESULT  :  0x%04x\n", PS.MEM_ALU_RESULT );
    fprintf(dumpsim_file,"MEM_ADDRESS     :  0x%04x\n", PS.MEM_ADDRESS ); 
    fprintf(dumpsim_file,"MEM_CC          :  %d\n", PS.MEM_CC );
    fprintf(dumpsim_file,"MEM_IR          :  0x%04x\n", PS.MEM_IR );
    fprintf(dumpsim_file,"MEM_DRID        :  %d\n", PS.MEM_DRID);
    fprintf(dumpsim_file,"MEM_CS          :  ");
    for ( k = 0 ; k < NUM_MEM_CS_BITS; k++) {
      fprintf(dumpsim_file,"%d",PS.MEM_CS[k]);
    }
    fprintf(dumpsim_file,"\n");
    fprintf(dumpsim_file,"MEM_V           :  %d\n", PS.MEM_V);
    fprintf(dumpsim_file,"\n");

    fprintf(dumpsim_file,"------------- SR   Latches --------------\n");
    fprintf(dumpsim_file,"SR_NPC          :  0x%04x\n", PS.SR_NPC );
    fprintf(dumpsim_file,"SR_DATA         :  0x%04x\n",PS.SR_DATA );
    fprintf(dumpsim_file,"SR_ALU_RESULT   :  0x%04x\n", PS.SR_ALU_RESULT );
    fprintf(dumpsim_file,"SR_ADDRESS      :  0x%04x\n", PS.SR_ADDRESS );
    fprintf(dumpsim_file,"SR_IR           :  0x%04x\n", PS.SR_IR );
    fprintf(dumpsim_file,"SR_DRID         :  %d\n", PS.SR_DRID);
    fprintf(dumpsim_file,"SR_CS           :  ");
    for ( k = 0 ; k < NUM_SR_CS_BITS; k++) {
    fprintf(dumpsim_file, "%d",PS.SR_CS[k]);
    }
    fprintf(dumpsim_file,"\n");
    fprintf(dumpsim_file,"SR_V            :  %d\n", PS.SR_V);
    
    fprintf(dumpsim_file,"\n");
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

    switch(buffer[0]) {
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

    case 'I':
    case 'i':
        idump(dumpsim_file);
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
    for(i = 0; i < CONTROL_STORE_ROWS; i++) {
  if (fscanf(ucode, "%[^\n]\n", line) == EOF) {
      printf("Error: Too few lines (%d) in micro-code file: %s\n",
       i, ucode_filename);
      exit(-1);
  }

  /* Put in bits one at a time. */
  index = 0;

  for (j = 0; j < NUM_CONTROL_STORE_BITS; j++) {
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
      CONTROL_STORE[i][j] = (line[index] == '0') ? 0:1;
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
    
     for (i=0; i < WORDS_IN_MEM; i++) {
  MEMORY[i][0] = 0;
  MEMORY[i][1] = 0;
    }
}


/***************************************************************/
/*                                                             */
/* Procedure : init_state                                      */
/*                                                             */
/* Purpose   : Zero out all latches and registers              */
/*                                                             */
/***************************************************************/
void init_state() {
  
  memset(&PS, 0 ,sizeof(PipeState_Entry)); 
  memset(&NEW_PS, 0 , sizeof(PipeState_Entry));
  
  dep_stall       = 0; 
  v_de_br_stall   = 0;
  v_agex_br_stall = 0;
  v_mem_br_stall  = 0;
  mem_stall       = 0;
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
  program_base = word >> 1 ;
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

    if (PC == 0) PC  = program_base << 1 ;
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

    for ( i = 0; i < num_prog_files; i++ ) {
  load_program(program_filename);
  while(*program_filename++ != '\0');
    }
    init_state();
    
    RUN_BIT = TRUE;
}

/***************************************************************/
/*                                                             */
/* dcache_access                                               */
/*                                                             */
/***************************************************************/
void dcache_access(int dcache_addr, int *read_word, int write_word, int *dcache_r, 
        int mem_w0, int mem_w1) {
  
  int addr = dcache_addr >> 1 ; 
  int random = CYCLE_COUNT % 9;

  if (!random) {
    *dcache_r = 0;
    *read_word = 0xfeed ;
  }
  else {
    *dcache_r = 1;
    
    *read_word = (MEMORY[addr][1] << 8) | (MEMORY[addr][0] & 0x00FF);
    if(mem_w0) MEMORY[addr][0] = write_word & 0x00FF;
    if(mem_w1) MEMORY[addr][1] = (write_word & 0xFF00) >> 8;
  }
}
/***************************************************************/
/*                                                             */
/* icache_access                                               */
/*                                                             */
/***************************************************************/
void icache_access(int icache_addr, int *read_word, int *icache_r) {
  
  int addr = icache_addr >> 1 ; 
  int random = CYCLE_COUNT % 13;

  if (!random) {
    *icache_r = 0;
    *read_word = 0xfeed;
  }
  else {
    *icache_r = 1;
    *read_word = MEMORY[addr][1] << 8 | MEMORY[addr][0];
  }
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

    if ( (dumpsim_file = fopen( "dumpsim", "w" )) == NULL ) {
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
   
   RUN_BIT
   REGS
   MEMORY

   PC
   N
   Z
   P

   dep_stall
   v_de_br_stall
   v_agex_br_stall
   v_mem_br_stall
   mem_stall 
   icache_r
 
   PS
   NEW_PS


   You may define your own local/global variables and functions.
   You may use the functions to get at the control bits defined
   above.


   Begin your code here                  */
/***************************************************************/
#define COPY_AGEX_CS_START 3 
#define COPY_MEM_CS_START 9
#define COPY_SR_CS_START  7

#define DEBUG 1

/* Signals generated by SR stage and needed by previous stages in the
pipeline are declared below. */
int sr_reg_data,
sr_n, sr_z, sr_p,
v_sr_ld_cc,
v_sr_ld_reg,
sr_reg_id;

/* MEM stage globals */
int mem_pc_mux = 0;
int v_mem_ld_reg = 0;
int v_mem_ld_cc = 0;

/* AGEX stage globals */
int v_agex_ld_cc = 0;
int v_agex_ld_reg = 0;

int target_pc, trap_pc;

int isBranch = 0; 
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
  return returnVal & 0x0FFFF;
}

/**
* TODO:
*   ctrl+f all "0x" masks and validate
*/

/************************* SR_stage() *************************/
void SR_stage()
{

  /* You are given the code for SR_stage to get you started. Look at
  the figure for SR stage to see how this code is implemented. */

  if (DEBUG)
    printf("Cycle count: %d\n", CYCLE_COUNT); 

  if(DEBUG)
    printf("*************** SR   Stage ***************\n"); 

  switch (Get_DR_VALUEMUX1(PS.SR_CS))
  {
  case 0:
    sr_reg_data = PS.SR_ADDRESS;
    break;
  case 1:
    sr_reg_data = PS.SR_DATA;
    break;
  case 2:
    sr_reg_data = PS.SR_NPC;
    break;
  case 3:
    sr_reg_data = PS.SR_ALU_RESULT;
    break;
  }

  sr_reg_id = PS.SR_DRID;
  v_sr_ld_reg = Get_SR_LD_REG(PS.SR_CS) & PS.SR_V;
  v_sr_ld_cc = Get_SR_LD_CC(PS.SR_CS) & PS.SR_V;

  /* CC LOGIC  */
  sr_n = ((sr_reg_data & 0x8000) ? 1 : 0);
  sr_z = ((sr_reg_data & 0xFFFF) ? 0 : 1);
  sr_p = 0;
  if ((!sr_n) && (!sr_z))
    sr_p = 1;

  if (DEBUG)
  {
    if (PS.SR_V)
    {
      printf("SR CC Data: %d\nSR Reg Data: 0x%x\n", ((sr_n<<2) | (sr_z<<1) | (sr_p)), sr_reg_data); 
      printf("LDREG: %d\nLDCC: %d\n", v_sr_ld_reg, v_sr_ld_cc); 
      printf("DRID: %d\n", PS.SR_DRID); 
    }
    else
      printf("SR stage has a bubble\n"); 
  }
}


/************************* MEM_stage() *************************/

/* Inner Logic Block Function Units */
int dataToDCache()
{
  int data, result;
  data = PS.MEM_ALU_RESULT; /* data from register */ 
  result = 0;

  if ((Get_DATA_SIZE(PS.MEM_CS))&0xFFFF > 0)
  {
    /* Word Data */
    result = (data & 0xFFFF);
    if (DEBUG)
      printf("Writing data to dCache: 0x%x\n", result);
    return result; 
  }
  else
  {
    /* Byte Data */
    if ((PS.MEM_ADDRESS & 0x0001) == 0)
    {
      /* return first byte */
      result = (data & 0x00FF);
      result = SextXtoY(result, 8, 16);
      if (DEBUG)
        printf("Data to write to dCache is byte at even address\nSign extended value = 0x%x\n", result);
    }
    else
    {
      /* return second byte */
      result = (data & 0x00FF);
      result = result << 8;
      if (DEBUG)
        printf("Data to write to dCache is byte at odd address\nSign extended value = 0x%x\n", result);
    }
  }

  return result;
}

int dCacheEnable()
{
  int enable = 0;

  if ((Get_DCACHE_EN(PS.MEM_CS) > 0) &&
    (PS.MEM_V > 0))
  {
    /* Valid access to memory: enable the cache */
    enable = 1;
  }
  else
    enable = 0;

  return enable;
}

int we_logic(int *we0, int *we1)
{
  int we_bits; 

  *we0 = 0; 
  *we1 = 0; 
  we_bits = 0;

  if (Get_DCACHE_RW(PS.MEM_CS) > 0)
  {
    /* write mode */
    if (Get_DATA_SIZE(PS.MEM_CS) > 0)
    {
      /* data to write is a word (16b) */
      we_bits = 0x0003;
      *we0 = 1; 
      *we1 = 1; 
    }
    else
    {
      /* data to write is a byte */
      if ((PS.MEM_ADDRESS & 0x0001) == 0)
      {
        /* even address */
        we_bits = 0x0001;
        *we0 = 1; 
      }
      else
      { /* odd address */
        we_bits = 0x0002;
        *we1 = 1; 
      }
    }
  }
  return we_bits;
}

int br_logic()
{
  int pcMux = 0;
  int irCCs = 0;

  pcMux = 0; 
  irCCs = 0; 
  if (PS.MEM_V > 0)
  {
    irCCs = PS.MEM_IR;
    irCCs = (irCCs & 0x0E00) >> 9;

    if (DEBUG) printf("IR CCs: 0x%x\n", irCCs); 

    pcMux = 0; 
    if (Get_BR_OP(PS.MEM_CS) > 0)
    {
      if (DEBUG) printf(" checking for branch********************************************\n"); 
      /* this is a branch isntruction */
      if (irCCs & 1) /* p */
        if (PS.MEM_CC & 1)
          pcMux = 1;
      if (irCCs & 2)
        if (PS.MEM_CC & 2)
          pcMux = 1;
      if (irCCs & 4)
        if (PS.MEM_CC & 4)
          pcMux = 1; 
    }
     if (pcMux)
        if (DEBUG) printf("Taking Branch\n");
    
      if (Get_UNCOND_OP(PS.MEM_CS))
        pcMux = 1;
      
        if (Get_TRAP_OP(PS.MEM_CS))
          pcMux = 2;
  }


  return pcMux;
}

void MEM_stage()
{

  int ii, jj = 0;

  /* your code for MEM stage goes here */
  int we_bits, dCacheEN;
  int mResult, dcache_ready;
  int lwe0, lwe1; 

  if (DEBUG)
    printf("*************** MEM  STAGE ***************\n");

  /* mememory PCMUX select line signals */
  mem_pc_mux = br_logic();
  if (DEBUG) printf("BR_LOGIC: 0x%x\n", mem_pc_mux); 

  we_bits = we_logic(&lwe0, &lwe1);
  dCacheEN = dCacheEnable();
  if (DEBUG)
    if (dCacheEN)
      printf("Accessing Memory...\n");

  /* dcache access */
  if (dCacheEN)
  {
    dcache_access(PS.MEM_ADDRESS,
      &mResult,
      dataToDCache(),
      &dcache_ready,
      lwe0,
      lwe1);
    if (DEBUG) printf("WE0: %x\nWE1: %x\n", lwe0, lwe1);
    if (DEBUG) printf("Value to write: 0x%x\n", dataToDCache());  
  }
  /* MEM_STALL */
  if ((dcache_ready == 0) && (dCacheEN > 0))
    mem_stall = 1;
  else
    mem_stall = 0;

  /* TARGET_PC */
  target_pc = PS.MEM_ADDRESS;

  /* MEM RESULT */
  if (Get_DATA_SIZE(PS.MEM_CS) > 0)
  {
    /* word size */
    mResult = mResult & 0xFFFF;
  }
  else
  {
    /* byte size */
    if ((PS.MEM_ADDRESS & 0x0001) == 0)
    {
      /* even address */
      mResult = (mResult & 0x00FF);
      mResult = SextXtoY(mResult, 8, 16);
    }
    else
    {
      /* odd address */
      mResult = (mResult & 0xFF00) >> 8;
      mResult = SextXtoY(mResult, 8, 16);
    }
  }

  /* trap_pc */
  trap_pc = mResult;

  /* mem latch logic */
  if (PS.MEM_V > 0)
  {
    /* If valid, output CC, REGLOAD, and BRSTALL signals */
    v_mem_ld_cc = Get_MEM_LD_CC(PS.MEM_CS);
    v_mem_ld_reg = Get_MEM_LD_REG(PS.MEM_CS);
    v_mem_br_stall = Get_MEM_BR_STALL(PS.MEM_CS);
  }
  else
  {
    v_mem_ld_cc = 0;
    v_mem_ld_reg = 0;
    v_mem_br_stall = 0;
  }


  

  /* The code below propagates the control signals from MEM.CS latch
  to SR.CS latch. You still need to latch other values into the
  other SR latches. */
  for (ii = COPY_SR_CS_START; ii < NUM_MEM_CS_BITS; ii++) {
    NEW_PS.SR_CS[jj++] = PS.MEM_CS[ii];
  }

  /* latch in the rest of SR values */
  NEW_PS.SR_ADDRESS = (PS.MEM_ADDRESS & 0xFFFF);
  NEW_PS.SR_DATA = (mResult & 0xFFFF);
  NEW_PS.SR_NPC = (PS.MEM_NPC & 0xFFFF);
  NEW_PS.SR_ALU_RESULT = (PS.MEM_ALU_RESULT & 0xFFFF);
  NEW_PS.SR_IR = (PS.MEM_IR & 0xFFFF);
  NEW_PS.SR_DRID = (PS.MEM_DRID & 0xFFFF);

  /* TODO enything else? */
  if (mem_stall > 0) /* waiting on dcache */
    NEW_PS.SR_V = 0;
  else
    if (PS.MEM_V)
      NEW_PS.SR_V = 1;
    else
      NEW_PS.SR_V = 0;
  
  if (DEBUG)
  {
    if (PS.MEM_V)
    {
      if (mem_stall)
        printf("Memory stalling"); 
      else
        printf("target_pc: 0x%x\ntrap_pc: 0x%x\nmem_pcmux: %d\n", target_pc, trap_pc, mem_pc_mux); 
    }
    else 
      printf("MEM has a bubble\n"); 
  }

}


/************************* AGEX_stage() *************************/

int getAddress()
{
  int addr1MUXOut, addr2MUXOut, addrOut, addressOut;
  addr1MUXOut = 0;
  addr2MUXOut = 0; 
  addrOut = 0; 
  addressOut = 0;  

  /* ADDR1MUX output */
  if (Get_ADDR1MUX(PS.AGEX_CS) > 0)
    addr1MUXOut = PS.AGEX_SR1;
  else
    addr1MUXOut = PS.AGEX_NPC;

  if (DEBUG && PS.AGEX_V) printf("Addr1Mux output: 0x%x\n", addr1MUXOut); 

  /* ADDR2MUX output */
  switch (Get_ADDR2MUX(PS.AGEX_CS))
  {
  case 0:
    addr2MUXOut = 0;
    break;
  case 1:
    addr2MUXOut = SextXtoY((PS.AGEX_IR & 0x003F), 6, 16);
    break;
  case 2:
    addr2MUXOut = SextXtoY((PS.AGEX_IR & 0x01FF), 9, 16);
    break;
  case 3:
    addr2MUXOut = SextXtoY((PS.AGEX_IR & 0x07FF), 11, 16);
    break;
  }
  if (Get_LSHF1(PS.AGEX_CS) > 0)
    addr2MUXOut *= 2;
  if (DEBUG && PS.AGEX_V) printf("Addr2Mux output: 0x%x\n", addr2MUXOut); 

  /* address adder output */
  addrOut = addr1MUXOut + addr2MUXOut;

  /* ADDRESSMUX output */
  if (Get_ADDRESSMUX(PS.AGEX_CS) > 0)
    addressOut = addrOut;
  else
    addressOut = ((PS.AGEX_IR & 0x00FF) << 1);

  if (DEBUG && PS.AGEX_V) printf("Address output: 0x%d\n", (addressOut & 0xFFFF)); 
  return (addressOut & 0xFFFF);
}

int getALUMUXOutput()
{
  int ir5bits;
  int laluA, laluB, lshf;
  int lALUOutput, lSHFOutput, lALUResult;

  laluA = PS.AGEX_SR1;
  laluB = (Get_SR2MUX(PS.AGEX_CS) > 0) ? (SextXtoY((PS.AGEX_IR & 0x001F), 5, 16)) : PS.AGEX_SR2;
  lshf = laluA; 
  /* ALU output */
  switch (Get_ALUK(PS.AGEX_CS))
  {
  case 0:
    lALUOutput = ((laluA + laluB) & 0xFFFF);
    if (DEBUG) printf("Operation is ADD\n"); 
    break;
  case 1:
    lALUOutput = (laluA & laluB);
    if (DEBUG) printf("Operation is AND\n"); 
    break;
  case 2:
    lALUOutput = (laluA ^ laluB);
    if (DEBUG) printf("Operation is XOR\n"); 
    break;
  case 3:
    lALUOutput = laluB;
    if (DEBUG) printf("Operations is PASSB\n"); 
    break;
  }
  if (DEBUG) printf("ALUK result is: 0x%x\n", lALUOutput);

  /* shf output */
  ir5bits = PS.AGEX_IR & 0x003F;
  switch (ir5bits & 0x30)
  {
  case 0x00: /* LSH */
    lshf = (lshf << (ir5bits & 0x0F));
    if (DEBUG) printf("LSHF output: 0x%x\n", lshf);
    break;
  case 0x010: /* RSHFL */
    printf("**********************************************************************\n"); 
    lshf = (lshf >> (ir5bits & 0x0F));
    if (DEBUG) printf("RSHFL output: 0x%x\n", lshf); 
    break;
  case 0x030: /* RSHFA */
    lshf = (lshf >> (ir5bits & 0x0F));
    lshf = SextXtoY(lshf,
      (16 - (ir5bits & 0x0F)),
      16);
    if (DEBUG) printf("RSHFA output: 0x%x\n", lshf); 
    break;
  }
  

  /* ALU_RESULTMUX Output */
  lALUResult = ( ((Get_ALU_RESULTMUX(PS.AGEX_CS) > 0) ? lALUOutput : lshf) & 0xFFFF);
  if (DEBUG) printf("ALU result mux output: 0x%x\n", lALUResult);
  return lALUResult;
}

void AGEX_stage()
{

  int ii, jj = 0;
  int LD_MEM; /* You need to write code to compute the value of LD.MEM
        signal */

  /* your code for AGEX stage goes here */
  int lmemAddress;
  int lALUResult;

  if (DEBUG)
    printf("*************** AGEX STAGE ***************\n");
  lmemAddress = getAddress();
  lALUResult = getALUMUXOutput();

  if (PS.AGEX_V > 0)
  {
    v_agex_ld_cc = Get_AGEX_LD_CC(PS.AGEX_CS);
    v_agex_ld_reg = Get_AGEX_LD_REG(PS.AGEX_CS);
    v_agex_br_stall = Get_AGEX_BR_STALL(PS.AGEX_CS);
  }
  else
  {
    v_agex_ld_cc = 0; 
    v_agex_ld_reg = 0; 
    v_agex_br_stall = 0; 
  }

  if (mem_stall)
    LD_MEM = 0;
  else
    LD_MEM = 1;

  if (LD_MEM) {
    /* Your code for latching into MEM latches goes here */
    NEW_PS.MEM_ADDRESS = (lmemAddress & 0xFFFF);
    NEW_PS.MEM_NPC = (PS.AGEX_NPC & 0xFFFF);
    NEW_PS.MEM_CC = (PS.AGEX_CC & 0xFFFF);
    NEW_PS.MEM_ALU_RESULT = (lALUResult & 0xFFFF);
    NEW_PS.MEM_IR = (PS.AGEX_IR & 0xFFFF);
    NEW_PS.MEM_DRID = (PS.AGEX_DRID & 0xFFFF);

    /* TODO: when to bubble into MEM.V */
    NEW_PS.MEM_V = PS.AGEX_V;
    /* br stall? */

    /* The code below propagates the control signals from AGEX.CS latch
    to MEM.CS latch. */
    for (ii = COPY_MEM_CS_START; ii < NUM_AGEX_CS_BITS; ii++) {
      NEW_PS.MEM_CS[jj++] = PS.AGEX_CS[ii];
    }

    if (DEBUG)
    {
      if (PS.AGEX_V)
      {
        printf("DRID: 0x%d\nAddress MUX: 0x%x\nALUK out: 0x%x\n", PS.AGEX_DRID, lmemAddress, lALUResult); 
      }
      else
        printf("AGEX has a bubble\n"); 
    }
  }
  else 
    if (DEBUG) printf("Not loading mem\n");
}



/************************* DE_stage() *************************/

void ldDepStall(int csaddr)
{
  int lsr1, lsr2;
  lsr1 = ((PS.DE_IR & 0x01C0) >> 6);
  if ((PS.DE_IR & 0x8000) > 0)
    lsr2 = ((PS.DE_IR & 0x0E00) >> 9);
  else
    lsr2 = (PS.DE_IR & 0x0007);
  int tempstall = 1; 

  dep_stall = 0; 

  if (PS.DE_V)
  {
    /* sr1 dependency */
    if (Get_SR1_NEEDED(CONTROL_STORE[csaddr]))
    {
      if ( (v_agex_ld_reg) && (lsr1 == PS.AGEX_DRID) )
      {
        dep_stall = 1;
        if (DEBUG) printf("dep_stall asserted: AGEX has SR1: %d\n", lsr1); 
      }
      else
        if ( (v_mem_ld_reg) &&  (lsr1 == PS.MEM_DRID) )
        {
          dep_stall = 1;
        if (DEBUG) printf("dep_stall asserted: MEM has SR1: %d\n", lsr1);           
        }
        else
          if ( (v_sr_ld_reg) && (lsr1 == PS.SR_DRID) )
          {
            dep_stall = 0;
            if (DEBUG) printf("dep_stall grounded: SR supplying SR1: %d\n", lsr1); 
          }
    }
    if (dep_stall)
      return; 

    /* sr2 dependency */
    if (Get_SR2_NEEDED(CONTROL_STORE[csaddr]))
    {
      if ( (v_agex_ld_reg) && (lsr2 == PS.AGEX_DRID) )
      {
        dep_stall = 1;
        if (DEBUG) printf("dep_stall asserted: AGEX has SR2: %d\n", lsr2); 
      }
      else
        if ( (v_mem_ld_reg) &&  (lsr2 == PS.MEM_DRID) )
        {
          dep_stall = 1;
        if (DEBUG) printf("dep_stall asserted: MEM has SR2: %d\n", lsr2);           
        }
        else
          if ( (v_sr_ld_reg) && (lsr2 == PS.SR_DRID) )
          {
            dep_stall = 0;
            if (DEBUG) printf("dep_stall grounded: SR supplying SR2: %d\n", lsr2); 
          }
    }
    if (dep_stall)
      return; 
    
    /* CC dependency */
    if (Get_DE_BR_OP(CONTROL_STORE[csaddr]))
    {
      if (v_agex_ld_cc)
        dep_stall = 1;
      else
        if (v_mem_ld_cc)
          dep_stall = 1;
        else
          if (v_sr_ld_cc)
          dep_stall = 0;
    }
    

    if (dep_stall)
      return; 
  }
  else
    dep_stall = 0;
}



void DE_stage()
{

  int CONTROL_STORE_ADDRESS;  /* You need to implement the logic to
                set the value of this variable. Look
                at the figure for DE stage */
  int ii, jj = 0;
  int LD_AGEX; /* You need to write code to compute the value of
         LD.AGEX signal */

  /* your code for DE stage goes here */
  int lsr1, lsr2, lcc, lsr1d, lsr2d;

  if (DEBUG)
    printf("*************** DE   STAGE ***************\n");

  /* control store logic */
  CONTROL_STORE_ADDRESS = ((PS.DE_IR >> 10) & 0x003e);
  if (PS.DE_IR & 0x0020)
    CONTROL_STORE_ADDRESS |= 0x0001;

  if (DEBUG && PS.DE_V)
    printf("Control store address: 0x%x\n", CONTROL_STORE_ADDRESS); 

  /* dependency check logic */
  ldDepStall(CONTROL_STORE_ADDRESS);

  /* V_DE_BR_STALL logic */
  if ((PS.DE_V > 0) && (Get_DE_BR_STALL(CONTROL_STORE[CONTROL_STORE_ADDRESS]) > 0))
    v_de_br_stall = 1;
  else
    v_de_br_stall = 0;

  
  /* store result values */ 
  if (v_sr_ld_reg > 0)
  {
    REGS[PS.SR_DRID] = sr_reg_data;
    if (DEBUG) printf("DR %d was written 0x%x\n", PS.SR_DRID, sr_reg_data); 
  }
  if (v_sr_ld_cc > 0)
  {
    N = sr_n;
    Z = sr_z;
    P = sr_p;
    if (DEBUG) printf("CCs were modified: N:%d Z:%d P:%d\n", N, Z, P); 
  }


  /* access register file and CCs*/
  lsr1 = ((PS.DE_IR & 0x01C0) >> 6);
  if ((PS.DE_IR & 0x2000) > 0)
    lsr2 = ((PS.DE_IR & 0x0E00) >> 9);
  else
    lsr2 = (PS.DE_IR & 0x0007);
  lsr1d = REGS[lsr1];
  lsr2d = REGS[lsr2];
  /* N, Z, P are already loaded in their globals */

  /* LD_AGEX logic */
  LD_AGEX = 1;
  /*if (v_agex_br_stall)
    LD_AGEX = 0;
  */
  if (mem_stall)
    LD_AGEX = 0;




  if (LD_AGEX)
  {
    /* Your code for latching into AGEX latches goes here */
    NEW_PS.AGEX_NPC = PS.DE_NPC;
    NEW_PS.AGEX_IR = PS.DE_IR;
    NEW_PS.AGEX_SR1 = (lsr1d & 0xFFFF);
    NEW_PS.AGEX_SR2 = (lsr2d & 0xFFFF);
    NEW_PS.AGEX_CC = ((N << 2) | (Z << 1) | (P));
    if (Get_DRMUX(CONTROL_STORE[CONTROL_STORE_ADDRESS]) > 0)
      NEW_PS.AGEX_DRID = 7;
    else
      NEW_PS.AGEX_DRID = ((PS.DE_IR & 0x0E00) >> 9);

    /* valid bit */
    if (dep_stall) /* invalid on stall */
    {
      NEW_PS.AGEX_V = 0;
      if (DEBUG) printf("Loading bubble: dep_stall\n"); 
    }
    else  /* otherwise, respect previous stage's validity */
      NEW_PS.AGEX_V = PS.DE_V;

    if (DEBUG)
    {
      if (PS.DE_V)
      {
        printf("SR1: %d\nSR2: %d\nCCs: 0x%x\nDRID: %d\n", lsr1, lsr2, ((N<<2)|(Z<<1)|(P)), NEW_PS.AGEX_DRID); 
      }
      else
        printf("DE has a bubble\n"); 
    }
    else
      if (DEBUG) printf("Not loading AGEX\n"); 


    /* The code below propagates the control signals from the CONTROL
    STORE to the AGEX.CS latch. */
    for (ii = COPY_AGEX_CS_START; ii< NUM_CONTROL_STORE_BITS; ii++) {
      NEW_PS.AGEX_CS[jj++] = CONTROL_STORE[CONTROL_STORE_ADDRESS][ii];
    }
  }
}



/************************* FETCH_stage() *************************/

int getPCMUX()
{
  int returnVal = 0;
  switch (mem_pc_mux)
  {
  case 0:
    returnVal = PC + 2;
    if (DEBUG) printf("MUX <= Incremented PC\n"); 
    break;
  case 1:
    returnVal = target_pc;
    if (DEBUG) printf("MUX <= Target PC\n"); 
    break;
  case 2:
    returnVal = trap_pc;
    if (DEBUG) printf("MUX <= Trap PC\n"); 
    break;
  }

  return returnVal;
}

void FETCH_stage()
{

  /* your code for FETCH stage goes here */
  int toPC, incPC, iCacheResult, iCacheReady;
  int LD_DE, LD_PC, bubble;

  if (DEBUG)
    printf("*************** F    Stage ***************\n");
  incPC = PC + 2;
  toPC = getPCMUX();
  icache_access(PC, &iCacheResult, &icache_r);

  /* LD_PC logic */
  LD_PC = 1;
  /* 0 when stalling, but not v_mem_br_stall */
  if (dep_stall)
  {
    LD_PC = 0; 
    if(DEBUG) printf("PC not loaded: dep_stall \n"); 
  }       
  if (v_de_br_stall)   
  {
    LD_PC = 0; 
    if(DEBUG) printf("PC not loaded: v_de_br_stall\n"); 
  }
  if (v_agex_br_stall) 
  {
    LD_PC = 0; 
    if(DEBUG) printf("PC not loaded: v_agex_br_stall\n"); 
  }
  if (v_mem_br_stall)
  {
    LD_PC = 0; 
    if (DEBUG) printf("PC not loaded: v_mem_br_stall\n"); 
  }
  if (mem_stall)       
  {
    LD_PC = 0; 
    if(DEBUG) printf("PC not loaded: mem_stall\n"); 
  }
  if ((icache_r == 0)) 
  {
    /* if (mem_pc_mux == 0)
    
    { */ 
      LD_PC = 0; 
      if(DEBUG) printf("PC not loaded: icache_r == 0\n"); 
    /*}*/ 
  }
  
  /*
  if ((mem_pc_mux == 0) && (v_mem_br_stall))
    LD_PC = 0; 
  */


    /* bubble logic */
    bubble = 1;
    if (v_de_br_stall ||
      v_agex_br_stall ||
      v_mem_br_stall  ||
      (icache_r == 0) )
      bubble = 0;

  /* LD_DE logic */
  LD_DE = 1;
  if (mem_stall)
    LD_DE = 0;
  if (dep_stall)
    LD_DE = 0;

  if (LD_PC)
  {
    PC = toPC;
    if (DEBUG)
      printf("PC written: 0x%x\n", PC); 
  }
  else
    printf("PC: 0x%x\n", PC); 
  
  if (LD_DE)
  {
    /* Load DE stage latches */
    NEW_PS.DE_NPC = (incPC & 0xFFFF);
    NEW_PS.DE_IR = (iCacheResult & 0xFFFF);
    NEW_PS.DE_V = bubble;
  }
  else
    if (DEBUG) printf("Not loading DE stage\n"); 
  if (mem_pc_mux == 1)
  {
    PC = target_pc; 
    if (DEBUG) printf("PC <= target pc\n"); 
  }
  if (mem_pc_mux == 2)
  {
    PC = trap_pc; 
    if (DEBUG) printf("PC <= trap pc\n"); 
  }
  if (DEBUG) printf("*******************************************************\n\n"); 
}


