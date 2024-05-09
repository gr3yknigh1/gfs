/*
 * GFS. Bitmap renderer.
 * 
 * Keeps one global backbuffer.
 *
 * FILE    gfs_bmr.hpp
 * AUTHOR  Ilya Akkuzin <gr3yknigh1@gmail.com>
 * LICENSE Copyright (c) 2024 Ilya Akkuzin
 * */

#ifndef GFS_BMR_HPP_INCLUDED
#define GFS_BMR_HPP_INCLUDED

#include "gfs_types.hpp"
#include "gfs_color.hpp"
#include "gfs_linalg.hpp"
#include "gfs_geometry.hpp"

// TODO(ilya.a): Parametrize it, if will be neccesery to change bytes per pixel
#define BMR_BPP 4

namespace BMR 
{

	enum class RenderCommandType 
    {
	    NOP      = 00,
	    CLEAR    = 01,

	    LINE     = 10,
	    RECT     = 11,
	    GRADIENT = 20,
	};


	template<typename T>
	struct RenderCommand
    {
	    RenderCommandType Type;
	    T Payload;

	    constexpr 
	    RenderCommand(RenderCommandType type, 
	                  T                 payload) noexcept
	        : Type(type), Payload(payload)
	    { }
	};


	void Init() noexcept;
	void DeInit() noexcept;

	void Update(HWND window) noexcept;
	void Resize(S32 w, S32 h) noexcept;

    V2U GetOffset() noexcept;
    void SetXOffset(U64 offset) noexcept;
    void SetYOffset(U64 offset) noexcept;

    void BeginDrawing(HWND window) noexcept;
	void EndDrawing() noexcept;

	void SetClearColor(const Color4 &c) noexcept;

    void Clear() noexcept;

    void DrawLine(U32 x1, U32 y1, U32 x2, U32 y2) noexcept;
    void DrawLine(V2U p1, V2U p2) noexcept;

	void DrawRect(const Rect &r, const Color4 &c) noexcept;
	void DrawRect(U32 x, U32 y, U32 w, U32 h, const Color4 &c) noexcept;

	void DrawGrad(U32 xOffset, U32 yOffset) noexcept;
	void DrawGrad(V2U offset) noexcept;

};  // namespace BMR

#endif  // GFS_BMR_HPP_INCLUDED

