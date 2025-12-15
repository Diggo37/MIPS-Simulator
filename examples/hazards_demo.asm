# hazards_demo.asm
# Demonstrates:
# - Forwarding (no stall) between back-to-back ALU instructions
# - Load-use data hazard causing a 1-cycle stall
# - Control hazard: branch resolved in EX causing a flush
# - Jump causing a flush

        # Initialize base and values
        ADDI $sp, $zero, 32       # stack base (word address 32)
        ADDI $t0, $zero, 5        # t0 = 5
        ADDI $t1, $zero, 7        # t1 = 7

        # Forwarding chain (no stall expected if forwarding is implemented)
        ADD  $t2, $t0, $t1        # t2 = 12  (uses $t0,$t1)
        SUB  $t3, $t2, $t0        # t3 = 7   (forward t2 from EX/MEM)
        ADD  $t4, $t3, $t1        # t4 = 14  (forward t3)

        # Store and load: create load-use stall
        SW   $t4, 0($sp)          # mem[32] = 14
        LW   $t5, 0($sp)          # t5 = mem[32] (14)
        ADD  $t6, $t5, $t0        # t6 = 19 (depends on LW result -> 1-cycle stall inserted)

        # Branch not taken: still shows branch resolution point
        BEQ  $t0, $t1, equal      # 5 == 7 ? no (not taken)
        ADDI $t7, $zero, 1        # falls through and executes
        J    done                 # jump -> flushes the next fetched instruction

equal:  ADDI $t7, $zero, 2        # would set t7=2 if branch taken (won't execute)

done:   NOP
        NOP
