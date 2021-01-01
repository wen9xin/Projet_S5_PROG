/*
Armator - simulateur de jeu d'instruction ARMv5T � but p�dagogique
Copyright (C) 2011 Guillaume Huard
Ce programme est libre, vous pouvez le redistribuer et/ou le modifier selon les
termes de la Licence Publique G�n�rale GNU publi�e par la Free Software
Foundation (version 2 ou bien toute autre version ult�rieure choisie par vous).

Ce programme est distribu� car potentiellement utile, mais SANS AUCUNE
GARANTIE, ni explicite ni implicite, y compris les garanties de
commercialisation ou d'adaptation dans un but sp�cifique. Reportez-vous � la
Licence Publique G�n�rale GNU pour plus de d�tails.

Vous devez avoir re�u une copie de la Licence Publique G�n�rale GNU en m�me
temps que ce programme ; si ce n'est pas le cas, �crivez � la Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307,
�tats-Unis.

Contact: Guillaume.Huard@imag.fr
	 B�timent IMAG
	 700 avenue centrale, domaine universitaire
	 38401 Saint Martin d'H�res
*/
#include "arm_load_store.h"
#include "arm_exception.h"
#include "arm_constants.h"
#include "util.h"
#include "debug.h"

#define get_Lbit(x) (((x)>>(20))&1)
#define get_Bbit(x) (((x)>>(22))&1)
#define get_Ubit(x) (((x)>>(23))&1)
#define get_Rd(x) (((x)>>(12))&0xF)
#define get_Rn(x) (((x)>>(16))&0xF)
#define get_Rm(x) ((x)&0xF)
#define get_Offset12(x) ((x)&0xFFF)

int conditionPassed(arm_core p, uint32_t ins){
    uint32_t cpsr = arm_read_cpsr(p);
    int n = get_bit(cpsr, 31);
    int z = get_bit(cpsr, 30);
    int c = get_bit(cpsr, 29);
    int v = get_bit(cpsr, 28);

    switch(get_bits(ins, 31, 28)){
        case 0:
            return z == 1;
        case 1:
            return z == 0;
        case 2:
            return c == 1;
        case 3:
            return c == 0;
        case 4:
            return n == 1;
        case 5:
            return n == 0;
        case 6:
            return v == 1;
        case 7:
            return v == 0;
        case 8:
            return c == 1 && z == 0;
        case 9:
            return c == 0 || z == 1;
        case 10:
            return n == v;
        case 11:
            return n != v;
        case 12:
            return z == 0 && n == v;
        case 13:
            return z == 1 || n != v;
        case 15:
            return 0xF;
        default: 
            return 1;
    }
}

int rotateRight(int x, int n) {
    int shifted = x >> n;
    int rot_bits = x << (32-n);
    int combined = shifted | rot_bits;

    return combined;
}

// Functions For Addressing mode 2
uint32_t addr_Imm_Offset(arm_core p, uint32_t ins){
    uint32_t addr = 0;
    if(get_Ubit(ins) == 1){
        addr = arm_read_register(p, get_Rn(ins)) + get_Offset12(ins);
    }else{
        addr = arm_read_register(p, get_Rn(ins)) - get_Offset12(ins);
    }
    return addr;
}

uint32_t addr_Reg_Offset(arm_core p, uint32_t ins){
    uint32_t addr = 0;
    if(get_Ubit(ins) == 1){
        addr = arm_read_register(p, get_Rn(ins)) + arm_read_register(p, get_Rm(ins));
    }else{
        addr = arm_read_register(p, get_Rn(ins)) - arm_read_register(p, get_Rm(ins));
    }
    return addr;
}

