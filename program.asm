# simple test: compute 5 + 7 -> $t2, store to memory, load back
ADDI $1, $0, 5
ADDI $2, $0, 7
ADD  $3, $1, $2
SW   $3, 0($sp)
LW   $5, 0($sp)
NOP
