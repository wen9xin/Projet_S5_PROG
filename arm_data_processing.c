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
#include "arm_data_processing.h"
#include "arm_exception.h"
#include "arm_constants.h"
#include "arm_branch_other.h"
#include "util.h"
#include "debug.h"

int get_cflag(arm_core p){
	uint32_t cpsr = arm_read_cpsr(p);
	return (cpsr >> 29) & 1;
}

int get_rn(uint32_t ins){
	return get_bits(ins, 19, 16);
}

int get_rd(uint32_t ins){
	return get_bits(ins, 15, 12);
}

int get_s_bit(uint32_t ins){
	return get_bit(ins, 20);
}

void modify_nzcv(arm_core p, int n, int z, int c, int v){
	uint32_t cpsr_current = arm_read_cpsr(p) & 0xFFFFFFF;
	uint32_t new_cpsr = (n << 31) | (z << 30) | (c << 29) | (v << 28) | cpsr_current;
	arm_write_cpsr(p, new_cpsr);
}

uint32_t arithmetic_shift_right(uint32_t x, uint32_t n){
	uint32_t res = 0;
	if(get_bit(x, 31) == 0) res = x >> n;
	else{
		uint32_t mask = 0xFFFFFFFF;
		mask = ~(mask >> n);
		res = (x >> n) & mask;
	}
	return res;
}

/*  
	Functions for Addressing mode 1
	return an 64 bit unsigned int where:
 	first 32 bits is shifter_operand,
 	second 32 bits is shifter_carry_out.
*/

uint64_t packing_shifter(uint32_t shifter_operand, uint32_t shifter_carry_out){
	uint64_t so = shifter_operand;
	uint64_t res = (so << 32) | shifter_carry_out;
	return res;
}

int32_t get_shifter_operand(uint64_t res){
	return (res >> 32) & 0xFFFFFFFF;
}

uint32_t get_shifter_carry_out(uint64_t res){
	return res & 0xFFFFFFFF;
}

// Data-processing operands - Immediate
uint64_t imm(arm_core p, uint32_t ins){
	uint32_t immed_8 = get_bits(ins, 7, 0);
	uint32_t rotate_imm = get_bits(ins, 11, 8);
	int32_t shifter_operand = rotateRight(immed_8, rotate_imm);
	uint32_t shifter_carry_out = 0;
	if(rotate_imm == 0) shifter_carry_out = get_cflag(p);
	else shifter_carry_out = get_bit(shifter_operand, 31);
	uint64_t res = packing_shifter(shifter_operand, shifter_carry_out);
	return res;
}

// Data-processing operands - Register
uint64_t reg(arm_core p, uint32_t ins){
	uint32_t shifter_operand = arm_read_register(p, get_bits(ins, 3, 0));
	uint32_t shifter_carry_out = get_cflag(p);
	uint64_t res = packing_shifter(shifter_operand, shifter_carry_out);
	return res;
}
// Data-processing operands - Logical shift left by immediate
uint64_t lsl_imm(arm_core p, uint32_t ins){
	uint32_t rm_data = arm_read_register(p, get_bits(ins, 3, 0));
	uint8_t shift_imm = get_bits(ins, 11, 7);
	uint32_t shifter_operand = 0;
	uint32_t shifter_carry_out = 0;
	if(shift_imm == 0){
		shifter_operand = rm_data;
		shifter_carry_out = get_cflag(p);
	}else{
		shifter_operand = rm_data << shift_imm;
		shifter_carry_out = get_bit(rm_data, 32-shift_imm);
	}
	uint64_t res = packing_shifter(shifter_operand, shifter_carry_out);
	return res;
}

