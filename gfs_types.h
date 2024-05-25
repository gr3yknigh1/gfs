
/*
 * FILE      gfs_types.h
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */

#ifndef GFS_TYPES_H_INCLUDED
#define GFS_TYPES_H_INCLUDED

/* First layer types. Aka base types. */

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

/* Second layer types */

typedef U8              Byte;
typedef S8              Char;
typedef U64             Size;

#ifndef bool
#define bool            _Bool
#endif // bool

#ifndef true
#define true            1
#endif  // true

#ifndef false
#define false           0
#endif  // false

typedef const S8 *      CString8;

#define BYTE_BITS       8

#define U8_MAX          255
#define U16_MAX         65535
#define U32_MAX         4294967295
#define U64_MAX         18446744073709551615

#endif // GFS_TYPES_H_INCLUDED
