	.set noreorder
	.set noat

#define zr 0

	.global setjmp_clone
	.global longjmp_clone

	.ent setjmp_clone
setjmp_clone:
	sw         $ra, 0($a0)
	sw         $gp, 44($a0)
	sw         $sp, 4($a0)
	sw         $fp, 8($a0)
	sw         $s0, 12($a0)
	sw         $s1, 16($a0)
	sw         $s2, 20($a0)
	sw         $s3, 24($a0)
	sw         $s4, 28($a0)
	sw         $s5, 32($a0)
	sw         $s6, 36($a0)
	sw         $s7, 40($a0)
	jr         $ra
	move       $v0, $zr
	.end setjmp_clone

	.ent longjmp_clone
longjmp_clone:
	lw         $ra, 0($a0)
	lw         $gp, 44($a0)
	lw         $sp, 4($a0)
	lw         $fp, 8($a0)
	lw         $s0, 12($a0)
	lw         $s1, 16($a0)
	lw         $s2, 20($a0)
	lw         $s3, 24($a0)
	lw         $s4, 28($a0)
	lw         $s5, 32($a0)
	lw         $s6, 36($a0)
	lw         $s7, 40($a0)
	jr         $ra
	move       $v0, $a1
	.end longjmp_clone