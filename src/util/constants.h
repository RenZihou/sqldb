// constants.h
// -*- encoding: utf-8 -*-
// @Author: RenZihou

#ifndef CONSTANTS_H_
#define CONSTANTS_H_

/* ==== hash map ==== */

#define HASH_OFFSET 2166136261  // hash init value
#define HASH_PRIME 16777619  // hash multiplier

/* ==== page file ==== */

#define PAGE_SIZE_IDX 12  // log2(PAGE_SIZE)
#define PAGE_SIZE (1 << PAGE_SIZE_IDX)  // page size in bytes
#define PAGE_SIZE_MASK (PAGE_SIZE - 1)  // page size mask
#define MAX_OPEN_FILE 128  // max open file number
#define MAX_BUF_PAGE 65536  // max page number in buffer

typedef unsigned char *BufType;

/* ==== table ==== */

#define PAGE_HEADER_SIZE 100  // data-page header size in bytes, used to store occupied info
#define MAX_COLUMN 32  // max column number in a table, must <= 8 * sizeof(unsigned)
#define MAX_COLUMN_NAME_LEN 127  // max column name length in a table
#define MAX_RECORD_SIZE 4080  // max record size in a table, must <= PAGE_SIZE - PAGE_HEADER_SIZE - sizeof(unsigned)

#define FLAG_NOT_NULL 1  // bit mask of not null flag
#define FLAG_HAS_DEFAULT 2  // bit mask of has default flag

/* === DATABASE ==== */

#define MAX_DATABASE 32  // max database number
#define MAX_DATABASE_NAME_LEN 63  // max database name length
#define GLOBAL_TABLE_NAME ".global"  // global table name (which stores db or table info)

#endif  // CONSTANTS_H_
