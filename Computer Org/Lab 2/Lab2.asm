#Travis Ritter, Section: 01
.data
Prompt1: .asciiz "Please enter first int: "
Prompt2: .asciiz "Please enter second int: "
Prompt3: .asciiz "Sum from [x, y]: "
End: .asciiz "\nEnd of program" 
Error: .asciiz "First number MUST be less than second"

.text
main: #loads prompts, accepts user input, and checks if x > y
	li $v0, 4 ##load string print service
	la $a0, Prompt1 ##load string into a0
	syscall ## print
	li $v0, 5 ##load int reading service
	syscall ##read an int
	move $s1, $v0 ##store the input (x)
	
	li $v0, 4 ##load string print service
	la $a0, Prompt1 ##load string into a0
	syscall ## print
	li $v0, 5 ##load int reading service
	syscall ##read an int
	move $s2, $v0 ##store the input (y)
	
	sgt $t0, $s1, $s2 #x must be less than y, so if it is greater than, that 
			  #is an error
	beq $t0, 1, error #if this equals 1, then x > y, which is not allowed
	
	andi $t0, $s1, 1 #checks the rightmost bit against 1
	beq $t0, 1, fixX #if it does equal 1, that means that the last bit is 1, meaning
			 #it is odd then it jumps and adds one, to make it even
			 
checkY: #check to see if Y is odd, if it is jump to make it even
	andi $t0, $s2, 1 #checks the rightmost bit against 1
	beq $t0, 1, fixY #if it does equal 1, that means that the last bit is 1, meaning
			 #it is odd then it jumps and subtracts one, to make it even
			 
loop: #for loop that adds the even integers from [x, y]
	add $s3, $s3, $s1 #add s1 (x) to the total
	addi $s1, $s1, 2 #increment the first number by 2, since we are adding even numbers
	bgt $s1, $s2, end #check to see if the first number is 
	j loop #if it doesn't branch, then jump back to the top and loop again
	
end: #prints out results and exits program
	li $v0, 4 #load string print service
	la $a0, Prompt3 #load message into a0
	syscall #print message
	move $a0, $s3 #move the sum total to a0
	li $v0, 1 #load int print service
	syscall #print int
	li $v0, 4 #load string print service
	la $a0, End #load ending message into a0
	syscall #print end message
	li $v0, 10 #load system exit 
	syscall #exit program
	
error: #prints out error when x > y, and exits program
	li $v0, 4 #load string print service
	la $a0, Error #load error message
	syscall #print error message
	li $v0, 10 #load system exit
	syscall	#exit
	
fixX: #used to correct x when it is odd, and make it even
	addi $s1, $s1, 1 #add 1 to the x to make it even 
	j checkY #jump to thenext check
fixY: #used to correct y when it is odd, and make it even
	addi $s2, $s2, -1 #subtract 1 to y to make it even 
	j loop #jump to the start of the loop