/*
 * FILE      gfs_macros.h
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */

#ifndef GFS_MACROS_H_INCLUDED
#define GFS_MACROS_H_INCLUDED

#define internal static
#define global_var static
#define persist_var static
#define procedure void

#define UNUSED(X) ((void)(X))

#ifndef NULL
#define NULL ((void *)0)
#endif  // NULL

#define MKFLAG(BITINDEX) (1 << (BITINDEX))
#define HASANYBIT(MASK, FLAG) ((MASK) | (FLAG))

#endif  // GFS_MACROS_H_INCLUDED
