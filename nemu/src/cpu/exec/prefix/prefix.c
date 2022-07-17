#include "cpu/exec/helper.h"

make_helper(exec);


// 66 这个 prefix 决定了 mov 指令立即数的长度，
// 在 opcode_table 里边将其分发到函数 operand_size ，
// 该函数将全局变量的域 is_operand_size_16 置位，
// 然后再按照正常的逻辑执行 exec 函数，不过返回值加１字节。

make_helper(operand_size) {
	ops_decoded.is_operand_size_16 = true;
	int instr_len = exec(eip + 1);
	ops_decoded.is_operand_size_16 = false;
	return instr_len + 1;
}
