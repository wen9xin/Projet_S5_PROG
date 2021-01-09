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
#include "arm_instruction.h"
#include "arm_exception.h"
#include "arm_data_processing.h"
#include "arm_load_store.h"
#include "arm_branch_other.h"
#include "arm_constants.h"
#include "util.h"

static int arm_execute_instruction(arm_core p) {
    uint32_t ins;
	uint32_t insType;
	arm_fetch(p, &ins);

	insType = get_bits(ins, 27, 25);

    switch(insType){
        case 0:
            if(get_bit(ins, 7) && get_bit(ins, 4)){
                // Extra Load/Store
                printf("case 0, lsExtra\n");
                arm_load_store_extra(p, ins);
            }else if(get_bits(ins, 24, 23) == 2 && get_bit(ins, 4) == 0){
                // Miscellaneous instructions part 1
                printf("case 0, Misc - MRS\n");
                arm_miscellaneous(p, ins);
            }else if (get_bits(ins, 24, 23) == 2 && get_bit(ins, 4) == 1 && get_bit(ins, 7) == 0){
                // Miscellaneous instructions part 2
                printf("case 0, branchMisc\n");
                arm_branch_misc(p, ins);
            }else {
                // Data processing
                printf("case 0, dataProcessing\n");
                arm_data_processing(p, ins);
            }
            break;

        case 1:
            printf("case 1, dataProcessing\n");
            arm_data_processing(p, ins);
            break;

        case 4: // Load Store Multiple
            printf("case 4, lsMultiple\n");
            arm_load_store_multiple(p, ins);
            break;

        case 5: // Branch & Branche with link
            printf("case 5, branch\n");
            arm_branch(p, ins);
            break;
        
        case 6: // Not implemented
            printf("case 6, nothing done\n");
            break;
        
        case 7 : // Software interrupt (swi)
            printf("case 7, swi\n");
			arm_coprocessor_others_swi(p, ins);
			break;
        
        default: // case 2 or 3 : Load Store
            printf("case 2/3, ls\n");
            arm_load_store(p, ins);
            break;
    }
    return 0;
}

int arm_step(arm_core p) {
    int result;

    result = arm_execute_instruction(p);
    if (result)
        arm_exception(p, result);
    return result;
}