uint32_t addr_Sca_Offset(arm_core p, uint32_t ins){
    uint32_t addr = 0;
    uint32_t index;
    switch(get_bits(ins, 6, 5)){
        case 0: // LSL
            index = arm_read_register(p, get_Rm(ins)) << get_bits(ins, 11, 7);
            break;
        case 1: // LSR
            index = arm_read_register(p, get_Rm(ins)) >> get_bits(ins, 11, 7);
            break;
        case 2: // ASR
            if(get_bits(ins, 11, 7) == 0){
                if(get_bit(arm_read_register(p, get_Rm(ins)), 31) == 1){
                    index = 0xFFFFFFFF;
                }else index = 0;
            }else{
                index = arm_read_register(p, get_Rm(ins));
                if(get_bit(arm_read_register(p, get_Rm(ins)), 31) == 1){
                    uint32_t mask = 0xFFFFFFFF;
                    mask = ~(mask >> get_bits(ins, 11, 7));
                    index = (index >> get_bits(ins, 11, 7)) & mask;
                }else index = index >> get_bits(ins, 11, 7);
            }
            break;
        default: // ROR or RRX
            if(get_bits(ins, 11, 7) == 0){ // RRX
                uint32_t cpsr = arm_read_cpsr(p);
                uint32_t c = get_bit(cpsr, 29) << 31;
                index = c || (arm_read_register(p, get_Rm(ins)) >> 1);
            }else{ // ROR
                index = rotateRight(arm_read_register(p, get_Rm(ins)), get_bits(ins, 11, 7));
            }
            break;
    }
    if(get_Ubit(ins) == 1){
        addr = arm_read_register(p, get_Rn(ins)) + index;
    }else{
        addr = arm_read_register(p, get_Rn(ins)) - index;
    }
    return addr;
}

int isImmediateOffset(uint32_t ins){
    return get_bit(ins, 25) == 0;
}

int isRegisterOffset(uint32_t ins){
    if(get_bit(ins, 25) == 1){
        if(get_bits(ins, 11, 4) == 0) return 1;
    }
    return 0;
}

int isScaledOffset(uint32_t ins){
    if(get_bit(ins, 25) == 1){
        if(get_bits(ins, 11, 4) != 0) return 1;
    }
    return 0;
}

int isNormalIndexed(uint32_t ins){
    return get_bit(ins, 24) == 1 && get_bit(ins, 21) == 0;
}

int isPreIndexed(uint32_t ins){
    return get_bit(ins, 24) == 1 && get_bit(ins, 21) == 1;
}

int isPostIndexed(uint32_t ins){
    return get_bit(ins, 24) == 0 && get_bit(ins, 21) == 0;
}

uint32_t getAddrByOffset(arm_core p, uint32_t ins){
    uint32_t addr = 0;
    if(isImmediateOffset(ins)) {
        if(isNormalIndexed(ins)) addr = addr_Imm_Offset(p, ins);
        if(isPreIndexed(ins)) addr = addr_Imm_Offset(p, ins);
        if(isPostIndexed(ins)) addr = arm_read_register(p, get_Rn(ins));
    }else if(isRegisterOffset(ins)){
        if(isNormalIndexed(ins)) addr = addr_Reg_Offset(p, ins);
        if(isPreIndexed(ins)) addr = addr_Reg_Offset(p, ins);
        if(isPostIndexed(ins)) addr = arm_read_register(p, get_Rn(ins));
    }else if(isScaledOffset(ins)){
        if(isNormalIndexed(ins)) addr = addr_Sca_Offset(p, ins);
        if(isPreIndexed(ins)) addr = addr_Sca_Offset(p, ins);
        if(isPostIndexed(ins)) addr = arm_read_register(p, get_Rn(ins));
    }
    return addr;
}

int refreshRnByOffset(arm_core p, uint32_t ins, uint32_t addr){
    if(conditionPassed(p, ins)){
        if(isImmediateOffset(ins)) {
            if(isPreIndexed(ins)) arm_write_register(p, get_Rn(ins), addr);
            else if(isPostIndexed(ins)) arm_write_register(p, get_Rn(ins), addr_Imm_Offset(p, ins));
        }else if(isRegisterOffset(ins)){ 
            if(isPreIndexed(ins)) arm_write_register(p, get_Rn(ins), addr);
            else if(isPostIndexed(ins)) arm_write_register(p, get_Rn(ins), addr_Reg_Offset(p, ins));
        }else if(isScaledOffset(ins)){
            if(isPreIndexed(ins)) arm_write_register(p, get_Rn(ins), addr);
            else if(isPostIndexed(ins)) arm_write_register(p, get_Rn(ins), addr_Sca_Offset(p, ins));
        }
    }
    return 0;
}

// Functions for addressing mode 3
int is_Misc_Imm_Offset(uint32_t ins){
    return get_bit(ins, 22) == 1;
}

int is_Misc_Reg_Offset(uint32_t ins){
    return get_bit(ins, 22) == 0;
}

