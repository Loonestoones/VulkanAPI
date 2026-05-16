#ifndef OGLDEV_TYPES_H
#define OGLDEV_TYPES_H

#ifndef _WIN64
#include <unistd.h>
#endif

#include <cstdint>

typedef unsigned int uint;
typedef unsigned short ushort;
typedef unsigned char uchar;

typedef uint8_t u8;
typedef int8_t i8;
typedef uint16_t u16;
typedef int16_t i16;
typedef int32_t i32;
typedef uint32_t u32;
typedef int64_t i64;
typedef uint64_t u64;

struct MemBuf {
    char* pMem = NULL;
    size_t Size = 0;
};


#endif 
