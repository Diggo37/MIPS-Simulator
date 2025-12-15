# pipeline_test.asm
# Exercises arithmetic, immediates, shifts, logicals, mul, loads/stores, branches, jump, and hazards.
# Registers: $t0-$t7 ($8-$15), $s0-$s3 ($16-$19), $sp ($29)
# Memory: word-addressed; use small offsets for clarity.

        ADDI $sp, $zero, 8        # set $sp = 8 (stack base at mem[8])
        ADDI $t0, $zero, 3       # t0 = 3
        ADDI $t1, $zero, 4       # t1 = 4
        MUL  $t2, $t0, $t1       # t2 = t0 * t1 = 12
        SLL  $t3, 2, $t2         # t3 = t2 << 2  (shift by immediate 2)
        SRL  $t4, 1, $t3         # t4 = t3 >> 1  (logical)
        AND  $t5, $t2, $t4      # t5 = t2 & t4
        OR   $t6, $t2, $t4      # t6 = t2 | t4

        SW   $t2, 0($sp)        # mem[8] = t2 (12)
        LW   $s0, 0($sp)        # s0 = mem[8] (load-use hazard for next instruction)
        ADD  $s1, $s0, $t1      # s1 = s0 + t1  (depends on LW result -> load-use stall)
        ADDI $s2, $s1, 5        # s2 = s1 + 5

        BEQ  $t0, $t1, equal    # branch not taken (3 != 4)
        ADDI $t7, $zero, 1      # delay slot style filler (will execute)
        J    done               # jump to done (flushes following IF)

equal:  ADDI $t7, $zero, 2      # if branch taken, t7 = 2

        NOP
done:   ADD  $s3, $t2, $t6      # s3 = t2 + t6
        NOP