// Data-processing operands - Logical shift left by register
uint64_t lsl_reg(arm_core p, uint32_t ins){
	uint8_t rs_7_0_data = arm_read_register(p, get_bits(ins, 11, 8)) & 0xFF;
	uint32_t rm_data = arm_read_register(p, get_bits(ins, 3, 0));
	uint32_t shifter_operand = 0;
	uint32_t shifter_carry_out = 0;
	if(rs_7_0_data == 0){
		shifter_operand = rm_data;
		shifter_carry_out = get_cflag(p);
	}else if (rs_7_0_data < 32){
		shifter_operand = rm_data << rs_7_0_data;
		shifter_carry_out = get_bit(rm_data, 32 - rs_7_0_data);
	}else if(rs_7_0_data == 32){
		shifter_operand = 0;
		shifter_carry_out = get_bit(rm_data, 0);
	}else{
		shifter_operand = 0;
		shifter_carry_out = 0;
	}
	uint64_t res = packing_shifter(shifter_operand, shifter_carry_out);
	return res;
}

// Data-processing operands - Logical shift right by register
uint64_t lsr_reg(arm_core p, uint32_t ins){
	uint8_t rs_7_0_data = arm_read_register(p, get_bits(ins, 11, 8)) & 0xFF;
	uint32_t rm_data = arm_read_register(p, get_bits(ins, 3, 0));
	uint32_t shifter_operand = 0;
	uint32_t shifter_carry_out = 0;
	if(rs_7_0_data == 0){
		shifter_operand = rm_data;
		shifter_carry_out = get_cflag(p);
	}else if (rs_7_0_data < 32){
		shifter_operand = rm_data >> rs_7_0_data;
		shifter_carry_out = get_bit(rm_data, rs_7_0_data - 1);
	}else if(rs_7_0_data == 32){
		shifter_operand = 0;
		shifter_carry_out = get_bit(rm_data, 31);
	}else{
		shifter_operand = 0;
		shifter_carry_out = 0;
	}
	uint64_t res = packing_shifter(shifter_operand, shifter_carry_out);
	return res;
}

// Data-processing operands - Logical shift right by immediate
uint64_t lsr_imm(arm_core p, uint32_t ins){
	uint32_t rm_data = arm_read_register(p, get_bits(ins, 3, 0));
	uint8_t shift_imm = get_bits(ins, 11, 7);
	uint32_t shifter_operand = 0;
	uint32_t shifter_carry_out = 0;
	if(shift_imm == 0){
		shifter_operand = 0;
		shifter_carry_out = get_bit(rm_data, 31);
	}else{
		shifter_operand = rm_data >> shift_imm;
		shifter_carry_out = get_bit(rm_data, shift_imm - 1);
	}
	uint64_t res = packing_shifter(shifter_operand, shifter_carry_out);
	return res;
}


// Data-processing operands - Arithmetic shift right by immediate
uint64_t asr_imm(arm_core p, uint32_t ins){
	uint32_t rm_data = arm_read_register(p, get_bits(ins, 3, 0));
	uint8_t shift_imm = get_bits(ins, 11, 7);
	uint32_t shifter_operand = 0;
	uint32_t shifter_carry_out = 0;
	if(shift_imm == 0){
		if(get_bit(rm_data, 31) == 0) shifter_operand = 0;
		else shifter_operand = 0xFFFFFFFF;
		shifter_carry_out = get_bit(rm_data, 31);	
	}else{		shifter_operand = arithmetic_shift_right(rm_data, shift_imm);
		shifter_carry_out = get_bit(rm_data, shift_imm - 1);
	}
	uint64_t res = packing_shifter(shifter_operand, shifter_carry_out);
	return res;
}

// Data-processing operands - Arithmetic shift right by register
uint64_t asr_reg(arm_core p, uint32_t ins){
	uint8_t rs_7_0_data = arm_read_register(p, get_bits(ins, 11, 8)) & 0xFF;
	uint32_t rm_data = arm_read_register(p, get_bits(ins, 3, 0));
	uint32_t shifter_operand = 0;
	uint32_t shifter_carry_out = 0;
	if(rs_7_0_data == 0){
		shifter_operand = rm_data;
		shifter_carry_out = get_cflag(p);
	}else if (rs_7_0_data < 32){
		shifter_operand = arithmetic_shift_right(rm_data, rs_7_0_data);
		shifter_carry_out = get_bit(rm_data, rs_7_0_data - 1);
	}else if(rs_7_0_data == 32){
		shifter_operand = 0;
		shifter_carry_out = get_bit(rm_data, 31);
	}else{
		shifter_operand = 0xFFFFFFFF;
		shifter_carry_out = 0;
	}
	uint64_t res = packing_shifter(shifter_operand, shifter_carry_out);
	return res;
}

// Data-processing operands - Rotate right with extend
uint64_t rrx(arm_core p, uint32_t ins){
	uint32_t rm_data = arm_read_register(p, get_bits(ins, 3, 0));
	uint32_t shifter_operand = (get_cflag(p) << 31) | (rm_data > 1);
	uint32_t shifter_carry_out = get_bit(rm_data, 0);
	uint64_t res = packing_shifter(shifter_operand, shifter_carry_out);
	return res;
}

// Data-processing operands - Rotate right by immediate
uint64_t ror_imm(arm_core p, uint32_t ins){
	uint32_t rm_data = arm_read_register(p, get_bits(ins, 3, 0));
	uint8_t shift_imm = get_bits(ins, 11, 7);
	uint32_t shifter_operand = 0;
	uint32_t shifter_carry_out = 0;
	if(shift_imm == 0) return rrx(p, ins);	
	else{
		shifter_operand = rotateRight(rm_data, shift_imm);
		shifter_carry_out = get_bit(rm_data, shift_imm - 1);
	}
	uint64_t res = packing_shifter(shifter_operand, shifter_carry_out);
	return res;
}

// Data-processing operands - Rotate right by register
uint64_t ror_reg(arm_core p, uint32_t ins){
	uint8_t rs_7_0_data = arm_read_register(p, get_bits(ins, 11, 8)) & 0xFF;
	uint8_t rs_4_0_data = get_bits(rs_7_0_data, 4, 0);
	uint32_t rm_data = arm_read_register(p, get_bits(ins, 3, 0));
	uint32_t shifter_operand = 0;
	uint32_t shifter_carry_out = 0;
	if(rs_7_0_data == 0){
		shifter_operand = rm_data;
		shifter_carry_out = get_cflag(p);
	}else if (rs_4_0_data == 0){
		shifter_operand = rm_data;
		shifter_carry_out = get_bit(rm_data, 31);
	}else{
		shifter_operand = rotateRight(rm_data, rs_4_0_data);
		shifter_carry_out = get_bit(rm_data, rs_4_0_data - 1);
	}
	uint64_t res = packing_shifter(shifter_operand, shifter_carry_out);
	return res;
}

uint64_t get_operande_and_carry_out(arm_core p, uint32_t ins) {
	uint64_t res = 0;
	if(get_bits(ins, 27, 25) == 1) res = imm(p, ins);
	else if(get_bits(ins, 27, 25) == 0){
		if(get_bit(ins, 4) == 0){	// Immediate shifts / RRX
			if(get_bits(ins, 6, 4) == 0) res = lsl_imm(p, ins);
			else if(get_bits(ins, 6, 4) == 2) res = lsr_imm(p, ins);
			else if(get_bits(ins, 6, 4) == 4) res = asr_imm(p, ins);
			else if(get_bits(ins, 6, 4) == 6) res = ror_imm(p, ins);
			else if(get_bits(ins, 11, 4) == 6) res = rrx(p, ins);
			else return 0;

		}else{	// register shifts
			if(get_bits(ins, 11, 4) == 0) res = reg(p, ins);
			else if(get_bits(ins, 7, 4) == 1) res = lsl_reg(p, ins);
			else if(get_bits(ins, 7, 4) == 3) res = lsr_reg(p, ins);
			else if(get_bits(ins, 7, 4) == 5) res = asr_reg(p, ins);
			else if(get_bits(ins, 7, 4) == 7) res = ror_reg(p, ins);
			else return 0;
		}
	}
	return res;
}

int get_opcode(uint32_t ins){
	return get_bits(ins, 24, 21);
}

int get_carry(arm_core p, int32_t number1, int32_t number2, int condition){
	if(condition == 1){ // ADC
		int complement = get_cflag(p);
		int64_t num1 = number1;
		int64_t num2 = number2;
		int32_t res = number1 + number2 + complement;
		int64_t res64 = num1 + num2 + complement;
		if(res == res64) return 0;
		else return 1;
	} else if(condition == 2){ // ADD & CMN
		int64_t num1 = number1;
		int64_t num2 = number2;
		int32_t res = number1 + number2;
		int64_t res64 = num1 + num2;
		if(res == res64) return 0;
		else return 1;
	}
	return 1;
}

int get_borrow(arm_core p, int32_t number1, int32_t number2, int condition){
	if(condition == 1){ // SUB or RSB
		if(number1 >= number2) return 0;
		else return 1;
	}else if(condition == 2){
		int borrow = get_cflag(p);
		int32_t res = number1 - number2 - ~borrow;
		if(res >= 0) return 0;
		else return 1;
	}
	return 1;	
}

int get_overflow_flag(int32_t number1, int32_t number2, int32_t res){
	int v = (~(get_bit(number1, 7)) & ~(get_bit(number2, 7)) & (get_bit(res, 7))) | ((get_bit(number1, 7)) & (get_bit(number2, 7)) & ~(get_bit(res, 7)));
	return v;
}

int32_t unsigned_to_signed(uint32_t uns) {
	int32_t signednumber = uns;
	return signednumber;
}

int isAdds(uint32_t ins){
	return get_opcode(ins) == 4 || get_opcode(ins) == 5;
}

int isSubs(uint32_t ins){
	return get_opcode(ins) == 2 || get_opcode(ins) == 6 || get_opcode(ins) == 3 || get_opcode(ins) == 7;
}

int isBitwises(uint32_t ins){
	return get_opcode(ins) == 0 || get_opcode(ins) == 14 || get_opcode(ins) == 1 || get_opcode(ins) == 12 || get_opcode(ins) == 15 || get_opcode(ins) == 13;
}

int isTest(uint32_t ins){
	return get_opcode(ins) == 9 || get_opcode(ins) == 8;
}

int isComp(uint32_t ins){
	return (get_opcode(ins) == 11 || get_opcode(ins) == 10) && get_bit(ins, 20) == 1;
}

uint32_t calcs_procedure(arm_core p, uint32_t ins){
	uint64_t packaged = get_operande_and_carry_out(p, ins);
	int32_t shifter_operand = get_shifter_operand(packaged);
	int32_t rn_data = unsigned_to_signed(arm_read_register(p, get_rn(ins)));
	int32_t rd_data;
	if(conditionPassed(p, ins)){
		if(get_opcode(ins) == 4) rd_data = rn_data + shifter_operand; // ADD
		if(get_opcode(ins) == 5) rd_data = rn_data + shifter_operand + get_cflag(p);// ADC
		if(get_opcode(ins) == 2) rd_data = rn_data - shifter_operand; // SUB
		if(get_opcode(ins) == 6) rd_data = rn_data - shifter_operand - (~get_cflag(p)); // SBC
		if(get_opcode(ins) == 3) rd_data = shifter_operand - rn_data; // RSB
		if(get_opcode(ins) == 7) rd_data = shifter_operand - rn_data - (~get_cflag(p)); // RSC
		arm_write_register(p, get_rd(ins), rd_data);
		if(get_s_bit(ins) == 1 && get_rd(ins) == 15){
			if(arm_current_mode_has_spsr(p)) arm_write_cpsr(p, arm_read_spsr(p));
		}else if(get_s_bit(ins) == 1){
			int n = get_bit(arm_read_register(p, get_rd(ins)), 31);
			int z = arm_read_register(p, get_rd(ins)) == 0 ? 1 : 0;
			int c = isAdds(ins) ? get_carry(p, rn_data, shifter_operand, 2) : ~get_borrow(p, shifter_operand, rn_data, 1);
			int v = get_overflow_flag(rn_data, shifter_operand, rd_data);
			modify_nzcv(p, n, z, c, v);
		}
	}
	return 0;
}

uint32_t bitwise_procedure(arm_core p, uint32_t ins){
	uint64_t packaged = get_operande_and_carry_out(p, ins);
	int32_t shifter_operand = get_shifter_operand(packaged);
	int32_t rn_data = unsigned_to_signed(arm_read_register(p, get_rn(ins)));
	int32_t rd_data;
	if(conditionPassed(p, ins)){
		if(get_opcode(ins) == 0) rd_data = rn_data & shifter_operand; // AND
		if(get_opcode(ins) == 14) rd_data = rn_data & (~shifter_operand); // BIC
		if(get_opcode(ins) == 1) rd_data = rn_data ^ shifter_operand; // EOR
		if(get_opcode(ins) == 12) rd_data = rn_data | shifter_operand; // ORR
		if(get_opcode(ins) == 15) rd_data = ~shifter_operand; // MVN
		if(get_opcode(ins) == 13) rd_data = shifter_operand; // MOV
		arm_write_register(p, get_rd(ins), rd_data);
		if(get_s_bit(ins) == 1 && get_rd(ins) == 15){
			if(arm_current_mode_has_spsr(p)) arm_write_cpsr(p, arm_read_spsr(p));
		}else if(get_s_bit(ins) == 1){
			int n = get_bit(arm_read_register(p, get_rd(ins)), 31);
			int z = arm_read_register(p, get_rd(ins)) == 0 ? 1 : 0;
			int c = get_shifter_carry_out(packaged);
			int v = (arm_read_cpsr(p) >> 28) & 1; // Unaffected
			modify_nzcv(p, n, z, c, v);
		}
	}
	return 0;
}

uint32_t cmptst_procedure(arm_core p, uint32_t ins){
	uint64_t packaged = get_operande_and_carry_out(p, ins);
	int32_t shifter_operand = get_shifter_operand(packaged);
	int32_t rn_data = unsigned_to_signed(arm_read_register(p, get_rn(ins)));
	if(conditionPassed(p, ins)){
		int alu_out = 0;
		if(get_opcode(ins) == 8) alu_out = rn_data & shifter_operand; // TST
		if(get_opcode(ins) == 9) alu_out = rn_data ^ shifter_operand; // TEQ
		if(get_opcode(ins) == 10) alu_out = rn_data - shifter_operand; // CMP
		if(get_opcode(ins) == 11) alu_out = rn_data + shifter_operand; // CMN
		int n = get_bit(alu_out, 31);
		int z = alu_out == 0 ? 1 : 0;
		int c = 0;
		if(isTest(ins)) c = get_shifter_carry_out(packaged);
		else c = (get_opcode(ins) == 10) ? get_carry(p, rn_data, shifter_operand, 2) : get_borrow(p, rn_data, shifter_operand, 1);
		int v = (arm_read_cpsr(p) >> 28) & 1; // Unaffected
		modify_nzcv(p, n, z, c, v);
	}
	return 0;
}

/* Decoding functions for different classes of instructions */
int arm_data_processing(arm_core p, uint32_t ins) {
	if(isAdds(ins) || isSubs(ins)) calcs_procedure(p, ins);
	else if(isBitwises(ins)) bitwise_procedure(p, ins);
	else if(isComp(ins) || isTest(ins)) cmptst_procedure(p, ins);
	else return UNDEFINED_INSTRUCTION;
	return 0;
}
