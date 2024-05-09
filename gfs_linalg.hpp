/*
 * GFS. Tools related to linear algebra.
 *
 * FILE    gfs_linalg.hpp
 * AUTHOR  Ilya Akkuzin <gr3yknigh1@gmail.com>
 * LICENSE Copyright (c) 2024 Ilya Akkuzin
 * */


#ifndef GFS_LIN_HPP_INCLUDED
#define GFS_LIN_HPP_INCLUDED

#include "gfs_types.hpp"

struct V2S {
    S32 X;
    S32 Y;

    constexpr V2S(S32 x = 0, S32 y = 0) noexcept 
        : X(x), Y(y)
    { }
};

struct V2U {
    U32 X;
    U32 Y;

    constexpr V2U(U32 x = 0, U32 y = 0) noexcept 
        : X(x), Y(y)
    { }
};

#endif // GFS_LIN_HPP_INCLUDED