uint32_t misc_Imm_Offset(arm_core p, uint32_t ins){
    uint32_t addr;
    uint8_t offset8 = (get_bits(ins, 11, 8) << 4) || (get_bits(ins, 3, 0));
    if(get_Ubit(ins) == 1) addr = arm_read_register(p, get_Rn(ins)) + offset8;
    else addr = arm_read_register(p, get_Rn(ins)) - offset8;
    return addr;
}

uint32_t misc_Reg_Offset(arm_core p, uint32_t ins){
    uint32_t addr = 0;
    if(get_Ubit(ins) == 1){
        addr = arm_read_register(p, get_Rn(ins)) + arm_read_register(p, get_Rm(ins));
    }else{
        addr = arm_read_register(p, get_Rn(ins)) - arm_read_register(p, get_Rm(ins));
    }
    return addr;
}

uint32_t getMiscAddrByOffset(arm_core p, uint32_t ins){
    uint32_t addr = 0;
    if(is_Misc_Imm_Offset(ins)) {
        if(isNormalIndexed(ins)) addr = misc_Imm_Offset(p, ins);
        if(isPreIndexed(ins)) addr = misc_Imm_Offset(p, ins);
        if(isPostIndexed(ins)) addr = arm_read_register(p, get_Rn(ins));
    }else if(is_Misc_Reg_Offset(ins)){
        if(isNormalIndexed(ins)) addr = misc_Reg_Offset(p, ins);
        if(isPreIndexed(ins)) addr = misc_Reg_Offset(p, ins);
        if(isPostIndexed(ins)) addr = arm_read_register(p, get_Rn(ins));
    }
    return addr;
}

int refreshMiscRnByOffset(arm_core p, uint32_t ins, uint32_t addr){
    if(conditionPassed(p, ins)){
        if(is_Misc_Imm_Offset(ins)) {
            if(isPreIndexed(ins)) arm_write_register(p, get_Rn(ins), addr);
            else if(isPostIndexed(ins)) arm_write_register(p, get_Rn(ins), misc_Imm_Offset(p, ins));
        }else if(is_Misc_Reg_Offset(ins)){
            if(isPreIndexed(ins)) arm_write_register(p, get_Rn(ins), addr);
            else if(isPostIndexed(ins)) arm_write_register(p, get_Rn(ins), misc_Reg_Offset(p, ins));
        }
    }
    return 0;
}

// LDR : Load a 32-bit word. 
int load_Word(arm_core p, uint32_t ins){
    uint32_t data;
    uint32_t addr = getAddrByOffset(p, ins);
    uint8_t rd = get_Rd(ins);
    if(conditionPassed(p, ins)) {
        arm_read_word(p, addr, &data);
        if(rd == 15){
            arm_write_register(p, rd, data & 0xFFFFFFFE);
            uint32_t cpsrModded = (arm_read_cpsr(p) & 0xFFFFFFDF) | (get_bit(data, 0) << 5);
            arm_write_cpsr(p, cpsrModded);
        }else arm_write_register(p, rd, data);
    }
    refreshRnByOffset(p, ins, addr);
    return 0;
}

// LDRB : loads a byte from memory and zero-extends the byte to a 32-bit word
int load_Byte(arm_core p, uint32_t ins){
    uint8_t data;
    uint32_t addr = getAddrByOffset(p, ins);
    uint8_t rd = get_Rd(ins);
    if(conditionPassed(p, ins)) {
        arm_read_byte(p, addr, &data);
        arm_write_register(p, rd, data);
    }
    refreshRnByOffset(p, ins, addr);
    return 0;
}

// LDRBT : loads a byte from memory and zero-extends the byte to a 32-bit word
int load_Byte_Trans(arm_core p, uint32_t ins){
    if(get_bit(ins, 24) != 0 || get_bit(ins, 21) != 1) return 2;
    uint8_t data;
    uint32_t addr = arm_read_register(p, get_Rn(ins));

    uint8_t rd = get_Rd(ins);
    if(conditionPassed(p, ins)) {
        arm_read_byte(p, addr, &data);
        arm_write_usr_register(p, rd, data);
    }
    if(conditionPassed(p, ins)){
        if(isImmediateOffset(ins)) arm_write_register(p, get_Rn(ins), addr_Imm_Offset(p, ins));
        else if(isRegisterOffset(ins)) arm_write_register(p, get_Rn(ins), addr_Reg_Offset(p, ins));
        else if(isScaledOffset(ins)) arm_write_register(p, get_Rn(ins), addr_Sca_Offset(p, ins));
    }
    return 0;
}

