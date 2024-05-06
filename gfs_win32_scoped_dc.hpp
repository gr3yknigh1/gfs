/*
 * GFS
 *
 * FILE    gfs_win32_scoped_dc.hpp
 * AUTHOR  Ilya Akkuzin <gr3yknigh1@gmail.com>
 * LICENSE Copyright (c) 2024 Ilya Akkuzin
 * */

#ifndef GFS_WIN32_SCOPED_DC_HPP_INCLUDED
#define GFS_WIN32_SCOPED_DC_HPP_INCLUDED

#include <Windows.h>

/*
 * RAII implementation of device context wrapper.
 *
 * Aquires device context and frees it on destruction.
 */
struct ScopedDC {
    HDC Handle;

    ScopedDC(HWND window) noexcept 
        : Handle(GetDC(window)), m_Window(window)
    { }

    ~ScopedDC() noexcept
    {
        ReleaseDC(this->m_Window, this->Handle);
    }

private:
    HWND m_Window;
};

#endif  // GFS_WIN32_SCOPED_DC_HPP_INCLUDED
