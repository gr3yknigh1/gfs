/*
 * GFS. Bitmap renderer.
 *
 * FILE    gfs_bmr.cpp
 * AUTHOR  Ilya Akkuzin <gr3yknigh1@gmail.com>
 * LICENSE Copyright (c) 2024 Ilya Akkuzin
 * */

#include "gfs_win32_bmr.hpp"

#include "gfs_types.hpp"
#include "gfs_linalg.hpp"
#include "gfs_geometry.hpp"
#include "gfs_color.hpp"
#include "gfs_macros.hpp"
#include "gfs_win32_misc.hpp"
#include "gfs_win32_scoped_dc.hpp"


#define BMR_RENDER_COMMAND_CAPACITY 1024


namespace BMR 
{
    internal void
    _UpdateWindow(
        Renderer *r,
        HDC dc,
        S32 windowXOffset,
        S32 windowYOffset,
        S32 windowWidth,
        S32 windowHeight) noexcept
    {
        StretchDIBits(
            dc,
            r->XOffset,    r->YOffset,    r->Pixels.Width, r->Pixels.Height,
            windowXOffset, windowYOffset, windowWidth,     windowHeight,
            r->Pixels.Buffer, &r->Info,
            DIB_RGB_COLORS, SRCCOPY
        );
    }

    Renderer
    Init(Color4 clearColor) noexcept
    {
        Renderer r;

        r.ClearColor = clearColor;
        r.CommandQueue.Begin = (Byte *)VirtualAlloc(
            nullptr, BMR_RENDER_COMMAND_CAPACITY, MEM_COMMIT, PAGE_READWRITE);
        r.CommandQueue.End = r.CommandQueue.Begin;
        r.CommandCount = 0;

        r.BPP = BMR_BPP;
        r.XOffset = 0;
        r.YOffset = 0;

        r.Pixels.Buffer = nullptr;
        r.Pixels.Width = 0;
        r.Pixels.Height = 0;

        return r;
    }

    void 
    DeInit(Renderer *r) noexcept
    { 
        if (r->CommandQueue.Begin != nullptr && VirtualFree(r->CommandQueue.Begin, 0, MEM_RELEASE) == 0) 
        {
            // TODO(ilya.a): Handle memory free error.
        }
        else 
        {
            r->CommandQueue.Begin = nullptr;
            r->CommandQueue.End   = nullptr;
        }

        if (r->Pixels.Buffer != nullptr && VirtualFree(r->Pixels.Buffer, 0, MEM_RELEASE) == 0) 
        {
            // TODO(ilya.a): Handle memory free error.
        } else {
            r->Pixels.Buffer = nullptr;
        }
    }

    void 
    BeginDrawing(Renderer *r, HWND window) noexcept 
    {
        r->Window = window;
    }

    void 
    EndDrawing(Renderer *r) noexcept
    {
        Size pitch = r->Pixels.Width * r->BPP;
        U8 * row = (U8 *) r->Pixels.Buffer;

        for (U64 y = 0; y < r->Pixels.Height; ++y)
        {
            Color4 *pixel = (Color4 *)row;

            for (U64 x = 0; x < r->Pixels.Width; ++x) 
            {
                Size offset = 0;

                for (U64 commandIdx = 0; commandIdx < r->CommandCount; ++commandIdx)
                {
                    RenderCommandType type = 
                        *((RenderCommandType *)(r->CommandQueue.Begin + offset));

                    offset += sizeof(RenderCommandType);

                    switch (type) {
                        case (RenderCommandType::CLEAR): {
                            Color4 color = *(Color4*)(r->CommandQueue.Begin + offset);
                            offset += sizeof(Color4);
                            *pixel = color;
                        } break;
                        case (RenderCommandType::LINE): {
                            V2U p1 = *(V2U*)(r->CommandQueue.Begin + offset);
                            offset += sizeof(V2U);

                            V2U p2 = *(V2U*)(r->CommandQueue.Begin + offset);
                            offset += sizeof(V2U);

                        } break;
                        case (RenderCommandType::RECT): {
                            Rect rect = *(Rect*)(r->CommandQueue.Begin + offset);
                            offset += sizeof(rect);

                            Color4 color = *(Color4*)(r->CommandQueue.Begin + offset);
                            offset += sizeof(Color4);

                            if (rect.IsInside(x, y)) 
                            {
                                *pixel = color;
                            }
                        } break;
                        case (RenderCommandType::GRADIENT): {
                            V2U v = *(V2U*)(r->CommandQueue.Begin + offset);
                            offset += sizeof(V2U);

                            *pixel = Color4(x + v.X, y + v.Y, 0);
                        } break;
                        case (RenderCommandType::NOP):
                        default: {
                            *pixel = r->ClearColor;
                        } break;
                    };
                }
                ++pixel;
            }

            row += pitch;
        }

        {
            // TODO(ilya.a): Check how it's differs with event thing.
            auto dc = ScopedDC(r->Window);

            RECT windowRect;
            GetClientRect(r->Window, &windowRect);
            S32 x = windowRect.left;
            S32 y = windowRect.top;
            S32 width = 0, height = 0;
            GetRectSize(&windowRect, &width, &height);

            _UpdateWindow(r, dc.Handle, x, y, width, height);
        }

        r->CommandQueue.End = r->CommandQueue.Begin;
        r->CommandCount = 0;
    }


