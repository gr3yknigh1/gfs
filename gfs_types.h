
/*
 * FILE      gfs_types.h
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */

#ifndef GFS_TYPES_H_INCLUDED
#define GFS_TYPES_H_INCLUDED

typedef signed char     S8;
typedef signed short    S16;
typedef signed int      S32;
typedef signed long     S64;

typedef unsigned char   U8;
typedef unsigned short  U16;
typedef unsigned int    U32;
typedef unsigned long   U64;

typedef float           F32;
typedef double          F64;

typedef U8              Byte;
typedef U64             Size;
typedef void            Void;

typedef char            Char8;
typedef const char *    CStr8;   // NOTE(ilya.a): Explicitly distiguasing C style 
                                 // string (null terminated).  [2024/05/26]

#ifndef Bool
#define Bool            _Bool
#endif // Bool

#ifndef true
#define true            1
#endif  // true

#ifndef false
#define false           0
#endif  // false

#ifndef NULL
#define NULL ((void *)0)
#endif  // NULL

#define BYTE_BITS       8

#define U8_MAX          255
#define U16_MAX         65535
#define U32_MAX         4294967295
#define U64_MAX         18446744073709551615

#endif // GFS_TYPES_H_INCLUDED
