/*
 * GFS. String manipulation procedurals.
 *
 * FILE    gfs_string.hpp
 * AUTHOR  Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT Copyright (c) 2024 Ilya Akkuzin
 * */

#ifndef GFS_STRING_HPP_INCLUDED
#define GFS_STRING_HPP_INCLUDED

#include "gfs_types.hpp"

constexpr Size 
CStr_GetLength(CStr s) noexcept
{
	Size size = 0;
	while (s[size] != '\0')
	{
		size++;
	}
	return size;
}

#endif  // GFS_STRING_HPP_INCLUDED
