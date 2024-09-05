#pragma once
/*
 * FILE      Code\StaticAssert.hpp
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */

#define STATIC_ASSERT(COND) static_assert((COND), "")
#define EXPECT_TYPE_SIZE(TYPE, SIZE) STATIC_ASSERT(sizeof(TYPE) == (SIZE))