// LDRD : loads a pair of ARM registers from two consecutive words of memory.
// Dont know what "CP15_reg1_Ubit" is, so we didnt implement it.
int load_Double_Word(arm_core p, uint32_t ins){
    uint32_t data1, data2;
    uint32_t addr = getMiscAddrByOffset(p, ins);
    uint8_t rd = get_Rd(ins);
    if(conditionPassed(p, ins)) {
        if((rd % 2 == 0) && (rd != 14)){
            if((get_bits(addr, 1, 0) == 0) && (get_bit(addr, 2) == 0)){
                arm_read_word(p, addr, &data1);
                arm_read_word(p, addr + 4, &data2);
                arm_write_register(p, rd, data1);
                arm_write_register(p, rd+1, data2);
            }
        }else return UNDEFINED_INSTRUCTION;
    }
    refreshMiscRnByOffset(p, ins, addr);
    return 0;
}

// LDREX skipped because it was implemrnted in armv6

// LDRH : loads a halfword from memory and zero-extends it to a 32-bit word
int load_Half(arm_core p, uint32_t ins){
    uint16_t data;
    uint32_t addr = getMiscAddrByOffset(p, ins);
    uint8_t rd = get_Rd(ins);
    if(conditionPassed(p, ins)) {
        if(get_bit(addr, 0) == 0) arm_read_half(p, addr, &data);
        else return UNDEFINED_INSTRUCTION;
        arm_write_register(p, rd, data);
    }
    refreshMiscRnByOffset(p, ins, addr);
    return 0;
}

// LDRSB : loads a byte from memory and sign-extends the byte to a 32-bit word.
int load_Signed_Byte(arm_core p, uint32_t ins){
    uint8_t data;
    uint32_t data32;
    uint32_t addr = getMiscAddrByOffset(p, ins);
    uint8_t rd = get_Rd(ins);
    if(conditionPassed(p, ins)) {
        arm_read_byte(p, addr, &data);
        if(get_bit(data, 7) == 0) data32 = data;
        else data32 = data | 0xFFFFFF00;
        arm_write_register(p, rd, data32);
    }
    refreshMiscRnByOffset(p, ins, addr);
    return 0;
}

// LDRSH : loads a half from memory and sign-extends the byte to a 32-bit word.
int load_Signed_Half(arm_core p, uint32_t ins){
    uint16_t data = 0;
    uint32_t data32;
    uint32_t addr = getMiscAddrByOffset(p, ins);
    uint8_t rd = get_Rd(ins);
    if(conditionPassed(p, ins)) {
        if(get_bit(addr, 0) == 0) arm_read_half(p, addr, &data);
        else return UNDEFINED_INSTRUCTION;

        if(get_bit(data, 15) == 0) data32 = data;
        else data32 = data | 0xFFFF0000;
        arm_write_register(p, rd, data32);
    }
    refreshMiscRnByOffset(p, ins, addr);
    return 0;
}

// LDRT : loads a word from memory with translation.
int load_Word_Trans(arm_core p, uint32_t ins){
    if(get_bit(ins, 24) != 0 || get_bit(ins, 21) != 1) return 2;
    uint32_t data;
    uint32_t addr = arm_read_register(p, get_Rn(ins));

    uint8_t rd = get_Rd(ins);
    if(conditionPassed(p, ins)) {
        arm_read_word(p, addr, &data);
        arm_write_usr_register(p, rd, data);
    }
    if(conditionPassed(p, ins)){
        if(isImmediateOffset(ins)) arm_write_register(p, get_Rn(ins), addr_Imm_Offset(p, ins));
        else if(isRegisterOffset(ins)) arm_write_register(p, get_Rn(ins), addr_Reg_Offset(p, ins));
        else if(isScaledOffset(ins)) arm_write_register(p, get_Rn(ins), addr_Sca_Offset(p, ins));
    }
    return 0;
}

// STR : Load a 32-bit word. 
int store_Word(arm_core p, uint32_t ins){
    uint32_t data;
    uint32_t addr = getAddrByOffset(p, ins);
    uint8_t rd = get_Rd(ins);
    if(conditionPassed(p, ins)) {
        data = arm_read_register(p, rd);
        arm_write_word(p, addr, data);
    }
    refreshRnByOffset(p, ins, addr);
    return 0;
}

