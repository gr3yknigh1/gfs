/*
 * GFS
 *
 * FILE    gfs_types.hpp
 * AUTHOR  Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT Copyright (c) 2024 Ilya Akkuzin
 * */

#ifndef GFS_TYPES_HPP_INCLUDED
#define GFS_TYPES_HPP_INCLUDED

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

typedef const S8 *      CStr;

#define MAX_U8          255
#define MAX_U16         65535
#define MAX_U32         4294967295
#define MAX_U64         18446744073709551615

#endif // GFS_TYPES_HPP_INCLUDED
