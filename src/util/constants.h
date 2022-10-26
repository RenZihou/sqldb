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
#define PAGE_SIZE_IDX 13  // log2(PAGE_SIZE)
//#define PAGE_SIZE 8192  // page size in bytes
#define PAGE_SIZE (1 << PAGE_SIZE_IDX)  // page size in bytes
#define MAX_FMT_INT_NUM 128
//#define BUF_PAGE_NUM 65536
#define MAX_OPEN_FILE 128  // max open file number
//#define MAX_TYPE_NUM 256
/*
 * 缓存中页面个数上限
 */
#define CAP 60000

#define HASH_OFFSET 2166136261  // hash init value
#define HASH_PRIME 16777619  // hash multiplier
#define IN_DEBUG 0
#define DEBUG_DELETE 0
#define DEBUG_ERASE 1
#define DEBUG_NEXT 1
/*
 * 一个表中列的上限
 */
#define MAX_COL_NUM 31
/*
 * 数据库中表的个数上限
 */
#define MAX_TB_NUM 31
#define RELEASE 1

typedef unsigned int uint;
typedef unsigned int* BufType;

#endif  // CONSTANTS_H_