// STRB : Load a 8-bit byte from the least significant byte of a register. 
int store_Byte(arm_core p, uint32_t ins){
    uint8_t data;
    uint32_t addr = getAddrByOffset(p, ins);
    uint8_t rd = get_Rd(ins);
    if(conditionPassed(p, ins)) {
        data = arm_read_register(p, rd) & 0xFF;
        arm_write_byte(p, addr, data);
    }
    refreshRnByOffset(p, ins, addr);
    return 0;
}

// STRBT : Load a 8-bit byte with translation. 
int store_Byte_Trans(arm_core p, uint32_t ins){
    if(get_bit(ins, 24) != 0 || get_bit(ins, 21) != 1) return 2;
    uint8_t data;
    uint32_t addr = arm_read_register(p, get_Rn(ins));
    uint8_t rd = get_Rd(ins);
    if(conditionPassed(p, ins)) {
        data = arm_read_usr_register(p, rd) & 0xFF;
        arm_write_byte(p, addr, data);
    }
    if(conditionPassed(p, ins)){
        if(isImmediateOffset(ins)) arm_write_register(p, get_Rn(ins), addr_Imm_Offset(p, ins));
        else if(isRegisterOffset(ins)) arm_write_register(p, get_Rn(ins), addr_Reg_Offset(p, ins));
        else if(isScaledOffset(ins)) arm_write_register(p, get_Rn(ins), addr_Sca_Offset(p, ins));
    }
    return 0;
}

// STRD : tores a pair of ARM registers to two consecutive words of memory..
// Dont know what "CP15_reg1_Ubit" is, so we didnt implement it.
int store_Double_Word(arm_core p, uint32_t ins){
    uint32_t data1, data2;
    uint32_t addr = getMiscAddrByOffset(p, ins);
    uint8_t rd = get_Rd(ins);
    if(conditionPassed(p, ins)) {
        if((rd % 2 == 0) && (rd != 14)){
            if((get_bits(addr, 1, 0) == 0) && (get_bit(addr, 2) == 0)){
                data1 = arm_read_register(p, rd);
                data2 = arm_read_register(p, rd + 1);
                arm_write_word(p, addr, data1);
                arm_write_word(p, addr + 4, data2);
            }
        }else return UNDEFINED_INSTRUCTION;
    }
    refreshMiscRnByOffset(p, ins, addr);
    return 0;
}

// STREX not implemented.

// STRH stores a halfword from the least significant halfword of a register to memory.
int store_Half(arm_core p, uint32_t ins){
    uint16_t data;
    uint32_t addr = getMiscAddrByOffset(p, ins);
    uint8_t rd = get_Rd(ins);
    if(conditionPassed(p, ins)) {
        if(get_bit(addr, 0) == 0) data = arm_read_register(p,rd);
        else return UNDEFINED_INSTRUCTION;
        arm_write_half(p, addr, data);
    }
    refreshMiscRnByOffset(p, ins, addr);
    return 0;
}

// STRT : loads a word from memory with translation.
int store_Word_Trans(arm_core p, uint32_t ins){
    if(get_bit(ins, 24) != 0 || get_bit(ins, 21) != 1) return 2;
    uint32_t data;
    uint32_t addr = arm_read_register(p, get_Rn(ins));
    uint8_t rd = get_Rd(ins);
    if(conditionPassed(p, ins)) {
        data = arm_read_usr_register(p, rd) & 0xFF;
        arm_write_word(p, addr, data);
    }
    if(conditionPassed(p, ins)){
        if(isImmediateOffset(ins)) arm_write_register(p, get_Rn(ins), addr_Imm_Offset(p, ins));
        else if(isRegisterOffset(ins)) arm_write_register(p, get_Rn(ins), addr_Reg_Offset(p, ins));
        else if(isScaledOffset(ins)) arm_write_register(p, get_Rn(ins), addr_Sca_Offset(p, ins));
    }
    return 0;
}

