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
#include <stdlib.h>
#include "memory.h"
#include "util.h"

struct memory_data {
    uint32_t * mem_data;
    uint8_t endianess;
    size_t size;
};

memory memory_create(size_t size, int is_big_endian) {
    uint32_t * mdata = calloc(size, sizeof(uint32_t));
    memory m = malloc(size*(sizeof(uint32_t)) + sizeof(uint8_t) + sizeof(size_t));
    m->mem_data = mdata;
    m->endianess = is_big_endian;
    m->size = size;
    return m;
}

size_t memory_get_size(memory mem) {
    return mem->size;
}

void memory_destroy(memory mem) {
    free(mem);
}

int memory_read_byte(memory mem, uint32_t address, uint8_t *value) {
    *value = *(mem->mem_data + address);
    return 1;
}

// May have to verify address % 2 == 0
int memory_read_half(memory mem, uint32_t address, uint16_t *value) {
    uint8_t res1 = 0, res2 = 0;
    switch (mem->endianess){
        case 1 :
            //big endian
            memory_read_byte(mem, address, &res1);
            memory_read_byte(mem, address + 1, &res2);
            *value = (res1 << 8) + res2;
            return 1;
            break;
        default :
            //Little endian
            memory_read_byte(mem, address, &res1);
            memory_read_byte(mem, address + 1, &res2);
            *value = (res2 << 8) + res1;
            return 1;
            break;
    }
}

int memory_read_word(memory mem, uint32_t address, uint32_t *value) {
    uint8_t res1 = 0, res2 = 0, res3 = 0, res4 = 0;
    switch (mem->endianess){
        case 1 :
            //big endian
            memory_read_byte(mem, address, &res1);
            memory_read_byte(mem, address + 1, &res2);
            memory_read_byte(mem, address + 2, &res3);
            memory_read_byte(mem, address + 3, &res4);
            *value = (res1 << 24) + (res2 << 16) + (res3 << 8) + res4;
            return 1;
            break;
        default :
            //Little endian
            memory_read_byte(mem, address, &res1);
            memory_read_byte(mem, address + 1, &res2);
            memory_read_byte(mem, address + 2, &res3);
            memory_read_byte(mem, address + 3, &res4);
            *value = (res4 << 24) + (res3 << 16) + (res2 << 8) + res1;
            return 1;
            break;
    }
}

int memory_write_byte(memory mem, uint32_t address, uint8_t value) {
    *(mem->mem_data + address) = value;
    return 1;
}

int memory_write_half(memory mem, uint32_t address, uint16_t value) {
    uint8_t res1 = value & 0xFF;
    uint8_t res2 = (value >> 8) & 0xFF;
    switch (mem->endianess){
        case 1:
        //Big Endian
            memory_write_byte(mem, address, res2);
            memory_write_byte(mem, address + 1, res1);
            return 1;
            break;
        default:
        //Little Endian
            memory_write_byte(mem, address, res1);
            memory_write_byte(mem, address + 1, res2);
            return 1;
            break;
    }
}

int memory_write_word(memory mem, uint32_t address, uint32_t value) {
    uint8_t res1 = value & 0xFF;
    uint8_t res2 = (value >> 8) & 0xFF;
    uint8_t res3 = (value >> 16) & 0xFF;
    uint8_t res4 = (value >> 24) & 0xFF;
    switch (mem->endianess){
        case 1:
        //Big Endian
            memory_write_byte(mem, address, res4);
            memory_write_byte(mem, address + 1, res3);
            memory_write_byte(mem, address + 2, res2);
            memory_write_byte(mem, address + 3, res1);
            return 1;
            break;
        default:
        //Little Endian
            memory_write_byte(mem, address, res1);
            memory_write_byte(mem, address + 1, res2);
            memory_write_byte(mem, address + 2, res3);
            memory_write_byte(mem, address + 3, res4);
            return 1;
            break;
    }
}
