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

#ifndef NULL
#define NULL ((void *)0)
#endif  // NULL

#endif  // GFS_MACROS_H_INCLUDED
