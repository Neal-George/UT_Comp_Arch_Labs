     .ORIG x3000
     AND R0,R0,#0; 00
     AND R1,R1,#0
     ADD R2,R2,#5
     ADD R7,R7,R2
     LEA R6,STORE2
     STW R7,R6,#0
     ADD R3,R3,#-1
     STB R3,R6,#0
     AND R2,R2,R0 ; 10
     AND R3,R3,R2
     AND R6,R6,R3
     AND R7,R7,R6
     NOT R1,R1
     LEA R5,STORE1
     LDW R5,R5,#0
     JSR NEG
     ADD R5,R5,R1 ; 20
     BRz NEXT
     HALT
NEG  NOT R5,R5
     ADD R5,R5,#1
     RET
NEXT LEA R2,STORE3
     LDB R5,R2,#0
     LDW R2,R2,#0 ; 30
     LSHF R2,R2,#8
     RSHFL R2,R2,#8
     JSR NEG
     ADD R0,R5,R2
     BRz NXT
     HALT
NXT  LEA R2,STORE1
     LDB R5,R2,#1 ; 40
     LSHF R5,R5,#8
     RSHFA R5,R5,#8
     AND R0,R0,#0
     ADD R0,R0,#1
     ADD R0,R5,R0
     BRz NXET
     HALT
NXET LEA R2,STORE4 ; 50
     LDB R5,R2,#1
     LEA R2,STORE5
     LDW R6,R2,#0
     JSR NEG
     ADD R5,R5,R6
     BRz L0
     HALT
L0   AND R0,R0,#0 ; 60
     AND R1,R1,R0
     AND R2,R2,R1
     AND R3,R3,R2
     AND R4,R4,R3
     AND R5,R5,R4
     AND R6,R6,R5
     AND R7,R7,R6
     ADD R0,R0,#1 ; 70
     BRp L1
     HALT
L1   ADD R1,R1,#-1
     BRn L2
     HALT
L2   ADD R2,R2,x1
     BRzp L3
     HALT			; 80
L3   ADD R2,R2,x-1
     BRzp L4
     HALT
L4   ADD R3,R3,#-1
     BRnz L5
     HALT
L5   ADD R3,R3,x1
     BRnz L6		; 90
     HALT 
L6   ADD R4,R4,#1
     BRnp L7
     HALT
L7   ADD R4,R4,#-2
     BRnp L8
     HALT
L8   LEA R5,NEG  ; A0
     JSRR R5
     LEA R6,NEG
     ADD R5,R6,R5
     BRz L9
     HALT
L9   LEA R3,STORE3
     AND R4,R4,#0
     ADD R4,R4,#5 ; B0
     XOR R0,R3,R4
     XOR R5,R0,#5
     JSR NEG
     ADD R0,R3,R5
     ADD R0, R0, #-3
     BRz L10
     HALT
L10  LEA R7,STORE4
     LDW R7,R7,#0  ; C0
     AND R0, R0, #0
     AND R1, R1, R0
     ADD R0, R0, #7
sub  ADD R1, R1, #1
     ADD R0, R0, #-1
     BRp sub
     TRAP x25      ; CE
STORE1 .FILL xFFFF
STORE2 .FILL x0000
STORE3 .FILL x1234
STORE4 .FILL x4321
STORE5 .FILL x0043
     .END