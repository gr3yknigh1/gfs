#if !defined(GFS_TYPES_H_INCLUDED)
/*
 * FILE      gfs_types.h
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */

#define GFS_TYPES_H_INCLUDED

#include "gfs/static_assert.h"

#if defined(WIN32)
#define _LONG_64 long long
#else
#define _LONG_64 long
#endif

#define POINTER_SIZE 8

typedef signed char i8;
typedef signed short i16;
typedef signed int i32;
typedef signed _LONG_64 i64;

EXPECT_TYPE_SIZE(i8, 1);
EXPECT_TYPE_SIZE(i16, 2);
EXPECT_TYPE_SIZE(i32, 4);
EXPECT_TYPE_SIZE(i64, 8);

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned _LONG_64 u64;

EXPECT_TYPE_SIZE(u8, 1);
EXPECT_TYPE_SIZE(u16, 2);
EXPECT_TYPE_SIZE(u32, 4);
EXPECT_TYPE_SIZE(u64, 8);

typedef float f32;
typedef double f64;

EXPECT_TYPE_SIZE(f32, 4);
EXPECT_TYPE_SIZE(f64, 8);

typedef u8 byte;
typedef u64 usize;

EXPECT_TYPE_SIZE(byte, 1);
EXPECT_TYPE_SIZE(usize, POINTER_SIZE);

typedef char char8;
typedef const char8 *cstring8; // NOTE(ilya.a): Explicitly distiguasing C style
                              // string (null terminated).  [2024/05/26]

EXPECT_TYPE_SIZE(char8, 1);
EXPECT_TYPE_SIZE(cstring8, POINTER_SIZE);

#if !defined(__cplusplus)

// TODO(gr3yknigh1): Typedef to _Bool if C11
#if !defined(bool)
typedef i8 bool;
#endif

#if !defined(true)
#define true 1
#endif // true

#if !defined(false)
#define false 0
#endif // false

#endif

EXPECT_TYPE_SIZE(bool, 1);

#ifndef NULL
#define NULL ((void *)0)
#endif // NULL

#define BYTE_BITS 8

#endif // GFS_TYPES_H_INCLUDED
