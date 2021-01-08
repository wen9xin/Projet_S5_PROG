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

uint32_t calcBLTargetAddress(uint32_t ins){
    uint32_t res = get_bits(ins, 23, 0) | 0x00000000;
    if(get_bit(res, 23) == 1) res = res | 0xFF000000;
    return res << 2;
}

uint32_t calcBLXTargetAddress(uint32_t ins){
    uint32_t res = get_bits(ins, 23, 0) | 0x00000000;
    if(get_bit(res, 23) == 1) res = res | 0xFF000000 | get_bit_24(ins) << 1;
    return res << 2;
}

int bl_procedure(arm_core p, uint32_t ins){
    uint32_t pc = arm_read_register(p, 15);
    if(conditionPassed(p, ins)){
        if(get_bit_24(ins)) arm_write_register(p, 14, pc);
    }
    pc = pc + calcBLTargetAddress(ins);
    arm_write_register(p, 15, pc);

    return 0;
}

int blx1_procedure(arm_core p, uint32_t ins){
    if(get_bits(ins, 31, 28) == 15){
        uint32_t pc = arm_read_register(p, 15);
        arm_write_register(p, 14, pc);
        uint32_t cpsr = arm_read_cpsr(p);
        cpsr = (cpsr & 0xFFFFFFDF) | 0x20;
        arm_write_cpsr(p, cpsr);
        pc = pc + calcBLXTargetAddress(ins);
        arm_write_register(p, 15, pc);
    }
    return 0;
}

int blx2_procedure(arm_core p, uint32_t ins){
    if(conditionPassed(p, ins)){
        uint32_t rm_data = arm_read_register(p, get_bits(ins, 3, 0));
        uint32_t pc = arm_read_register(p, 15);
        arm_write_register(p, 14, pc);

        uint32_t cpsr = arm_read_cpsr(p);
        cpsr = (cpsr & 0xFFFFFFDF) | (get_bit(rm_data, 0) << 5);
        arm_write_cpsr(p, cpsr);

        pc = rm_data & 0xFFFFFFFE;
        arm_write_register(p, 15, pc);
    }
    return 0;
}

int bx_procedure(arm_core p, uint32_t ins){
    if(conditionPassed(p, ins)){
        uint32_t rm_data = arm_read_register(p, get_bits(ins, 3, 0));
        uint32_t pc = arm_read_register(p, 15);
        
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
        if(get_bits(ins, 31, 28) == 15) blx1_procedure(p, ins);
        else bl_procedure(p, ins);
    }else if(get_bits(ins, 27, 20) == 0x12){
        if(get_bits(ins, 7, 4) == 3) blx2_procedure(p, ins);
        else if(get_bits(ins, 7, 4) == 1) bx_procedure(p, ins);
        else return UNDEFINED_INSTRUCTION;
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

int arm_miscellaneous(arm_core p, uint32_t ins) {
    return UNDEFINED_INSTRUCTION;
}
