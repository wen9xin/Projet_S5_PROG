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
#ifndef __ARM_LOAD_STORE_H__
#define __ARM_LOAD_STORE_H__
#include <stdint.h>
#include "arm_core.h"

// Functions for addressing mode 2
uint32_t addr_Imm_Offset(arm_core p, uint32_t ins);
uint32_t addr_Reg_Offset(arm_core p, uint32_t ins);
uint32_t addr_Sca_Offset(arm_core p, uint32_t ins);

int isImmediateOffset(uint32_t ins);
int isRegisterOffset(uint32_t ins);
int isScaledOffset(uint32_t ins);

int isNormalIndexed(uint32_t ins);
int isPreIndexed(uint32_t ins);
int isPostIndexed(uint32_t ins);

uint32_t getAddrByOffset(arm_core p, uint32_t ins);
int refreshRnByOffset(arm_core p, uint32_t ins, uint32_t addr);

// Functions for addressing mode 3
int is_Misc_Imm_Offset(uint32_t ins);
int is_Misc_Reg_Offset(uint32_t ins);
uint32_t misc_Imm_Offset(arm_core p, uint32_t ins);
uint32_t misc_Reg_Offset(arm_core p, uint32_t ins);
uint32_t getMiscAddrByOffset(arm_core p, uint32_t ins);
int refreshMiscRnByOffset(arm_core p, uint32_t ins, uint32_t addr);

// LDR : Load a 32-bit word.
int load_Word(arm_core p, uint32_t ins);
// LDRB : loads a byte from memory and zero-extends the byte to a 32-bit word
int load_Byte(arm_core p, uint32_t ins);
// LDRBT : loads a byte from memory and zero-extends the byte to a 32-bit word
int load_Byte_Trans(arm_core p, uint32_t ins);
// LDRD : loads a pair of ARM registers from two consecutive words of memory.
int load_Double_Word(arm_core p, uint32_t ins);
// LDRH : loads a halfword from memory and zero-extends it to a 32-bit word
int load_Half(arm_core p, uint32_t ins);
// LDRSB : loads a byte from memory and sign-extends the byte to a 32-bit word.
int load_Signed_Byte(arm_core p, uint32_t ins);
// LDRSH : loads a half from memory and sign-extends the byte to a 32-bit word.
int load_Signed_Half(arm_core p, uint32_t ins);
// LDRT : loads a word from memory with translation.
int load_Word_Trans(arm_core p, uint32_t ins);
// STR : Load a 32-bit word.
int store_Word(arm_core p, uint32_t ins);
// STRB : Load a 8-bit byte from the least significant byte of a register.
int store_Byte(arm_core p, uint32_t ins);
// STRBT : Load a 8-bit byte with translation.
int store_Byte_Trans(arm_core p, uint32_t ins);
// STRD : tores a pair of ARM registers to two consecutive words of memory..
int store_Double_Word(arm_core p, uint32_t ins);
// STRH stores a halfword from the least significant halfword of a register to memory.
int store_Half(arm_core p, uint32_t ins);
// STRT : loads a word from memory with translation.
int store_Word_Trans(arm_core p, uint32_t ins);


// Function for Addressing Mode 4
uint32_t nbSetBits(uint32_t ins);
uint32_t getStartAddress(arm_core p, uint32_t ins);
uint32_t getEndAddress(arm_core p, uint32_t ins);

// LDM (1) (Load Multiple) loads a non-empty subset, or possibly all, 
// of the general-purpose registers from sequential memory locations.
int ldm1(arm_core p, uint32_t ins);

// LDM(2) loads User mode registers when the processor is in a privileged mode.
int ldm2(arm_core p, uint32_t ins);

// LDM (3) loads a subset, or possibly all, 
// of the general-purpose registers and the PC from sequential memory locations.
int ldm3(arm_core p, uint32_t ins);

// STM (1) (Store Multiple) stores a non-empty subset (or possibly all) 
// of the general-purpose registers to sequential memory locations. 
int stm1(arm_core p, uint32_t ins);

// STM (2) stores a subset (or possibly all) of the User mode 
// general-purpose registers to sequential memory locations.
int stm2(arm_core p, uint32_t ins);

int arm_load_store(arm_core p, uint32_t ins);
int arm_load_store_multiple(arm_core p, uint32_t ins);
int arm_coprocessor_load_store(arm_core p, uint32_t ins);

#endif
