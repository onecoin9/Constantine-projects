#pragma once
/******************************************************************************
* Copyright (C) 2004 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/

/* Note: This file depends on the following files having been included prior to self being included.
   1. portab.h
*/

#ifndef BL_SREC_H
#define BL_SREC_H

#define SREC_MAX_BYTES        255  /* Maximum record length */
#define SREC_DATA_MAX_BYTES   123  /* Maximum of 123 data bytes */

#define SREC_TYPE_0  0
#define SREC_TYPE_1  1
#define SREC_TYPE_2  2
#define SREC_TYPE_3  3
#define SREC_TYPE_5  5
#define SREC_TYPE_7  7
#define SREC_TYPE_8  8
#define SREC_TYPE_9  9

#include "portab.h"
#include "errors.h"
#include <stdio.h>
#include <stdlib.h>
typedef struct srec_info_s {
	int8    type;
	uint8*  addr;
	uint8*  sr_data;
	uint8   dlen;
} srec_info_t;

void initializeSrecInfo(srec_info_t* info) {
	info->type = 0;

	// 分配内存给 addr，并设置为有效的地址
	info->addr = (uint8*)malloc(4 * sizeof(uint8));
	if (info->addr != NULL) {
		//*(info->addr) = 42; // 设置为任意的初始值，这里设置为 42
		memset((uint8*)info->addr, 0, 4 * sizeof(uint8));
	}
	else {
		// 处理内存分配失败的情况
		fprintf(stderr, "Memory allocation failed for addr\n");
		exit(EXIT_FAILURE);
	}

	// 分配内存给 sr_data，并设置为有效的数据
	info->sr_data = (uint8*)malloc(16 * sizeof(uint8));
	if (info->sr_data != NULL) {
		//*(info->sr_data) = 255; // 设置为任意的初始值，这里设置为 255
		memset((uint8*)info->sr_data, 0, 16 * sizeof(uint8));
	}
	else {
		// 处理内存分配失败的情况
		fprintf(stderr, "Memory allocation failed for sr_data\n");
		// 在这里可以考虑释放之前分配的内存，然后退出程序
		free(info->addr);
		exit(EXIT_FAILURE);
	}

	info->dlen = 8 * sizeof(uint8); // 设置数据长度为 1，根据实际需求设置
}

// 释放内存的函数
void freeSrecInfo(srec_info_t* info) {
	//free(info->addr);
	free(info->sr_data);
}

#ifdef __cplusplus
extern "C" {
#endif

	uint8   decode_srec_line(uint8* sr_buf, srec_info_t* info);
	uint8  grab_hex_byte(uint8* buf);
	uint16 grab_hex_word(uint8* buf);
	uint32 grab_hex_dword(uint8* buf);
	uint32 grab_hex_word24(uint8* buf);

#ifdef __cplusplus
}
#endif

//uint8   decode_srec_line (uint8 *sr_buf, srec_info_t *info);
//uint8  grab_hex_byte (uint8 *buf);
//uint16 grab_hex_word (uint8 *buf);
//uint32 grab_hex_dword (uint8 *buf);
//uint32 grab_hex_word24 (uint8 *buf);


#endif /* BL_SREC_H */
