# safe_demo_mixed.asm — hazard-free, some lines use numeric registers

# Initialize independent registers
ADDI $t0, $zero, 5        # symbolic
ADDI $9,  $0,    12       # numeric form of $t1
ADDI $t2, $zero, 7
ADDI $3,  $0,    3        # numeric form of $t3
ADDI $t4, $zero, 9
ADDI $29, $0,    64       # numeric form of $sp

# Independent ALU operations
ADD  $s0, $t0, $9         # mix: symbolic $t0, numeric $9
SUB  $s1, $t2, $3         # mix: symbolic $t2, numeric $3
AND  $s2, $t0, $t4
OR   $s3, $9,  $t2        # mix: numeric $9, symbolic $t2
MUL  $s4, $3,  $t4        # mix: numeric $3, symbolic $t4

# Shift operations
SLL  $s5, 2, $9           # numeric $9
SRL  $s6, 1, $t4

# Memory operations
SW   $s0, 0($29)          # numeric $29 for $sp
SW   $s1, 4($sp)          # symbolic $sp
LW   $s7, 0($29)          # numeric $29
LW   $5,  4($sp)          # numeric $5 for $t5

# Further independent operations
ADD  $t6, $t0, $t2
SUB  $t7, $t4, $3         # numeric $3
AND  $t8, $9,  $3         # mix: numeric $9 and $3
OR   $t9, $t2, $t4

# Control instructions
BEQ  $t0, $9, label_not_taken   # mix symbolic + numeric

J    label_after_jump

ADD  $s0, $t6, $t7
AND  $s1, $t8, $t9

label_not_taken:
ADDI $1, $0, 1           # numeric $1 for $a0
ADDI $a1, $zero, 2       # symbolic $a1
ADD  $a2, $1, $a1        # mix numeric $1 and symbolic $a1

label_after_jump:
SLL  $a3, 3, $a2
SRL  $v0, 1, $t6

NOP
NOP
