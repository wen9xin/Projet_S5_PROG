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
#include "arm_branch_other.h"
#include "arm_constants.h"
#include "util.h"
#include <debug.h>
#include <stdlib.h>

int get_bit_24(uint32_t ins){
    return get_bit(ins, 24);
}

uint32_t calcBLTargetAddress(uint32_t ins, int mode){
    // Mode : 0 --> BL, 1--> BLX
    uint32_t res = get_bits(ins, 23, 0) | 0x00000000;
    if(get_bit(res, 23) == 1){
        if(mode) res = res | 0xFF000000 | get_bit_24(ins) << 1;
        else res = res | 0xFF000000;
    } 
    return res << 2;
}

int blx1_procedure(arm_core p, uint32_t ins){
    uint32_t pc = arm_read_register(p, 15);
    if(conditionPassed(p, ins)){
        if(get_bit_24(ins)) arm_write_register(p, 14, pc - 4);
        pc = pc + calcBLTargetAddress(ins, conditionPassed(p, ins) == 15);
        arm_write_register(p, 15, pc);
    }

    if(conditionPassed(p, ins) == 15){
        arm_write_register(p, 14, pc - 4);
        uint32_t cpsr = arm_read_cpsr(p);
        cpsr = (cpsr & 0xFFFFFFDF) | 0x20;
        arm_write_cpsr(p, cpsr);
        pc = pc + calcBLTargetAddress(ins, conditionPassed(p, ins) == 15);
        arm_write_register(p, 15, pc);
    }
    return 0;
}

int blx2_procedure(arm_core p, uint32_t ins){
    if(conditionPassed(p, ins)){
        uint32_t rm_data = arm_read_register(p, get_bits(ins, 3, 0));
        uint32_t pc = arm_read_register(p, 15);
        if(get_bit(ins, 5) == 1) arm_write_register(p, 14, pc);

        uint32_t cpsr = arm_read_cpsr(p);
        cpsr = (cpsr & 0xFFFFFFDF) | (get_bit(rm_data, 0) << 5);
        arm_write_cpsr(p, cpsr);

        pc = rm_data & 0xFFFFFFFE;
        arm_write_register(p, 15, pc);
    }
    return 0;
}

int arm_branch(arm_core p, uint32_t ins) {
    if(get_bits(ins, 27, 25) == 5){
        blx1_procedure(p, ins);
    }else return UNDEFINED_INSTRUCTION;
    return 0;
}

int arm_branch_misc(arm_core p, uint32_t ins){
    if(get_bits(ins, 27, 20) == 0x12){
        blx2_procedure(p, ins);
    }else return UNDEFINED_INSTRUCTION;
    return 0;
}
    
int arm_coprocessor_others_swi(arm_core p, uint32_t ins) {
    if (get_bit(ins, 24)) {
        /* Here we implement the end of the simulation as swi 0x123456 */
        if ((ins & 0xFFFFFF) == 0x123456)
            exit(0);
        return SOFTWARE_INTERRUPT;
    } 
    return UNDEFINED_INSTRUCTION;
}

int mrs_procedure(arm_core p,uint32_t ins){
    if(conditionPassed(p, ins)){
        uint8_t rd = get_bits(ins, 15, 12);
        uint32_t val = 0;
        if(get_bit(ins, 22) == 1) val = arm_read_spsr(p);
        else val = arm_read_cpsr(p);
        arm_write_register(p, rd, val);
    }
    return 0;
}

int arm_miscellaneous(arm_core p, uint32_t ins) {
    if((get_bits(ins, 27, 23) == 2) && (get_bits(ins, 21, 20) == 0)) mrs_procedure(p, ins);
    return 0;
}
