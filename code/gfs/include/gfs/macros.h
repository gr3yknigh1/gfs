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
#define GFS_EXPORT CLINKAGE
#else
#define GFS_EXPORT
#endif

#define STRINGIFY(X) #X

#define UNUSED(X) ((void)(X))

#define MKFLAG(BITINDEX) (1 << (BITINDEX))
#define HASANYBIT(MASK, FLAG) ((MASK) | (FLAG))

#define EXPAND(X) (X)

#if defined(__cplusplus)
#define LITERAL(X) X
#else
#define LITERAL(X) EXPAND(X)
#endif

#if defined(__cplusplus)
#define INIT_EMPTY_STRUCT(X)                                                                                           \
    LITERAL(X) {}
#else
#define INIT_EMPTY_STRUCT(X)                                                                                           \
    LITERAL(X) { 0 }
#endif

#if defined(__cplusplus)
#define GFS_NS(S) GFS::S
#else
#define GFS_NS(S) GFS_##S
#endif

#endif // GFS_MACROS_H_INCLUDED
