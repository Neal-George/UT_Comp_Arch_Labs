; User program
; initialize x4000 to 1
; calculate sum of first SUM*2 bytes from xC000 
; store sum at 0xC014
	.ORIG x3000
	; R6 <- USP
	LEA R6, USP
	LDW R6, R6, #0
	; initialize x4000 to 1
	AND R4, R4, #0
	ADD R1, R1, #1
	LEA R2, SLOC
	LDW R2, R2, #0
	STW R1, R2, #0
	; calculate sum of first 20 bytes from xC000
	AND R4, R4, #0
	LEA R2, SUM
	LDW R3, R2, #0
	LEA R2, LIST
	LDW R2, R2, #0
AGN	LDB R1, R2, #0
	ADD R4, R4, R1
	LDB R1, R2, #1
	ADD R4, R4, R1
	ADD R2, R2, #2
	ADD R3, R3, #-1
	BRp AGN
	LEA R2, DEST
	LDW R2, R2, #0
	STW R4, R2, #0
	HALT
SLOC	.FILL x4000
LIST	.FILL xC000
SUM 	.FILL x000A
DEST	.FILL xC014
USP  .FILL xFFE0
	.END