.global main
.text
main:
    mov r0, #0x11
    mov r1, #0x22
    mov r2, #0x33
    add r3, r0, r1
    sub r3, r1, r0
    rsc r3, r2, r0
    adc r3, r2, r1
    rsb r3, r2, r1
    
    mov r0, #0xaaaa
    mov r1, #0x5555
    mov r2, #0xFFFF

    and r4, r0, r1
    orr r4, r0, r1
    bic r4, r0, r1
    eor r4, r0, r1

    cmn r0, r0
    cmn r0, r1
    cmp r0, r0
    cmp r0, r1
    swi 0x123456
    