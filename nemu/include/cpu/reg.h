#ifndef __REG_H__
#define __REG_H__

#include "common.h"
#include "../../../lib-common/x86-inc/cpu.h"// CR0、CR3


// 在现阶段的NEMU 中通用寄存器为：
// 32位寄存器：EAX , EDX , ECX , EBX , EBP , ESI , EDI , ESP
// 16位寄存器：AX , DX , CX , BX , BP , SI , DI , SP
// 8 位寄存器：AL , DL , CL , BL , AH , DH , CH , BH
//
// 但它们在物理上并不是相互独立的, 例如 EAX 的低 16 位是 AX , 而 AX 又分成 AH 和 AL。

enum { R_EAX, R_ECX, R_EDX, R_EBX, R_ESP, R_EBP, R_ESI, R_EDI };
enum { R_AX, R_CX, R_DX, R_BX, R_SP, R_BP, R_SI, R_DI };
enum { R_AL, R_CL, R_DL, R_BL, R_AH, R_CH, R_DH, R_BH };

enum { R_ES, R_CS, R_SS, R_DS, R_FS, R_GS};

/* TODO: Re-organize the `CPU_state' structure to match the register
 * encoding scheme in i386 instruction format. For example, if we
 * access cpu.gpr[3]._16, we will get the `bx' register; if we access
 * cpu.gpr[1]._8[1], we will get the 'ch' register. Hint: Use `union'.
 * For more details about the register encoding scheme, see i386 manual.
 */


// 段寄存器
typedef struct{
	uint16_t selector;// visible
	/*invisible*/
	uint16_t attribute;//read,write,execute
	uint32_t limit;
	uint32_t base;
}Segment_Reg;


//
// CPU中存在段寄存器是因为其内存是分段访问的，这是设计之初决定的，属于基因里的东西。
// 前面已经介绍过了内存分段访问的方法，这里不再赘述。
// 
// CPU内部的段寄存器（Segment reg）如下。
// （1）CS——代码段寄存器（Code Segment Register），其值为代码段的段基值。
// （2）DS——数据段寄存器（Data Segment Register），其值为数据段的段基值。
// （3）ES——附加段寄存器（Extra Segment Register），其值为附加数据段的段基值，称为“附加”是因为此段寄存器用途不像其他sreg那样固定，可以额外做他用。
// （4）FS——附加段寄存器（Extra Segment Register），其值为附加数据段的段基值，同上，用途不固定，使用上灵活机动。
// （5）GS——附加段寄存器（Extra Segment Register），其值为附加数据段的段基值。
// （6）SS——堆栈段寄存器（Stack Segment Register），其值为堆栈段的段值。
//
//
// 32位CPU有两种不同的工作模式：实模式和保护模式。
// 
// 每种模式下，段寄存器中值的意义是不同的，但不管其为何值，在段寄存器中所表达的都是指向的段在哪里。
// 在实模式下，CS、DS、ES、SS中的值为段基址，是具体的物理地址，内存单元的逻辑地址仍为“段基值：段内偏移量”的形式。
// 在保护模式下，装入段寄存器的不再是段地址，而是“段选择子”（Selector），当然，选择子也是数值，其依然为16位宽度。
// 
// 可见，在32位CPU中，sreg无论是工作在16位的实模式，还是32位的保护模式，用的段寄存器都是同一组，
// 并且在32位下的段选择子是16位宽度，排除了段寄存器在32位环境下是32位宽的可能，综上所述，sreg都是16位宽。
//
// 

