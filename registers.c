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
#include "registers.h"
#include "arm_constants.h"
#include <stdlib.h>

struct registers_data {
    uint32_t reg_data;
};

registers registers_create() {
    registers r = calloc(37, sizeof(uint32_t));
    return r;
}

void registers_destroy(registers r) {
    free(r);
}

uint8_t get_mode(registers r) {
    // modes:
    // usr = 16
    // fiq = 17
    // irq = 18
    // svc = 19
    // abt = 23
    // und = 27
    // sys = 31
    uint32_t cpsr = (r + 31)->reg_data;
    uint8_t mode = cpsr & 0x1f;
    return mode;
} 

int current_mode_has_spsr(registers r) {
    uint8_t mode = get_mode(r);
    switch (mode){
        case 16 :
            return 0;
            break;
        case 31 :
            return 0;
            break;
        default :
            return 1;
            break;
    }
}

int in_a_privileged_mode(registers r) {
    uint8_t mode = get_mode(r);
    switch (mode){
        case 16 :
            return 0;
            break;
        default :
            return 1;
            break;
    }
}

uint32_t read_register(registers r, uint8_t reg) {
    if(reg < 0) return 0; 
    if(reg < 8){
        return (r + reg)->reg_data;
    }
    uint8_t mode = get_mode(r);
    if(reg < 13){
        switch (mode){
            case 17 :
                return (r + 9 + (reg-8)*2) ->reg_data;
                break;
            default :
                return (r + 8 + (reg-8)*2) ->reg_data;
                break;
        }
    }if(reg < 15){
        switch (mode){
            case 17 :
                return (r + 23 + (reg-8)*2) -> reg_data;
                break;
            case 18 :
                return (r + 22 + (reg-8)*2) -> reg_data;
                break;
            case 19 :
                return (r + 19 + (reg-8)*2) -> reg_data;
                break;
            case 23 :
                return (r + 20 + (reg-8)*2) -> reg_data;
                break;
            case 27 :
                return (r + 21 + (reg-8)*2) -> reg_data;
                break;
            default :
                return (r + 18 + (reg-13)*6) -> reg_data;
                break;
        }
    }else{
        return (r + 30) -> reg_data;
    }
}

uint32_t read_usr_register(registers r, uint8_t reg) {
    if(reg < 0) return 0; 
    else if(reg < 8){
        return (r + reg)->reg_data;
    }else if(reg < 13){
        return (r + 8 + (reg-8)*2) ->reg_data;
    }else if(reg < 15){
        return (r + 18 + (reg-13)*6) -> reg_data;
    }else{
        return (r + 30) -> reg_data;
    }
}

uint32_t read_cpsr(registers r) {
    return (r + 31) -> reg_data;
}

uint32_t read_spsr(registers r) {
    return (r + 32) -> reg_data;
}

void write_register(registers r, uint8_t reg, uint32_t value) {
    if(reg < 0) return;
    uint8_t mode = get_mode(r);
    if(reg < 8){
        (r + reg) -> reg_data = value;
        return;
    }else if(reg < 13){
        switch (mode){
            case 17 :
                (r + 9 + (reg-8)*2) -> reg_data = value;
                return;
                break;
            default :
                (r + 8 + (reg-8)*2) -> reg_data = value;
                return;
                break;
        }
    }else if(reg < 15){
        switch (mode){
            case 17 :
                (r + 23 + (reg-8)*2) -> reg_data = value;
                return ;
                break;
            case 18 :
                (r + 22 + (reg-8)*2) -> reg_data = value;
                return;
                break;
            case 19 :
                (r + 19 + (reg-8)*2) -> reg_data = value;
                return;
                break;
            case 23 :
                (r + 20 + (reg-8)*2) -> reg_data = value;
                return;
                break;
            case 27 :
                (r + 21 + (reg-8)*2) -> reg_data = value;
                return;
                break;
            default :
                (r + 18 + (reg-13)*6) -> reg_data = value;
                return;
                break;
        }
    }else{
        (r + 30) -> reg_data = value;
        return;
    }
}

void write_usr_register(registers r, uint8_t reg, uint32_t value) {
    if(reg < 0) return;
    uint8_t mode = get_mode(r);
    if(reg < 8){
        (r + reg) -> reg_data = value;
    }else if(reg < 13){
        (r + 8 + (reg-8)*2) -> reg_data = value;
    }else if(reg < 15){
        (r + 18 + (reg-13)*6) -> reg_data = value;      
    }else{
        (r + 30) -> reg_data = value;
    }
}

void write_cpsr(registers r, uint32_t value) {
    (r + 31) -> reg_data = value;
}

void write_spsr(registers r, uint32_t value) {
    (r + 32) -> reg_data = value;
}
