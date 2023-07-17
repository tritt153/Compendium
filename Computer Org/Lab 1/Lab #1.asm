	##Travis Ritter. Section: 01
	.data
Prompt1: .asciiz "\n Please enter an Integer: "
Prompt2: .asciiz "\n The sum from 1 - N: "
Done: .asciiz "\n Have a nice day."

	.text
main:
	li $v0, 4 #load string print service
	la $a0, Prompt1 #load string into a0
	syscall #print message
	
	li $v0, 5 ##load int reading service
	syscall ##read an int from the user
	
	move $t0, $v0 #store the user input
while:
	bltz $t0, end #Branch, when t0 is < 0 meaning it is not > 0 which means the loop is finished
	add $s1, $s1, $t0 #add the user input to a variable
	addi $t0, $t0, -1 #decriment input by one
	j while #unconditional jump to start of loop to see if the condtion has changed
end:
	li $v0, 4 #load string print service
	la $a0 , Prompt2 #load string into a0
	syscall #print message
	
	move $a0, $s1 #store the sum result into a0
	li $v0, 1 #load int print service 
	syscall #print sum
	
	li $v0, 4 #load string print service
	la $a0, Done #load string into a0
	syscall #print message 