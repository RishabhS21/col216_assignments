.data
found_msg: .asciiz "Yes at index "
not_found_msg: .asciiz "Not found"
.text
main:
	li $v0,5	#taking first input i.e number of elements in array "n"
	syscall		
	move $t1,$v0	#moving it into register $t1, so $t1="n"
	
	sll $t9,$t1,2    	#multiplying the number of elements by 4 to get the space for array
   	move $a0,$t9        	#allocating space for the array
	li $v0, 9          	#allocating command for space in heap
    	syscall
    	
    	move $a2,$v0		#moving header into $a2..
	li $t0,0		#initiating the loop variable with 0.
	move $t2,$a2				
loop1:
	beq $t0,$t1,search_literals	#loop exit condition.
	li $v0,5		#taking input of the element.
	syscall
	move $t3,$v0		
	li $v0,9
	sw $t3,($t2)		#storing each element at its address which is $t2..
	addi $t0,$t0,1		#increasing the loop variable by 1.
	addi $t2,$t2,4		#increasing the address by 4 for next element..
	j loop1
search_literals:
	li $v0,5		#taking the element to match as a input..
	syscall
	move $t8,$v0		
#	subi $sp,$sp,12		#allocating registers a space in stack.
#	move $s0,$a2		#moving registers to save in stack.
#	move $s1,$t1	
#	sw $ra,0($sp)
#	sw $s0,4($sp)
#	sw $s1,8($sp)
	li $t4,0		#the low index set as 0.. i.e $t4--> low
	subi $t5,$t1,1		#the high is set as n-1
binary_search:
	bgt $t4,$t5,not_found	#when low is greater than high binary search stop with not found.
	
	#calcualting middle index
	add $t6,$t4,$t5		
	div $t6,$t6,2
	mflo $t6
	
	#calculating the middle index element..
	mul $t7,$t6,4
	add $t7,$t7,$a2
	lw $s3,($t7)
	
	beq $s3,$t8,found	#if middle element equal to the required element then branch to found
	
	
	blt $s3,$t8,right	#if middle element is less than riquired then branch to right.
	bgt $s3,$t8,left	#if middle element is less than required elemet branch to left.
right:	
	addi $t4,$t6,1		#for right make the low to be middle index+1..
	j binary_search
left:
	subi $t5,$t6,1		#for left traversing make high as middle index-1.
	j binary_search
found:				#this branch display message Yes at index_
	li $v0,4
	la $a0,found_msg	
	syscall
	move $a0,$t6
	li $v0,1
	syscall
	li $v0,10
	syscall
not_found:			#This branch display message Not found.
	la $a0,not_found_msg
	li $v0,4
	syscall
	li $v0,10
	syscall
