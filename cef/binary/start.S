	.set noreorder

.global _start
.ent    _start
_start:
	move       $a0, $sp
	move       $a1, $ra
	la         $sp, 0x00014000
	jal        _main
	nop
	move       $ra, $v0
	jr         $ra
	move       $sp, $v1
.end _start