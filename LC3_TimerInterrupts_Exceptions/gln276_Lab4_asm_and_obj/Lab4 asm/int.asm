; interrupt service routine
; increment x4000 by 1 (word access) 
; to think about: save registers in routine? or save in uArch?
.ORIG x1200
	; Push r1 and r2 onto the supervisor stack
	ADD R6, R6, #-2
	STW R1, R6, #0
	ADD R6, R6, #-2
	STW R2, R6, #0
	; Increment word at x4000 by 1
	LEA R2, SLC
	LDW R2, R2, #0
	LDW R1, R2, #0
	ADD R1, R1, #1
	STW R1, R2, #0
	; Restore used registers (pop off the supervisor stack)
	LDW R2, R6, #0
	ADD R6, R6, #2
	LDW R1, R6, #0
	ADD R6, R6, #2
	RTI
SLC	.FILL x4000 
.END