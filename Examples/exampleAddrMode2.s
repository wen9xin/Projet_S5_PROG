.global main
.text
main:
    ldr r0, =limite             @ R0 = 0x2800
    mov r1, #0x1                @ R1 = 0x1
    add r2, r0, #8              @ R2 = 0x2808
    mov r3, #8                  @ R3 = 0x8

    ldr r4, [r2, #4]            @ should load 0xccddeeff to R4
    ldr r4, [r2, #-4]           @ should load 0x44556677 to R4
    ldr r4, [r2, r3]            @ should load 0x12345678 to R4
    ldr r4, [r2, -r3]           @ should load 0x00112233 to R4
    ldr r4, [r2, r1, LSL #2]    @ should load 0xccddeeff to R4
    ldr r4, [r2, r3, LSR #1]    @ should load 0xccddeeff to R4
    ldr r4, [r2, r3, ASR #1]    @ should load 0xccddeeff to R4
    ldr r4, [r2, r3, ROR #1]    @ should load 0xccddeeff to R4
    ldr r4, [r2, r3, RRX]       @ should load 0xccddeeff to R4

    ldr r4, [r2, #4]!           @ should load 0xccddeeff to R4, then write 0x0000280C to R2
    ldr r4, [r2, #-4]!          @ should load 0x8899aabb to R4 (because R2 is 0x280C now), then write 0x00002808 to R2
    ldr r4, [r2, r3]!           @ should load 0x12345678 to R4 (because R2 is 0x2808 now), then write 0x00002810 to R2
    ldr r4, [r2, -r3]!          @ should load 0x8899aabb to R4 (because R2 is 0x2810 now), then write 0x00002808 to R2
    ldr r4, [r2, r1, LSL #2]!   @ should load 0xccddeeff to R4 (because R2 is 0x2808 now), then write 0x0000280C to R2
    
    ldr r4, [r2], #4            @ should load 0xccddeeff to R4 (because R2 is 0x280C now), then write 0x00002810 to R2
    ldr r4, [r2], #-4           @ should load 0x12345678 to R4 (because R2 is 0x280C now), then write 0x0000280C to R2
    ldr r4, [r2], r3            @ should load 0xccddeeff to R4 (because R2 is 0x280C now), then write 0x00002814 to R2
    ldr r4, [r2], -r3           @ should load 0x00000000 to R4 (because R2 is 0x2814, and it's empty), then write 0x0000280C to R2
    ldr r4, [r2], r1, LSL #2    @ should load 0xccddeeff to R4 (because R2 is 0x280C now), then write 0x00002810 to R2
    ldr r4, [r2], -r3, LSR #1   @ should load 0x12345678 to R4 (because R2 is 0x2810 now), then write 0x0000280C to R2

    swi 0x123456

.data
limite:
    .word 0x00112233
    .word 0x44556677
    .word 0x8899aabb
    .word 0xccddeeff
    .word 0x12345678
fin:
