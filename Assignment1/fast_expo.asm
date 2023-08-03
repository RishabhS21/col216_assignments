.data
ans: .word 1

.text

main:
	#taking input number "x" as $a0
	li $v0,5
	syscall
	move $a0,$v0
	
	#taking input number "n" as $a1
	li $v0,5
	syscall
	move $a1,$v0
	
	#passing the $a0 and $a1 to function expansion
	jal expansion
	
	#taking the final return output $vo to $a0 and then printing it..
	move $a0,$v0
	li $v0,1
	syscall
	li $v0,10
	syscall
	
expansion:
	#allocating stack space for return adresss..
	subu $sp,$sp,4
	sw $ra,0($sp)
	
	#base case n==0..
	li $v0,1 	#returning 1.
	beq $a1,0,done		#branching to done on n=0.
	
	#taking out the remainder on dividing b 2..
	move $t1,$a1
	andi $t0,$t1,1
	
	#checking it to be odd if remainder is not zero..
	bne $t0,$zero,odd 	#branching it to odd..
	
	#making $a1 = n///2..
	srl $a1,$t1,1
	jal expansion	#passing the input x annd n//2
	
	#here the returning value is squared so that x^n=x^(n//2)*x^(n//2) if n is even..
	mul $v0,$v0,$v0	
	lw $ra,0($sp)
	addu $sp,$sp,4		#exiting the recursion by using return addresses..
	jr $ra
	odd:
		subi $a1,$a1,1		#if n is odd then we calculate expansion with (n-1) and multiply a x in it..
		jal expansion
		mul $v0,$v0,$a0
		lw $ra,0($sp)
		addu $sp,$sp,4
		jr $ra
	done:
		lw $ra,0($sp)		#exiting the recursion..
		addu $sp,$sp,4
		jr $ra
