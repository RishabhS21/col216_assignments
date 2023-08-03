.data
str: .space 100  #specify the input variable with size 100.

.text
.globl main
main:	#taking the input string to read
	la $a0, str	#taking the address of the str variable..
	li $a1, 100	#taking size of the variable
	li $v0, 8	#command for taking input of a string
	syscall
	
	la $a0,100	#allocating space in heap with size 100
	li $v0,9	#command for allocation
	syscall
	#This print the string we took as an input
	la $a0, str	#taking str adress again 
	li $v0, 4	#command for printing a string
	syscall
	
	#This is the end of the main program , The exit call
	li $v0, 10
	syscall
