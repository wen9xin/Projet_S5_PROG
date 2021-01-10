.global main
.text
main:
    mov r0, #0x11
    mov r1, #0x22
    mov r2, #4
    mov r3, #0xff00
    mvn r4, r3              @ R4 = ~R3 = 0xffff00ff, R4 < 0
    add r5, r0, #0xff00     @ should write 0xff11 to R5
    add r5, r0, r1          @ should write 0x33 to R5
    add r5, r0, r1, LSL #8  @ should write 0x2211 to R5
    add r5, r0, r1, LSL r2  @ should write 0x231 to R5
    add r5, r0, r3, LSR #8  @ should write 0x110 to R5
    add r5, r0, r3, LSR r2  @ should write 0x1001 to R5
    add r5, r0, r4, ASR #8  @ should write 0xffffff11 to R5
    add r5, r0, r4, ASR r2  @ should write 0xffffff20 to R5
    add r5, r0, r4, ROR #12 @ should write 0x10000001 to R5
    add r5, r0, r4, ROR r2  @ should write 0xfffff020 to R5
    add r5, r1, r0, RRX     @ should write 0x2A to R5
    swi 0x123456
    