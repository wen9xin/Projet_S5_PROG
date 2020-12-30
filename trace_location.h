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
#ifndef __TRACE_LOCATION_H__
#define __TRACE_LOCATION_H__
#include "trace.h"

#define LOCATION trace_start_location(__FILE__, __LINE__)
#define END_LOCATION trace_end_location(__FILE__, __LINE__)

#define arm_fetch(p, ins) (LOCATION, arm_fetch(p, ins)+END_LOCATION)

#define arm_read_register(p, reg) (LOCATION, \
                                        arm_read_register(p, reg)+END_LOCATION)
#define arm_read_usr_register(p, reg) (LOCATION, \
                                    arm_read_usr_register(p, reg)+END_LOCATION)
#define arm_read_cpsr(p) (LOCATION, arm_read_cpsr(p)+END_LOCATION)
#define arm_read_spsr(p) (LOCATION, arm_read_spsr(p)+END_LOCATION)
#define arm_write_register(p, reg, val) \
                      (LOCATION, arm_write_register(p, reg, val), END_LOCATION)
#define arm_write_usr_register(p, reg, val) \
                  (LOCATION, arm_write_usr_register(p, reg, val), END_LOCATION)
#define arm_write_cpsr(p, val) (LOCATION, arm_write_cpsr(p, val), END_LOCATION)
#define arm_write_spsr(p, val) (LOCATION, arm_write_spsr(p, val), END_LOCATION)

#define arm_read_byte(p, addr, val) (LOCATION, \
                                      arm_read_byte(p, addr, val)+END_LOCATION)
#define arm_read_half(p, addr, val) (LOCATION, \
                                      arm_read_half(p, addr, val)+END_LOCATION)
#define arm_read_word(p, addr, val) (LOCATION, \
                                      arm_read_word(p, addr, val)+END_LOCATION)
#define arm_write_byte(p, addr, val) (LOCATION, \
                                     arm_write_byte(p, addr, val)+END_LOCATION)
#define arm_write_half(p, addr, val) (LOCATION, \
                                     arm_write_half(p, addr, val)+END_LOCATION)
#define arm_write_word(p, addr, val) (LOCATION, \
                                     arm_write_word(p, addr, val)+END_LOCATION)

#endif
