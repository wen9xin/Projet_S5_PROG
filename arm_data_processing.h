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
#ifndef __ARM_DATA_PROCESSING_H__
#define __ARM_DATA_PROCESSING_H__
#include <stdint.h>
#include "arm_core.h"


int get_cflag(arm_core p);
int get_rn(uint32_t ins);
int get_rd(uint32_t ins);
int get_s_bit(uint32_t ins);
void modify_nzcv(arm_core p, int n, int z, int c, int v);
uint32_t arithmetic_shift_right(uint32_t x, uint32_t n);

/*  
	Functions for Addressing mode 1
	return an 64 bit unsigned int where:
 	first 32 bits is shifter_operand,
 	second 32 bits is shifter_carry_out.
*/
uint64_t packing_shifter(uint32_t shifter_operand, uint32_t shifter_carry_out);
int32_t get_shifter_operand(uint64_t res);
uint32_t get_shifter_carry_out(uint64_t res);

// Data-processing operands - Immediate
uint64_t imm(arm_core p, uint32_t ins);

// Data-processing operands - Register
uint64_t reg(arm_core p, uint32_t ins);

// Data-processing operands - Logical shift left by immediate
uint64_t lsl_imm(arm_core p, uint32_t ins);

// Data-processing operands - Logical shift left by register
uint64_t lsl_reg(arm_core p, uint32_t ins);

// Data-processing operands - Logical shift right by immediate
uint64_t lsr_imm(arm_core p, uint32_t ins);

// Data-processing operands - Logical shift right by register
uint64_t lsr_reg(arm_core p, uint32_t ins);

// Data-processing operands - Arithmetic shift right by immediate
uint64_t asr_imm(arm_core p, uint32_t ins);

// Data-processing operands - Arithmetic shift right by register
uint64_t asr_reg(arm_core p, uint32_t ins);

// Data-processing operands - Rotate right with extend
uint64_t rrx(arm_core p, uint32_t ins);

// Data-processing operands - Rotate right by immediate
uint64_t ror_imm(arm_core p, uint32_t ins);

// Data-processing operands - Rotate right by register
uint64_t ror_reg(arm_core p, uint32_t ins);

uint64_t get_operande_and_carry_out(arm_core p, uint32_t ins);
int get_opcode(uint32_t ins);
int get_carry(arm_core p, int32_t number1, int32_t number2, int condition);
int get_borrow(arm_core p, int32_t number1, int32_t number2, int condition);
int get_overflow_flag(int32_t number1, int32_t number2, int32_t res);
int32_t unsigned_to_signed(uint32_t uns);

int isAdds(uint32_t ins);
int isSubs(uint32_t ins);
int isBitwises(uint32_t ins);
int isTest(uint32_t ins);
int isComp(uint32_t ins);

uint32_t calcs_procedure(arm_core p, uint32_t ins);
uint32_t bitwise_procedure(arm_core p, uint32_t ins);
uint32_t cmptst_procedure(arm_core p, uint32_t ins);

/* Decoding functions for different classes of instructions */
int arm_data_processing(arm_core p, uint32_t ins);


#endif
