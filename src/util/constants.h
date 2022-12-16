// constants.h
// -*- encoding: utf-8 -*-
// @Author: RenZihou

#ifndef CONSTANTS_H_
#define CONSTANTS_H_

// bitmap
#define LEAF_BIT 32
#define MAX_LEVEL 5
#define MAX_INNER_NUM 67
//#define MOD 61
#define BIAS 5

// page
/*
 * 一个页面中的整数个数
 */
#define PAGE_INT_NUM 2048
/*
 * 页面字节数以2为底的指数
 */

#define HASH_OFFSET 2166136261  // hash init value
#define HASH_PRIME 16777619  // hash multiplier

#define PAGE_SIZE_IDX 12  // log2(PAGE_SIZE)
#define PAGE_SIZE (1 << PAGE_SIZE_IDX)  // page size in bytes
#define PAGE_SIZE_MASK (PAGE_SIZE - 1)  // page size mask
#define MAX_OPEN_FILE 128  // max open file number
#define MAX_BUF_PAGE 65536  // max page number in buffer

#define IN_DEBUG 0
#define DEBUG_DELETE 0
#define DEBUG_ERASE 1
#define DEBUG_NEXT 1

#define MAX_COLUMN 32  // max column number in a table, must <= 8 * sizeof(unsigned)
#define MAX_COLUMN_NAME_LEN 127  // max column name length in a table
#define MAX_RECORD_SIZE 4080  // max record size in a table, must <= PAGE_SIZE - sizeof(unsigned)

#define FLAG_NOT_NULL 1  // bit mask of not null flag
#define FLAG_HAS_DEFAULT 2  // bit mask of has default flag
/*
 * 数据库中表的个数上限
 */
#define MAX_TB_NUM 31
#define RELEASE 1

typedef char *BufType;

#endif  // CONSTANTS_H_
