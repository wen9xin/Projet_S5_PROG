.global main
.text
main:
    ldr r0, =limite             @ R0 = 0x2800
    mov r1, #0x1                @ R1 = 0x1
    add r2, r0, #8              @ R2 = 0x2808
    mov r3, #8                  @ R3 = 0x8
    
    ldrh r4, [r2]               @ should load 0x8899 to R4
    ldrh r4, [r2, #4]           @ should load 0xccdd to R4
    ldrh r4, [r2, #-4]          @ should load 0x4455 to R4
    ldrh r4, [r2, r3]           @ should load 0x1234 to R4
    ldrh r4, [r2, -r3]          @ should load 0x0011 to R4

    ldrh r4, [r2, #4]!          @ should load 0xccdd to R4, then write 0x280C to R2
    ldrh r4, [r2, #-4]!         @ should load 0x8899 to R4 (because R2 is 0x280C now), then write 0x2808 to R2
    ldrh r4, [r2, r3]!          @ should load 0x1234 to R4 (because R2 is 0x2808 now), then write 0x2810 to R2
    ldrh r4, [r2, -r3]!         @ should load 0x8899 to R4 (because R2 is 0x2810 now), then write 0x2808 to R2

    ldrh r4, [r2], #4           @ should load 0x8899 to R4 (because R2 is 0x2808 now), then write 0x280C to R2
    ldrh r4, [r2], #-4          @ should load 0xccdd to R4 (because R2 is 0x280C now), then write 0x2808 to R2
    ldrh r4, [r2], r3           @ should load 0x8899 to R4 (because R2 is 0x2808 now), then write 0x2810 to R2
    ldrh r4, [r2], -r3          @ should load 0x1234 to R4 (because R2 is 0x2810 now), then write 0x2808 to R2

    swi 0x123456

.data
limite:
    .word 0x00112233
    .word 0x44556677
    .word 0x8899aabb
    .word 0xccddeeff
    .word 0x12345678
fin:
