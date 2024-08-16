/*
 * FILE      gfs_types.h
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */

#if !defined(GFS_TYPES_H_INCLUDED)
#define GFS_TYPES_H_INCLUDED

#include "gfs_assert.h"

#define LONG_SIZE sizeof(long)
#define LONG_LONG_SIZE sizeof(long long)
#define INT_SIZE sizeof(int)
#define POINTER_SIZE sizeof(void *)

typedef signed char i8;
typedef signed short i16;
typedef signed int i32;
typedef signed long long i64;

GFS_EXPECT_TYPE_SIZE(i8, 1);
GFS_EXPECT_TYPE_SIZE(i16, 2);
GFS_EXPECT_TYPE_SIZE(i32, 4);
GFS_EXPECT_TYPE_SIZE(i64, 8);

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

GFS_EXPECT_TYPE_SIZE(u8, 1);
GFS_EXPECT_TYPE_SIZE(u16, 2);
GFS_EXPECT_TYPE_SIZE(u32, 4);
GFS_EXPECT_TYPE_SIZE(u64, 8);

typedef float f32;
typedef double f64;

GFS_EXPECT_TYPE_SIZE(f32, 4);
GFS_EXPECT_TYPE_SIZE(f64, 8);

typedef u8 byte;
typedef u64 usize;

GFS_EXPECT_TYPE_SIZE(byte, 1);
GFS_EXPECT_TYPE_SIZE(usize, POINTER_SIZE);

typedef char char8;
typedef const char *cstring8; // NOTE(ilya.a): Explicitly distiguasing C style
                              // string (null terminated).  [2024/05/26]

GFS_EXPECT_TYPE_SIZE(char8, 1);
GFS_EXPECT_TYPE_SIZE(cstring8, POINTER_SIZE);

typedef _Bool bool;

GFS_EXPECT_TYPE_SIZE(bool, 1);

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
