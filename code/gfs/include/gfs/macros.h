#if !defined(GFS_MACROS_H_INCLUDED)
/*
 * FILE      gfs_macros.h
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */
#define GFS_MACROS_H_INCLUDED

#if defined(__cplusplus)
#define CLINKAGE extern "C"
#define CLINKAGE_BEGIN extern "C" {
#define CLINKAGE_END }
#else
#define CLINKAGE
#define CLINKAGE_BEGIN
#define CLINKAGE_END
#endif

#if defined(__cplusplus)
#define GFS_API CLINKAGE
#else
#define GFS_API
#endif

#define STRINGIFY(X) #X

#define STATIC_ARRAY_LENGTH(A) (sizeof((A)) / sizeof((A)[0]))

#define UNUSED(X) ((void)(X))

#define MKFLAG(BITINDEX) (1 << (BITINDEX))
#define HASANYBIT(MASK, FLAG) ((MASK) | (FLAG))

#define EXPAND(X) (X)
#define UNWRAPED(X) X

#if defined(__cplusplus)
#define LITERAL(X) X
#else
#define LITERAL(X) EXPAND(X)
#endif

#if defined(__cplusplus)
#define INIT_EMPTY_STRUCT(X) LITERAL(X) UNWRAPED({})
#else
#define INIT_EMPTY_STRUCT(X) LITERAL(X) UNWRAPED({0})
#endif

#if defined(__cplusplus)
#define EMPTY_STACK_ARRAY UNWRAPED({})
#else
#define EMPTY_STACK_ARRAY UNWRAPED({0})
#endif

// @cleanup Replace `INIT_EMPTY_STRUCT` with `EMPTY_STRUCT`.
#define EMPTY_STRUCT INIT_EMPTY_STRUCT

#if defined(__cplusplus)
#define GFS_NS(S) GFS::S
#else
#define GFS_NS(S) GFS_##S
#endif

// TODO(gr3yknigh1): Check attributes and C version. [2024/10/21]
#if !defined(GFS_NORETURN)
#define GFS_NORETURN
#endif

#endif // GFS_MACROS_H_INCLUDED
