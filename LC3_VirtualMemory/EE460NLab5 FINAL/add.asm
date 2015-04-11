; User program
; calculate sum of first SUM*2 bytes from xC000 
; store sum at 0xC014
; jump to address pointed to by SUM
	.ORIG x3000
	; R6 <- USP
	LEA R6, USP		; x3000
	LDW R6, R6, #0
	AND R4, R4, #0
	; calculate sum of first 20 bytes from xC000
	AND R4, R4, #0
	LEA R2, sum 	
	LDW R3, R2, #0
	LEA R2, LIST
	LDW R2, R2, #0
AGN	LDB R1, R2, #0 	; x3010
	ADD R4, R4, R1
	LDB R1, R2, #1
	ADD R4, R4, R1
	ADD R2, R2, #2 
	ADD R3, R3, #-1
	BRp AGN
	LEA R2, DEST
	LDW R2, R2, #0 	; 0x3020
	STW R4, R2, #0			 
	JMP R4
	HALT
SLOC	.FILL x4000
LIST	.FILL xC000
SUM 	.FILL x000A
DEST	.FILL xC014
USP  .FILL xFFE0
	.END

