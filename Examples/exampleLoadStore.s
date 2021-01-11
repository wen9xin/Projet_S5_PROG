.global main
.text
main:
    ldr r0, =limite
    add r4, r0, #4

    ldr r2, [r0]        @ should load 0x11223344 to R2
    ldrb r2, [r0]       @ should load 0x00000011 to R2
    ldrbt r2, [r0]      @ should load 0x00000011 to R2_USR
    ldrd r2, [r0]       @ should load 0x11223344 to R2, and load 0x99aabbcc to R3
    ldrh r2, [r0]       @ should load 0x00112233 to R2
    ldrsb r2, [r4]      @ should load 0xffffff99 to R2
    ldrsh r2, [r4]      @ should load 0xffff99aa to R2
    ldrt r2, [r4]       @ should load 0x11223344 to R2_USR

    mov r2, #0xff00
    mov r3, #0xa0
    mov r4, #0xff0

    str r2, [r0]        @ should store 0x0000FF00 to Mem[0x2800]
    strb r3, [r0]       @ should store 0x000000A0 to Mem[0x2800]
    strbt r3, [r0]      @ should store 0x000000A0 to Mem[0x2800]
    strd r2, [r0]       @ should store 0x0000FF00 to Mem[0x2800], and store 0x000000A0 to Mem[0x2804]
    strh r4, [r0]       @ should store 0x00000FF0 to Mem[0x2800]
    strt r2, [r0]       @ should store 0x0000FF00 to Mem[0x2800]
    swi 0x123456        
.data
limite:
    .word 0x11223344
    .word 0x99aabbcc
fin:
