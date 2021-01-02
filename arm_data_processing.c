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

int get_cflag(arm_core p){
	return get_bit(arm_read_cpsr(p), 29);
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
	return (n << 31) | (z << 30) | (c << 29) | (v << 28) | cpsr_current;
}

uint32_t rotateRight(uint32_t x, uint32_t n) {
    uint32_t shifted = x >> n;
    uint32_t rot_bits = x << (32-n);
    uint32_t combined = shifted | rot_bits;

    return combined;
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
	uint64_t res = (shifter_operand << 32) | shifter_carry_out;
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

int get_carry_flag(arm_core p, int32_t number1, int32_t number2, int opcode){
	switch(opcode){
		case 5 : // ADC
			int complement = get_cflag(p);
			int64_t num1 = number1;
			int64_t num2 = number2;
			int32_t res = number1 + number2 + complement;
			int64_t res64 = num1 + num2 + complement;
			if(res == res64) return 0;
			else return 1;
			break;
		case 4 : // ADD
			int64_t num1 = number1;
			int64_t num2 = number2;
			int32_t res = number1 + number2;
			int64_t res64 = num1 + num2;
			if(res == res64) return 0;
			else return 1;
			break;
		default:
			break;
	}
	
}

int get_sub_carry_flag(int32_t number1, int32_t number2){
	if(number1 >= number2) return 0;
	else return 1;
}

int get_overflow_flag(int32_t number1, int32_t number2, int32_t res){
	int v = (~(get_bit(number1, 7)) & ~(get_bit(number2, 7)) & (get_bit(res, 7))) | ((get_bit(number1, 7)) & (get_bit(number2, 7)) & ~(get_bit(res, 7)));
	return v;
}

int32_t unsigned_to_signed(uint32_t uns) {
	int32_t signednumber = uns;
	return signednumber;
}

uint32_t adc(arm_core p, uint32_t ins){
	uint64_t packaged = get_operande_and_carry_out(p, ins);
	int32_t shifter_operand = get_shifter_operand(packaged);
	int32_t rn_data = unsigned_to_signed(arm_read_register(p, get_rn(ins)));
	int32_t rd_data;
	if(conditionPassed(p, ins)){
		rd_data = rn_data + shifter_operand + get_cflag(p);
		arm_write_register(p, get_rd(ins), rd_data);
		if(get_s_bit(ins) == 0 && get_rd(ins) == 15){
			if(arm_current_mode_has_spsr) arm_write_cpsr(p, arm_read_spsr(p));
		}else if(get_s_bit(ins) == 1){
			int n = get_bit(arm_read_register(p, get_rd(ins)), 31);
			int z = arm_read_register(p, get_rd(ins)) == 0 ? 1 : 0;
			int c = get_carry_flag(p, rn_data, shifter_operand, 5);
			int v = get_overflow_flag(rn_data, shifter_operand, rd_data);
			modify_nzcv(p, n, z, c, v);
		}
	}
	return 0;
}

uint32_t add(arm_core p, uint32_t ins){
	uint64_t packaged = get_operande_and_carry_out(p, ins);
	int32_t shifter_operand = get_shifter_operand(packaged);
	int32_t rn_data = unsigned_to_signed(arm_read_register(p, get_rn(ins)));
	int32_t rd_data;
	if(conditionPassed(p, ins)){
		rd_data = rn_data + shifter_operand;
		arm_write_register(p, get_rd(ins), rd_data);
		if(get_s_bit(ins) == 0 && get_rd(ins) == 15){
			if(arm_current_mode_has_spsr) arm_write_cpsr(p, arm_read_spsr(p));
		}else if(get_s_bit(ins) == 1){
			int n = get_bit(arm_read_register(p, get_rd(ins)), 31);
			int z = arm_read_register(p, get_rd(ins)) == 0 ? 1 : 0;
			int c = get_carry_flag(p, rn_data, shifter_operand, 4);
			int v = get_overflow_flag(rn_data, shifter_operand, rd_data);
			modify_nzcv(p, n, z, c, v);
		}
	}
	return 0;
}

/* Decoding functions for different classes of instructions */
int arm_data_processing_shift(arm_core p, uint32_t ins) {
    return UNDEFINED_INSTRUCTION;
}

int arm_data_processing_immediate_msr(arm_core p, uint32_t ins) {
    return UNDEFINED_INSTRUCTION;
}
