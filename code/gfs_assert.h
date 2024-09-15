#if !defined(GFS_ASSERT_H_INCLUDED)
/*
 * FILE      gfs_assert.h
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */
#define GFS_ASSERT_H_INCLUDED

#include "gfs_platform.h"

#define ASSERT(COND)                                                           \
    do {                                                                       \
        if (!(COND)) {                                                         \
            PutString("E: Assertion error: '" #COND "'.\n");                   \
            PutLastError();                                                    \
            ThrowDebugBreak();                                                 \
            ProcessExit(1);                                                    \
        }                                                                      \
    } while (0)

#define ASSERT_ISZERO(EXPR) ASSERT((EXPR) == 0)
#define ASSERT_ISOK(EXPR) ASSERT_ISZERO(EXPR)
#define ASSERT_NONZERO(EXPR) ASSERT((EXPR) != 0)
#define ASSERT_EQ(EXPR, VAL) ASSERT((EXPR) == (VAL))
#define ASSERT_NOTEQ(EXPR, VAL) ASSERT((EXPR) != (VAL))
#define ASSERT_NONNULL(EXPR) ASSERT((EXPR) != NULL)
#define ASSERT_ISTRUE(EXPR) ASSERT((EXPR))
#define ASSERT_ISFALSE(EXPR) ASSERT(!(EXPR))

#define THROW(MESSAGE)                                                         \
    do {                                                                       \
        PutString((MESSAGE));                                                  \
        PutLastError();                                                        \
        ThrowDebugBreak();                                                     \
        ProcessExit(1);                                                        \
    } while (0)

#endif // GFS_ASSERT_H_INCLUDED
