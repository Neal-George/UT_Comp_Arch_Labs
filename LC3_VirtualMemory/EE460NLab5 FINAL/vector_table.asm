; interrupt and exception vector table
.ORIG x0200
	.FILL x0000 ; x0)  nothing for now
	.FILL x1200 ; x1) address of timer handler
	.FILL x1400 ; x2) address of page fault handler
.FILL x1A00 ; x3) address of unaligned handler
	.FILL x1600 ; x4) address of protection handler
	.FILL x1C00 ; x5) address of unknown op handler
.END