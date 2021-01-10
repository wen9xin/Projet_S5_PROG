.global main
.text
main:
    mov r0, #0x11
    mov r1, #0x22
    mov r2, #0x33
    adds r3, r0, r1     @ should write 0x33 to R3, with nzcv = 0000
    subs r3, r1, r0     @ should write 0x11 to R3, with nzcv = 0010
    rsbs r3, r0, r1     @ should write 0x11 to R3, with nzcv = 0010
    subs r3, r0, r1     @ should write 0xFFFFFFEF to R3, with nzcv = 1001
    rsbs r3, r1, r0     @ should write 0xFFFFFFEF to R3, with nzcv = 1001
    mov r0, #0xff000000
    adds r5, r0, r0     @ should write 0xFE000000 to R5, with nzcv = 1010
    adcs r3, r1, r1     @ should write 0x45 to R3, with nzcv = 0000
    sub r3, r2, r1      @ should write 0x11 to R3
    sbc r3, r2, r1      @ should write 0x10 to R3
    rsc r3, r1, r2      @ should write 0x10 to R3
    sbc r3, r1, r2      @ should write 0xFFFFFFEE to R3
    rsc r3, r2, r1      @ should write 0xFFFFFFEE to R3
    mov r0, #0x00ff
    mov r1, #0
    mov r2, #0xff00
    mvn r6, r1          @ move ~R1(0xffffffff) to R6
    and r4, r0, r1      @ should write 0 to R4
    and r4, r2, r1      @ should write 0 to R4
    orr r4, r0, r1      @ should write 0xFF to R4
    orr r4, r2, r1      @ should write 0xFF00 to R4
    bic r4, r0, r1      @ should write 0xFF to R4
    bic r4, r2, r1      @ should write 0xFF00 to R4
    eor r4, r0, r1      @ should write 0xFF to R4
    eor r4, r0, r6      @ should write 0xFFFFFF00 to R4
    
    add r1, r1, #1      @ R1 = 1 = -R6

    cmn r0, r0          @ z = 0 because R0 != -R0
    cmn r1, r6          @ z = 1 because R1 == -R6
    cmp r0, r0          @ z = 1 because R0 == R0
    cmp r0, r1          @ z = 0 because R0 != R1

    tst r0, r2          @ z = 1
    tst r0, r0          @ z = 0
    teq r0, r2          @ z = 0
    teq r0, r0          @ z = 1
    swi 0x123456
    