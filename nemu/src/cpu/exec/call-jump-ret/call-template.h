#include "cpu/exec/template-start.h"

#define instr call


// 创建 call-template.h ，根据该指令的含义实现。
// 比如该指令（CALL rel16/32）表示的意思是：
// 跳转到下一条指令的首地址加偏移量的位置，比如 mov-c.txt 中为 10000f + 6 = 100015 。
//
// 该指令需要自己编写 make_helper 函数。
make_helper(concat(call_i_, SUFFIX)) {
	// 根据 SUFFIX 进行译码，len 为取出来的参数的字节数
	int len = concat(decode_i_, SUFFIX)(cpu.eip + 1);
	// 堆栈操作
    reg_l(R_ESP) -= DATA_BYTE;
    swaddr_write(reg_l(R_ESP), 4, cpu.eip + len + 1);
    // imm 为偏移量的值
    DATA_TYPE_S imm = op_src -> val;
    // 打印指令
    print_asm("call\t%x",cpu.eip + 1 + len + imm);
    // 当前地址 eip 加上偏移量
    cpu.eip += imm;
    // 返回该 call 指令总长度，最后会在外面再加上该值，正好满足指令：eip 跳到下一条指令位置加上偏移量的位置。
    return len + 1;
} 

make_helper(concat(call_rm_, SUFFIX)){
    int len = concat(decode_rm_, SUFFIX)(cpu.eip + 1);
	reg_l(R_ESP) -= DATA_BYTE;
	swaddr_write(reg_l(R_ESP) , 4, cpu.eip + len + 1);
	DATA_TYPE_S imm = op_src -> val;
	print_asm("call %x",imm);
	cpu.eip = imm - len - 1;
	return len + 1;
}
#include "cpu/exec/template-end.h"