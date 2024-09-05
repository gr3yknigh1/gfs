#pragma once
/*
 * FILE      Code\Assert.hpp
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */

#include "Platform.hpp"

#define ASSERT(COND)                                                                                                   \
    do {                                                                                                               \
        if (!(COND)) {                                                                                                 \
            PlatformPutString("E: Assertion error: '" #COND "'.");                                                     \
            PlatformDebugBreak();                                                                                      \
            PlatformExitProcess(1);                                                                                    \
        }                                                                                                              \
    } while (0)

#define ASSERT_ISZERO(EXPR) ASSERT((EXPR) == 0)
#define ASSERT_ISOK(EXPR) ASSERT_ISZERO(EXPR)
#define ASSERT_NONZERO(EXPR) ASSERT((EXPR) != 0)
#define ASSERT_EQ(EXPR, VAL) ASSERT((EXPR) == (VAL))
#define ASSERT_NONNULL(EXPR) ASSERT((EXPR) != NULL)
