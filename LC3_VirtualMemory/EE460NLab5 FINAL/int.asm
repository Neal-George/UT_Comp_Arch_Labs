; interrupt service routine
; set all reference bits of PTEs P-Mem to 1 
.ORIG x1200
	; Push r1, r2, and r3 onto the supervisor stack
	ADD R6, R6, #-2
	STW R1, R6, #0
	ADD R6, R6, #-2
	STW R2, R6, #0
	ADD R6, R6, #-2
	STW R3, R6, #0
	; load r3 with address of first PTE
	LEA R3, PTLC
	LDW R3, R3, #0
	; r3 has address of first PTE
	; load r2 with counter (32)
	AND R2, R2, #0
	ADD R2, R2, #8
	ADD R2, R2, R2
	ADD R2, R2, R2
	; r2 has 32
AGAIN	LDW R1, R3, #0
	AND R1, R1, #-2
	STW R1, R3, #0
	ADD R3, R3, #2
	ADD R2, R2, #-1
	BRp AGAIN
	; Restore used registers (pop off the supervisor stack)
	LDW R3, R6, #0
	ADD R6, R6, #2
	LDW R2, R6, #0
	ADD R6, R6, #2
	LDW R1, R6, #0
	ADD R6, R6, #2
	RTI
PTLC	.FILL x1000 
.END

