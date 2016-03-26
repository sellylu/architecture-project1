		addi	$sp, $sp, -4
		sw		$ra, 0($sp)
		addi	$s0, $s0, 0x80		# store a in $1
		lh		$s1, 0($0)		# read b in $2
		add		$0, $s0, $0
		add		$a1, $s1, $0
		slt		$s2, $s0, $s1		
		bne		$s2, $0, first	# if a >= b
		sub		$a1, $s1, $s0		# a = b-a
		lb		$a0, 2($0)		# 0100
		jal		func
		lw		$ra, 0($sp)
		addi	$sp, $sp, 4
		j		end
first:	sll		$t0, $a1, 16
		addu	$v0, $t0, $t0
		j		end
func:	lw		$t0, 4($0)		# 0xfffd	# 68 = 0x11
		lw		$t1, 8($0)		# 0xffff
		xor		$s3, $t0, $t1	# $4 = 2
		addi	$sp, $sp, -8
		sw		$ra, 4($sp)
		sw		$a0, 0($sp)
		bne		$a0, $s3, recur
		srl		$v0, $a1, 2
		addi	$sp, $sp, 8
		jr		$ra
recur:	addi	$a0, $a0, -1
		jal		func
		lw		$a0, 0($sp)
		lw		$ra, 4($sp)
		addi	$sp, $sp, 8
		jr		$ra
end:	lw      $0, 0x802($0)
		sw      $v1, 8($0)
		halt
