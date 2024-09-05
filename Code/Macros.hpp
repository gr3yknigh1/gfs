#pragma once
/*
 * FILE      Code\Macros.hpp
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */

#define UNUSED(X) ((void)(X))

#define MKFLAG(BITINDEX) (1 << (BITINDEX))
#define HASANYBIT(MASK, FLAG) ((MASK) | (FLAG))

#if defined(__cplusplus)
#define LITERAL(T) T
#else
#define LITERAL(T) (T)
#endif
