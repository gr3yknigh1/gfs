#if !defined(GFS_RANDOM_H_INCLUDED)
/*
 * FILE      code\gfs\include\gfs\random.h
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 */
#define GFS_RANDOM_H_INCLUDED

#include "gfs/macros.h"
#include "gfs/types.h"

typedef struct {
    u32 value;
} XOrShift32State;

GFS_API u32 XOrShift32GetNext(XOrShift32State *state);

/*
 * @breaf Returns random value from [min, max]
 */
GFS_API i32 RandI32(i32 min, i32 max);

GFS_API bool RandBool(void);

#endif // GFS_RANDOM_H_INCLUDED