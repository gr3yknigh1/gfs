#if !defined(GFS_MACROS_H_INCLUDED)
/*
 * FILE      gfs_macros.h
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */
#define GFS_MACROS_H_INCLUDED

#define UNUSED(X) ((void)(X))

#define MKFLAG(BITINDEX) (1 << (BITINDEX))
#define HASANYBIT(MASK, FLAG) ((MASK) | (FLAG))

#endif // GFS_MACROS_H_INCLUDED
