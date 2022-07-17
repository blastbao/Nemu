#ifndef __MODRM_H__
#define __MODRM_H__

#include "common.h"
#include "cpu/decode/operand.h"

/* See i386 manual for more details about instruction format. */

// ModR/M 由 Mod，Reg/Opcode，R/M 三部分组成。
//
// Mod 是前两位，提供寄存器寻址和内存寻址，
// Reg/Opcode为3-5位，如果是Reg表示使用哪个寄存器，Opcode表示对group属性的Opcode进行补充；
// R/M为6-8位，与mod结合起来查图得8个寄存器和24个内存寻址

typedef union {
	struct {
		uint8_t R_M		:3;
		uint8_t reg		:3;
		uint8_t mod		:2;
	};
	struct {
		uint8_t dont_care	:3;
		uint8_t opcode		:3;
	};
	uint8_t val;
} ModR_M;


typedef union {
	struct {
		uint8_t base	:3;
		uint8_t index	:3;
		uint8_t ss		:2;
	};
	uint8_t val;
} SIB;

int load_addr(swaddr_t, ModR_M *, Operand *);
int read_ModR_M(swaddr_t, Operand *, Operand *);

#define MODRM_ASM_BUF_SIZE 32
extern char ModR_M_asm[];

#endif