    void 
    Update(Renderer *r, HWND window) noexcept 
    {
        PAINTSTRUCT ps = {0};
        HDC dc = BeginPaint(window, &ps);

        if (dc == nullptr) {
            // TODO(ilya.a): Handle error
        } else {
            S32 x = ps.rcPaint.left;
            S32 y = ps.rcPaint.top;
            S32 width = 0, height = 0;
            GetRectSize(&(ps.rcPaint), &width, &height);
            _UpdateWindow(r, dc, x, y, width, height);
        }

        EndPaint(window, &ps);
    }


    void 
    Resize(Renderer *r, S32 w, S32 h) noexcept
    {
        if (r->Pixels.Buffer != nullptr && VirtualFree(r->Pixels.Buffer, 0, MEM_RELEASE) == 0) 
        {
            //                                                              ^^^^^^^^^^^
            // NOTE(ilya.a): Might be more reasonable to use MEM_DECOMMIT instead for 
            // MEM_RELEASE. Because in that case it's will be keep buffer around, until
            // we use it again.
            // P.S. Also will be good to try protect buffer after deallocating or other
            // stuff.
            //
            // TODO(ilya.a): 
            //     - [ ] Checkout how it works.
            //     - [ ] Handle allocation error.
            OutputDebugString("Failed to free backbuffer memory!\n");
        }
        r->Pixels.Width = w;
        r->Pixels.Height = h;

        r->Info.bmiHeader.biSize          = sizeof(r->Info.bmiHeader);
        r->Info.bmiHeader.biWidth         = w;
        r->Info.bmiHeader.biHeight        = h;
        r->Info.bmiHeader.biPlanes        = 1;
        r->Info.bmiHeader.biBitCount      = 32;      // NOTE: Align to WORD
        r->Info.bmiHeader.biCompression   = BI_RGB;
        r->Info.bmiHeader.biSizeImage     = 0;
        r->Info.bmiHeader.biXPelsPerMeter = 0;
        r->Info.bmiHeader.biYPelsPerMeter = 0;
        r->Info.bmiHeader.biClrUsed       = 0;
        r->Info.bmiHeader.biClrImportant  = 0;

        Size bufferSize = w * h * r->BPP;
        r->Pixels.Buffer = VirtualAlloc(nullptr, bufferSize, MEM_COMMIT, PAGE_READWRITE);

        if (r->Pixels.Buffer == nullptr) 
        {
            // TODO:(ilya.a): Check for errors.
            OutputDebugString("Failed to allocate memory for backbuffer!\n");
        }
    }

    // TODO(ilya.a): Find better way to provide payload.
    template<typename T> internal void 
    _PushRenderCommand(Renderer *r, RenderCommandType type, const T &payload) noexcept
    {
        *(RenderCommand<T> *)r->CommandQueue.End = RenderCommand<T>(type, payload);
        r->CommandQueue.End += sizeof(RenderCommand<T>);

        r->CommandCount++;
    }


    void 
    Clear(Renderer *r) noexcept 
    {
        _PushRenderCommand(
            r,
            RenderCommandType::CLEAR, 
            r->ClearColor
        );
    }

    struct _DrawLine_Payload 
    {
        V2U p1;
        V2U p2;
    };

    void 
    DrawLine(Renderer *r, U32 x1, U32 y1, U32 x2, U32 y2) noexcept
    {
        _PushRenderCommand(
            r,
            RenderCommandType::LINE, 
            _DrawLine_Payload{V2U(x1, y1), V2U(x2, y2)}
        );
    }


    void
    DrawLine(Renderer *r, V2U p1, V2U p2) noexcept
    {
        _PushRenderCommand(
            r,
            RenderCommandType::LINE, 
            _DrawLine_Payload{p1, p2}
        );
    }

    struct _DrawRect_Payload 
    {
        Rect Rect;
        Color4 Color;
    };

    void 
    DrawRect(Renderer *r, Rect r_, Color4 c) noexcept 
    {
        _PushRenderCommand(
            r,
            RenderCommandType::RECT, 
            _DrawRect_Payload{r_, c}
        );
    }

    void 
    DrawRect(
        Renderer *r,
        U32 x, U32 y, 
        U32 w, U32 h, 
        Color4 c) noexcept 
    {
        _PushRenderCommand(
            r,
            RenderCommandType::RECT, 
            _DrawRect_Payload{Rect(x, y, w, h), c}
        );
    }

    void 
    DrawGrad(Renderer *r, U32 xOffset, U32 yOffset) noexcept 
    {
        _PushRenderCommand(
            r, 
            RenderCommandType::GRADIENT, 
            V2U(xOffset, yOffset)
        );
    }

    void 
    DrawGrad(Renderer *r, V2U offset) noexcept 
    {
        _PushRenderCommand(
            r, 
            RenderCommandType::GRADIENT, 
            offset
        );
    }


};  // namespace BMR