// 各个标志寄存器作用：
// SF：结果为负，即符号位为1时SF=1，否则为0.
// ZF：结果为0，ZF=1，否则为0
// CF：最高位进位，CF=1，不进位为0
// OF：溢出，OF=1
//
//
// FLAGS寄存器的状态标志(0、2、4、6、7以及11位)指示算术指令（如ADD, SUB, MUL以及DIV指令）的结果，EFLAGS寄存器中的系统标志用于控制操作系统或是执行操作，它们不允许被应用程序所修改。
// CF(bit 0) [Carry flag] ：若算术操作产生的结果在最高有效位(most-significant bit)发生进位或借位则将其置1，反之清零。这个标志指示无符号整型运算的溢出状态，这个标志同样在多倍精度运算(multiple-precision arithmetic)中使用。
// PF(bit 2) [Parity flag] ： 如果结果的最低有效字节(least-significant byte)包含偶数个1位则该位置1，否则清零。
// ZF(bit 6) [Zero flag] ： 若结果为0则将其置1，反之清零。
// SF(bit 7) [Sign flag] ：该标志被设置为有符号整型的最高有效位。(0指示结果为正，反之则为负)。
// IF(bit 9) [Interrupt enable flag] ：该标志用于控制处理器对可屏蔽中断请求(maskable interrupt requests)的响应。置1以响应可屏蔽中断，反之则禁止可屏蔽中断。
// DF标志(DF flag)：设置DF标志使得串指令自动递减（从高地址向低地址方向处理字符串），清除该标志则使得串指令自动递增。STD以及CLD指令分别用于设置以及清除DF标志。
// OF(bit 11) [Overflow flag]： 如果整型结果是较大的正数或较小的负数，并且无法匹配目的操作数时将该位置1，反之清零。这个标志为带符号整型运算指示溢出状态。



 // 31                  23                  15               7             0
 // +-------------------+-------------------+-------+-+-+-+-+-+-+-------+-+-+
 // |                                               |O| |I| |S|Z|       | |C|
 // |                       X                       | |X| |X| | |   X   |1| |
 // |                                               |F| |F| |F|F|       | |F|
 // +-------------------+-------------------+-------+-+-+-+-+-+-+-------+-+-+
typedef struct {
	union {

		union {
			uint32_t _32;
			uint16_t _16;
			uint8_t _8[2];
		} gpr[8];

		struct {
			// 通用寄存器
			uint32_t eax, ecx, edx, ebx, esp, ebp, esi, edi;
			// 标记寄存器
			union{
				struct{
					uint32_t CF:	1;
					uint32_t :		1;	//无名位域
					uint32_t PF:	1;
					uint32_t :		1;	//无名位域
					uint32_t AF:	1;
					uint32_t :		1;	//无名位域
					uint32_t ZF:	1;
					uint32_t SF:	1;
					uint32_t TF:	1;
					uint32_t IF:	1;
					uint32_t DF:	1;
					uint32_t OF:	1;
					uint32_t IOPL:	2;
					uint32_t NT:	1;
					uint32_t :		1;	//无名位域
					uint32_t RF:	1;
					uint32_t VM:	1;
					uint32_t :		14;	//无名位域
				};
s				uint32_t EFLAGS;		//用于赋初值，初始值为 0x2 。
			};
		};
	};

	/* Do NOT change the order of the GPRs' definitions. */
	

	// 指令寄存器
	swaddr_t eip;


	struct GDTR{
		uint32_t base;
		uint16_t limit;
	}gdtr;




	// i386 CPU 实现内存管理的基本思路是通过页目录和页表两极映射实现从线性地址到物理地址的转换。
	// 
	// 原因：
	// 4GB的线性地址空间，如果我们采用一级映射，页大小是4K，那么需要的页表项数量为4G/4K=1M；
	// 另外，一个页表项的大小是8B，如此一个进程的页表需要的存储空间位8M。
	// 实际情况下，可能线性地址空间仅仅某一部分有效（例如0x00000000～0x0000ffff），
	// 此时如果采用二级页表，可以避免一些无用线性地址的映射（如果相应的线性地址无效，那么对应的页目录项设置为空）。
	//
	// 其中寄存器 cr3 用于存放当前进程正在使用的页目录基地址。

	// 控制寄存器（CR0～CR3）用于控制和确定处理器的操作模式以及当前执行任务的特性，
	// CR0 中含有控制处理器操作模式和状态的系统控制标志；
	// CR1保留不用；
	// CR2含有导致页错误的线性地址；
	// CR3中含有页目录表物理内存基地址，因此该寄存器也被称为页目录基地址寄存器 PDBR（Page-Directory Base address Register）。

	// CR0 中的保护控制位：
	// （1）PE：CR0的位0是启用保护（Protection Enable）标志。当设置该位时即开启了保护模式；当复位时即进入实地址模式。这个标志仅开启段级保护，而并没有启用分页机制。若要启用分页机制，那么PE和PG标志都要置位。
	// （2）PG：CR0的位31是分页（Paging）标志。当设置该位时即开启了分页机制；当复位时则禁止分页机制，此时所有线性地址等同于物理地址。在开启这个标志之前必须已经或者同时开启PE标志。即若要启用分页机制，那么PE和PG标志都要置位。
	// （3）WP：对于Intel 80486或以上的CPU，CR0的位16是写保护（Write Proctect）标志。当设置该标志时，处理器会禁止超级用户程序（例如特权级0的程序）向用户级只读页面执行写操作；当该位复位时则反之。该标志有利于UNIX类操作系统在创建进程时实现写时复制（Copy on Write）技术。
	// 当改变PE和PG位时，必须小心。只有当执行程序至少有部分代码和数据在线性地址空间和物理地址空间中具有相同地址时，我们才能改变PG位的设置。此时这部分具有相同地址的代码在分页和未分页世界之间起着桥梁的作用。无论是否开启分页机制，这部分代码都具有相同的地址。另外，在开启分页（PG=1）之前必须先刷新页高速缓冲TLB。
	// 在修改该了PE位之后程序必须立刻使用一条跳转指令，以刷新处理器执行管道中已经获取的不同模式下的任何指令。在设置PE位之前，程序必须初始化几个系统段和控制寄存器。在系统刚上电时，处理器被复位成PE=0和PG=0（即实模式状态），以允许引导代码在启用分段和分页机制之前能够初始化这些寄存器和数据结构。


	// CR2和CR3用于分页机制。
	//
	// CR3含有存放页目录表页面的物理地址（注意，是物理地址！！！），因此CR3也被称为PDBR。
	//
	// 因为页目录表页面是页对齐的，所以该寄存器只有高 20 位是有效的。
	// 而低12位保留供更高级处理器使用，因此在往 CR3 中加载一个新值时低 12 位必须设置为0。
	// 
	// 使用MOV指令加载CR3时具有让页高速缓冲无效的副作用。
	// 为了减少地址转换所要求的总线周期数量，最近访问的页目录和页表会被存放在处理器的页高速缓冲器件中，
	// 该缓冲器件被称为转换查找缓冲区（Translation Lookaside Buffer，TLB）。
	// 只有当TLB中不包含要求的页表项时才会使用额外的总线周期从内存中读取页表项。
	// 
	// 即使CR0中的PG位处于复位状态（PG=0），我们也能先加载CR3，以允许对分页机制进行初始化。
	// 当切换任务时，CR3的内容也会随之改变。但是如果新任务的CR3值与原任务的一样，处理器就无需刷新页高速缓冲。
	// 这样共享页表的任务可以执行得更快。
	//
	// CR2用于出现页异常时报告出错信息。在报告页异常时，处理器会把引起异常的线性地址存放在CR2中。
	// 因此操作系统中的页异常处理程序可以通过检查CR2的内容来确定线性地址空间中哪一个页面引发了异常。
	// 

	CR0 cr0;

	// 段寄存器，共 6 个
	union{
		struct{
			Segment_Reg sreg[6];
		};
		struct {
			Segment_Reg es, cs, ss, ds, fs, gs;
		};
	};

	CR3 cr3;

} CPU_state;