int arm_load_store(arm_core p, uint32_t ins) {
    int bit27 = get_bit(ins, 27);
    int bit26 = get_bit(ins, 26);
    int bit25 = get_bit(ins, 25);
    int bit22 = get_bit(ins, 22); // B bit
    int bit20 = get_bit(ins, 20); // L bit
    int bit7 = get_bit(ins, 7);
    int bit6 = get_bit(ins, 6); // S bit
    int bit5 = get_bit(ins, 5); // H bit
    int bit4 = get_bit(ins, 4); 

    if(bit27 == 0 && bit26 == 1){ 
        // Load and store word or unsigned byte instructions
        if(bit22 == 1 && bit20 == 1){
            if(get_bit(ins, 24) == 0 && get_bit(ins, 21) == 1) load_Byte_Trans(p, ins); // LDRBT
            else load_Byte(p, ins); // LDRB
        } 
        else if(bit22 == 1 && bit20 == 0) {
            if(get_bit(ins, 24) == 0 && get_bit(ins, 21) == 1) store_Byte_Trans(p, ins); // STRBT
            else store_Byte(p, ins); //STRB
        }
        else if(bit22 == 0 && bit20 == 1){
            if(get_bit(ins, 24) == 0 && get_bit(ins, 21) == 1) load_Word_Trans(p, ins); // LDRT
            else load_Word(p, ins); // LDR
        } 
        else if(bit22 == 0 && bit20 == 0){
            if(get_bit(ins, 24) == 0 && get_bit(ins, 21) == 1) store_Word_Trans(p, ins); // STRT
            else store_Word(p, ins); // STR
        } else return UNDEFINED_INSTRUCTION;

    }else if(bit27 == 0 && bit26 == 0 && bit25 == 0 && bit7 == 1 && bit4 == 1){
        if(bit20 == 0 && bit6 == 1 && bit5 == 0) load_Double_Word(p, ins); // LDRD
        else if(bit20 == 1 && bit6 == 0 && bit5 == 1) load_Half(p, ins); // LDRH
        else if(bit20 == 1 && bit6 == 0 && bit5 == 1) load_Signed_Byte(p, ins); // LDRSB
        else if(bit20 == 1 && bit6 == 1 && bit5 == 1) load_Signed_Half(p, ins); // LDRSH
        else if(bit20 == 0 && bit6 == 1 && bit5 == 1) store_Double_Word(p, ins);// STRD
        else if(bit20 == 0 && bit6 == 0 && bit5 == 1) store_Half(p, ins); // STRH
        else return UNDEFINED_INSTRUCTION;
    }else return UNDEFINED_INSTRUCTION;
    return 0;
}

// Function for Addressing Mode 4
uint32_t nbSetBits(uint32_t ins){
    int count = 0;
    int number = ins & 0xFFFF;
    while(number){
        number &= (number -1);
        count ++;
    }
    return count;
}

uint32_t getStartAddress(arm_core p, uint32_t ins){
    int idab = get_bits(ins, 24, 23);
    switch(idab){
        case 1: // Increment after
            return arm_read_register(p, get_Rn(ins));
        case 3 : // Increment before
            return arm_read_register(p, get_Rn(ins)) + 4;
        case 0 : //Decrement after
            return arm_read_register(p, get_Rn(ins)) - 4*nbSetBits(ins) + 4;
        default : // Decrement before
            return arm_read_register(p, get_Rn(ins)) - 4*nbSetBits(ins);
    }
}

uint32_t getEndAddress(arm_core p, uint32_t ins){
    int idab = get_bits(ins, 24, 23);
    switch(idab){
        case 1: // Increment after
            return arm_read_register(p, get_Rn(ins)) + 4*nbSetBits(ins) - 4;
        case 3 : // Increment before
            return arm_read_register(p, get_Rn(ins)) + 4*nbSetBits(ins);
        case 0 : //Decrement after
            return arm_read_register(p, get_Rn(ins));
        default : // Decrement before
            return arm_read_register(p, get_Rn(ins)) - 4;
    }
}

// LDM (1) (Load Multiple) loads a non-empty subset, or possibly all, 
// of the general-purpose registers from sequential memory locations.
int ldm1(arm_core p, uint32_t ins){
    if(conditionPassed(p, ins)){
        uint32_t data = 0;
        uint32_t startAddr = getStartAddress(p, ins);
        uint32_t currAddr = startAddr;
        for(int i=0; i<15; i++){
            if(get_bit(ins, i) == 1){
                arm_read_word(p, currAddr, &data);
                arm_write_register(p, i, data);
                currAddr = currAddr + 4;
            }
        }
        if(get_bit(ins, 15) == 1){
            arm_read_word(p, currAddr, &data);
            arm_write_register(p, 15, data & 0xFFFFFFFE);
            uint32_t cpsrModded = (arm_read_cpsr(p) & 0xFFFFFFDF) | (get_bit(data, 0) << 5);
            arm_write_cpsr(p, cpsrModded);
            currAddr = currAddr + 4;
        }
        if(currAddr - 4 != getEndAddress(p, ins)) return 1;
    }
    return 0;
}

