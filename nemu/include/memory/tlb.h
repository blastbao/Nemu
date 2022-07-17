#ifndef __TLB_H__
#define __TLB_H__

#include "common.h"

#define TLB_SIZE 64


typedef struct{
    bool valid;			// 有效位
    uint32_t tag;		// 标记
    uint32_t page_num;	// 页号
}TLB;


// 页表
TLB tlb[TLB_SIZE];


// 初始化
void init_tlb();

// 根据地址读取页号
int read_tlb(uint32_t addr);

// 写入
void write_tlb(uint32_t lnaddr,uint32_t hwaddr);

#endif
 
