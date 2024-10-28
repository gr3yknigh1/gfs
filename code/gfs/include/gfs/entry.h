#if !defined(GFS_ENTRY_H_INCLUDED)
/*
 * FILE      code\gfs\include\gfs\entry.h
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */
#define GFS_ENTRY_H_INCLUDED

#include "gfs/macros.h"

#if !defined(GFS_NOENTRY)

/*
 * @breaf Game's entry. Mainloop code.
 *
 * Should be defined in Game specific code.
 * */
GFS_API void Entry(int argc, char *argv[]);

#endif

#endif // GFS_ENTRY_H_INCLUDED
