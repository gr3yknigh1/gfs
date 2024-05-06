/*
 * GFS. Tools related to linear algebra.
 *
 * FILE    gfs_macros.hpp
 * AUTHOR  Ilya Akkuzin <gr3yknigh1@gmail.com>
 * LICENSE Copyright (c) 2024 Ilya Akkuzin
 * */


#ifndef GFS_LIN_HPP_INCLUDED
#define GFS_LIN_HPP_INCLUDED


#include "gfs_types.hpp"


// TODO: Scope `T` type to numeric only.
// template <typename T>
struct Vec2i {
    S32 X;
    S32 Y;

    constexpr Vec2i(S32 x = 0, S32 y = 0) noexcept 
        : X(x), Y(y)
    { }
};

struct Vec2u {
    U32 X;
    U32 Y;

    constexpr Vec2u(U32 x = 0, U32 y = 0) noexcept 
        : X(x), Y(y)
    { }
};

#endif // GFS_LIN_HPP_INCLUDED
