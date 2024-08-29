/*
 * FILE      gfs_assert.h
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */
#if !defined(GFS_ASSERT_H)
#define GFS_ASSERT_H

#include <Windows.h>

#define ASSERT(COND)                                                                                               \
    do {                                                                                                               \
        if (!(COND)) {                                                                                                 \
            OutputDebugString("E: Assertion error: '" #COND "'.");                                                     \
            DebugBreak();                                                                                              \
            ExitProcess(1);                                                                                            \
        }                                                                                                              \
    } while (0)

#define ASSERT_ISZERO(EXPR) ASSERT((EXPR) == 0)
#define ASSERT_NONZERO(EXPR) ASSERT((EXPR) != 0)
#define ASSERT_EQ(EXPR, VAL) ASSERT((EXPR) == (VAL))
#define ASSERT_NONNULL(EXPR) ASSERT((EXPR) != NULL)

#define STATIC_ASSERT(COND) _Static_assert((COND), "")
#define EXPECT_TYPE_SIZE(TYPE, SIZE) STATIC_ASSERT(sizeof(TYPE) == (SIZE))

#endif // GFS_ASSERT_H