// LDM(2) loads User mode registers when the processor is in a privileged mode.
int ldm2(arm_core p, uint32_t ins){
    if(conditionPassed(p, ins)){
        uint32_t data = 0;
        uint32_t startAddr = getStartAddress(p, ins);
        uint32_t currAddr = startAddr;
        for(int i=0; i<15; i++){
            if(get_bit(ins, i) == 1){
                arm_read_word(p, currAddr, &data);
                arm_write_usr_register(p, i, data);
                currAddr = currAddr + 4;
            }
        }
        if(currAddr - 4 != getEndAddress(p, ins)) return 1;
    }
    return 0;
}

// LDM (3) loads a subset, or possibly all, 
// of the general-purpose registers and the PC from sequential memory locations.
int ldm3(arm_core p, uint32_t ins){
    if(conditionPassed(p, ins)){
        uint32_t data = 0;
        uint32_t startAddr = getStartAddress(p, ins);
        uint32_t currAddr = startAddr;
        for(int i=0; i<15; i++){
            if(get_bit(ins, i) == 1){
                arm_read_word(p, currAddr, &data);
                arm_write_register(p, i, data);
                currAddr = currAddr + 4;
            }
        }
        
        if(arm_current_mode_has_spsr(p)) arm_write_cpsr(p, arm_read_spsr(p));

        uint32_t value = 0;
        arm_read_word(p, currAddr, &value);
        arm_write_register(p, 15, value);
        currAddr = currAddr + 4;

        if(currAddr - 4 != getEndAddress(p, ins)) return 1;
    }
    return 0;
}

// STM (1) (Store Multiple) stores a non-empty subset (or possibly all) 
// of the general-purpose registers to sequential memory locations. 
int stm1(arm_core p, uint32_t ins){
    if(conditionPassed(p, ins)){
        uint32_t data = 0;
        uint32_t startAddr = getStartAddress(p, ins);
        uint32_t currAddr = startAddr;
        for(int i =0; i<16; i++){
            data = arm_read_register(p, i);
            arm_write_word(p, currAddr, data);
            currAddr = currAddr + 4;
        }
        if(currAddr - 4 != getEndAddress(p, ins)) return 1;
    }
    return 0;
}

// STM (2) stores a subset (or possibly all) of the User mode 
// general-purpose registers to sequential memory locations.
int stm2(arm_core p, uint32_t ins){
    if(conditionPassed(p, ins)){
        uint32_t data = 0;
        uint32_t startAddr = getStartAddress(p, ins);
        uint32_t currAddr = startAddr;
        for(int i =0; i<16; i++){
            data = arm_read_usr_register(p, i);
            arm_write_word(p, currAddr, data);
            currAddr = currAddr + 4;
        }
        if(currAddr - 4 != getEndAddress(p, ins)) return 1;
    }
    return 0;
}


int arm_load_store_multiple(arm_core p, uint32_t ins) {
    int bit27 = get_bit(ins, 27);
    int bit26 = get_bit(ins, 26);
    int bit25 = get_bit(ins, 25);
    int bit22 = get_bit(ins, 22); // S bit
    int bit21 = get_bit(ins, 21); // W bit
    int bit20 = get_bit(ins, 20); // L bit
    int bit15 = get_bit(ins, 15); // PC Reg
    
    if(bit27 == 1 && bit26 == 0 && bit25 == 0){
        {
            if(bit22 == 0 && bit20 == 1) ldm1(p, ins);
            else if (bit22 == 1 && bit20 == 1 && bit15 == 0) ldm2(p, ins);
            else if (bit22 == 1 && bit20 == 1 && bit15 == 1) ldm3(p, ins);
            else if (bit22 == 0 && bit20 == 0) stm1(p, ins);
            else if (bit22 == 1 && bit21 == 0 && bit20 == 0) stm2(p, ins);
            else return UNDEFINED_INSTRUCTION;
        }
    }
    return 0;
}

int arm_coprocessor_load_store(arm_core p, uint32_t ins) {
    /* Not implemented */
    return UNDEFINED_INSTRUCTION;
}
