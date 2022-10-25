#ifndef PAGE_DEF
#define PAGE_DEF
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
typedef unsigned int uint;
typedef unsigned short ushort;
typedef unsigned char uchar;
typedef unsigned long long ull;
typedef long long ll;
typedef double db;
typedef int INT;
typedef int(cf)(uchar*, uchar*);
int current = 0;
int tt = 0;
#endif
