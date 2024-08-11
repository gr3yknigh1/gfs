/*
 * FILE      gfs_assert.h
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */
#if !defined(GFS_ASSERT_H)
#define GFS_ASSERT_H

#include <Windows.h>

#define GFS_ASSERT(COND)                                                                                               \
    do {                                                                                                               \
        if (!(COND)) {                                                                                                 \
            OutputDebugString("E: Assertion error: '" #COND "'.");                                                     \
            DebugBreak();                                                                                              \
            ExitProcess(1);                                                                                            \
        }                                                                                                              \
    } while (0)

#define GFS_STATIC_ASSERT(COND) _Static_assert((COND), "")
#define GFS_EXPECT_TYPE_SIZE(TYPE, SIZE) GFS_STATIC_ASSERT(sizeof(TYPE) == (SIZE))

#endif // GFS_ASSERT_H
