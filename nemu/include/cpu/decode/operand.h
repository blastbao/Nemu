#ifndef __OPERAND_H__
#define __OPERAND_H__

enum { OP_TYPE_REG, OP_TYPE_MEM, OP_TYPE_IMM };

#define OP_STR_SIZE 40


//
typedef struct {
	uint32_t type;	 		//操作数类型
	size_t size;			//操作数宽度
	union {
		uint32_t reg;		//寄存器操作数
		//swaddr_t addr;
		struct {
			swaddr_t addr;	//操作数地址
			uint8_t sreg;
		};
		uint32_t imm;		//立即数
		int32_t simm;		//有符号立即数
	};
	uint32_t val;			//存操作数的值
	char str[OP_STR_SIZE];	//操作数字符串
} Operand;


typedef struct {
	uint32_t opcode;			// 操作码
	bool is_operand_size_16;	// 
	Operand src, dest, src2;	// 源、目的、源
} Operands;

#endif
