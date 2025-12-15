# numeric_demo.asm
# Demonstrates using numeric registers ($1, $2, …) instead of symbolic names

        # Initialize base and values
        ADDI $29, $0, 32       # $sp = 32 (stack base, $29 is stack pointer)
        ADDI $1,  $0, 5        # $1 = 5
        ADDI $2,  $0, 7        # $2 = 7

        # Forwarding chain
        ADD  $3, $1, $2        # $3 = 12
        SUB  $4, $3, $1        # $4 = 7
        ADD  $5, $4, $2        # $5 = 14

        # Store and load
        SW   $5, 0($29)        # mem[32] = 14
        LW   $6, 0($29)        # $6 = 14
        ADD  $7, $6, $1        # $7 = 19

        # Branch not taken
        BEQ  $1, $2, equal     # 5 == 7 ? no
        ADDI $8, $0, 1         # $8 = 1
        J    done              # jump -> flush

equal:  ADDI $8, $0, 2         # would set $8=2 if branch taken

done:   NOP
        NOP
