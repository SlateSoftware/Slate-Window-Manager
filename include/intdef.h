#ifndef __INTDEF_H__
#define __INTDEF_H__ 1

#include <stdint.h>
#include <stdbool.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

#ifndef true
    #define true 1
#endif
#ifndef false
    #define false 0
#endif

#endif