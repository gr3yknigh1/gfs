/*
 * FILE      gfs_types.h
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */

#if !defined(GFS_TYPES_H_INCLUDED)
#define GFS_TYPES_H_INCLUDED

typedef signed char i8;
typedef signed short i16;
typedef signed int i32;
typedef signed long i64;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long u64;

typedef float f32;
typedef double f64;

typedef u8 byte;
typedef u64 usize;

typedef char char8;
typedef const char *cstr8; // NOTE(ilya.a): Explicitly distiguasing C style
                           // string (null terminated).  [2024/05/26]

typedef _Bool bool;

#ifndef true
#define true 1
#endif // true

#ifndef false
#define false 0
#endif // false

#ifndef NULL
#define NULL ((void *)0)
#endif // NULL

#define BYTE_BITS 8

#define U8_MAX 255
#define U16_MAX 65535
#define U32_MAX 4294967295
#define U64_MAX 18446744073709551615



#endif // GFS_TYPES_H_INCLUDED