typedef struct{
	union{
		struct{
			uint16_t limit1;
			uint16_t base1;
		};
		uint32_t part1;
	};
	union{
		struct{
			uint32_t base2:		8;
			uint32_t a:			1;
			uint32_t type:		3;
			uint32_t s:			1;
			uint32_t dpl:		2;
			uint32_t p:			1;
			uint32_t limit2:	4;
			uint32_t avl:		1;
			uint32_t :			1;
			uint32_t x:			1;
			uint32_t g:			1;
			uint32_t base3:		8;
		};
		uint32_t part2;
	};
}Sreg_Descriptor;

typedef struct {
	union {
		struct {
			uint32_t p 	:1;
			uint32_t rw	:1;
			uint32_t us	:1;
			uint32_t 	:2;
			uint32_t a	:1;
			uint32_t d 	:1;
			uint32_t 	:2;
			uint32_t avail	:3;
			uint32_t addr 	:20;
		};
		uint32_t val;
	};
}Page_Descriptor;


Sreg_Descriptor *sreg_desc;


extern CPU_state cpu;
uint8_t current_sreg;
void sreg_load(uint8_t);

static inline int check_reg_index(int index) {
	assert(index >= 0 && index < 8);
	return index;
}

// reg_l, reg_w,reg_b 是三个宏，分别对应 32 位、16 位、8 位寄存器。
#define reg_l(index) (cpu.gpr[check_reg_index(index)]._32)
#define reg_w(index) (cpu.gpr[check_reg_index(index)]._16)
#define reg_b(index) (cpu.gpr[check_reg_index(index) & 0x3]._8[index >> 2])

extern const char* regsl[];
extern const char* regsw[];
extern const char* regsb[];

#endif
