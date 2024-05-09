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

    InternalFunc void
    _UpdateWindow(
        State s,
        HDC dc,
        S32 windowXOffset,
        S32 windowYOffset,
        S32 windowWidth,
        S32 windowHeight) noexcept
    {
        StretchDIBits(
            dc,
            s.XOffset,     s.YOffset,     s.Pixels.Width, s.Pixels.Height,
            windowXOffset, windowYOffset, windowWidth,    windowHeight,
            s.Pixels.Buffer, &s.Info,
            DIB_RGB_COLORS, SRCCOPY
        );
    }

    State
    Init(Color4 clearColor) noexcept
    {
        State s;

        s.ClearColor = clearColor;
        s.CommandQueue.Begin = (Byte *)VirtualAlloc(
            nullptr, BMR_RENDER_COMMAND_CAPACITY, MEM_COMMIT, PAGE_READWRITE);
        s.CommandQueue.End = s.CommandQueue.Begin;
        s.CommandCount = 0;

        s.BPP = BMR_BPP;
        s.XOffset = 0;
        s.YOffset = 0;

        s.Pixels.Buffer = nullptr;
        s.Pixels.Width = 0;
        s.Pixels.Height = 0;

        return s;
    }

    void 
    DeInit(State s) noexcept
    { 
        if (s.CommandQueue.Begin != nullptr && VirtualFree(s.CommandQueue.Begin, 0, MEM_RELEASE) == 0) 
        {
            // TODO(ilya.a): Handle memory free error.
        }
        else 
        {
            s.CommandQueue.Begin = nullptr;
            s.CommandQueue.End   = nullptr;
        }

        if (s.Pixels.Buffer != nullptr && VirtualFree(s.Pixels.Buffer, 0, MEM_RELEASE) == 0) 
        {
            // TODO(ilya.a): Handle memory free error.
        } else {
            s.Pixels.Buffer = nullptr;
        }
    }

    void 
    BeginDrawing(State s, HWND window) noexcept 
    {
        s.Window = window;
    }

    void 
    EndDrawing(State s) noexcept
    {
        Size pitch = s.Pixels.Width * s.BPP;
        U8 * row = (U8 *) s.Pixels.Buffer;

        for (U64 y = 0; y < s.Pixels.Height; ++y) {
            Color4 *pixel = (Color4 *)row;

            for (U64 x = 0; x < s.Pixels.Width; ++x) {
                Size offset = 0;

                for (U64 commandIdx = 0; commandIdx < s.CommandCount; ++commandIdx) {
                    RenderCommandType type = 
                        *((RenderCommandType *)(s.CommandQueue.Begin + offset));

                    offset += sizeof(RenderCommandType);

                    switch (type) {
                        case (RenderCommandType::CLEAR): {
                            Color4 color = *(Color4*)(s.CommandQueue.Begin + offset);
                            offset += sizeof(Color4);
                            *pixel = color;
                        } break;
                        case (RenderCommandType::LINE): {
                            V2U p1 = *(V2U*)(s.CommandQueue.Begin + offset);
                            offset += sizeof(V2U);

                            V2U p2 = *(V2U*)(s.CommandQueue.Begin + offset);
                            offset += sizeof(V2U);

                        } break;
                        case (RenderCommandType::RECT): {
                            Rect rect = *(Rect*)(s.CommandQueue.Begin + offset);
                            offset += sizeof(rect);

                            Color4 color = *(Color4*)(s.CommandQueue.Begin + offset);
                            offset += sizeof(Color4);

                            if (rect.IsInside(x, y)) {
                                *pixel = color;
                            }
                        } break;
                        case (RenderCommandType::GRADIENT): {
                            V2U v = *(V2U*)(s.CommandQueue.Begin + offset);
                            offset += sizeof(V2U);

                            *pixel = Color4(x + v.X, y + v.Y, 0);
                        } break;
                        case (RenderCommandType::NOP):
                        default: {
                            *pixel = s.ClearColor;
                        } break;
                    };
                }
                ++pixel;
            }

            row += pitch;
        }

        {
            // TODO(ilya.a): Check how it's differs with event thing.
            auto dc = ScopedDC(s.Window);

            RECT windowRect;
            GetClientRect(s.Window, &windowRect);
            S32 x = windowRect.left;
            S32 y = windowRect.top;
            S32 width = 0, height = 0;
            GetRectSize(&windowRect, &width, &height);

            _UpdateWindow(s, dc.Handle, x, y, width, height);
        }

        s.CommandQueue.End = s.CommandQueue.Begin;
        s.CommandCount = 0;
    }


    void 
    Update(State s, HWND window) noexcept 
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
            _UpdateWindow(s, dc, x, y, width, height);
        }

        EndPaint(window, &ps);
    }


    void 
    Resize(State s, S32 w, S32 h) noexcept
    {
        if (s.Pixels.Buffer != nullptr && VirtualFree(s.Pixels.Buffer, 0, MEM_RELEASE) == 0) 
        {
            //                                                            ^^^^^^^^^^^
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
        s.Pixels.Width = w;
        s.Pixels.Height = h;

        s.Info.bmiHeader.biSize          = sizeof(s.Info.bmiHeader);
        s.Info.bmiHeader.biWidth         = w;
        s.Info.bmiHeader.biHeight        = h;
        s.Info.bmiHeader.biPlanes        = 1;
        s.Info.bmiHeader.biBitCount      = 32;      // NOTE: Align to WORD
        s.Info.bmiHeader.biCompression   = BI_RGB;
        s.Info.bmiHeader.biSizeImage     = 0;
        s.Info.bmiHeader.biXPelsPerMeter = 0;
        s.Info.bmiHeader.biYPelsPerMeter = 0;
        s.Info.bmiHeader.biClrUsed       = 0;
        s.Info.bmiHeader.biClrImportant  = 0;

        Size bufferSize = w * h * s.BPP;
        s.Pixels.Buffer = VirtualAlloc(nullptr, bufferSize, MEM_COMMIT, PAGE_READWRITE);

        if (s.Pixels.Buffer == nullptr) {
            // TODO:(ilya.a): Check for errors.
            OutputDebugString("Failed to allocate memory for backbuffer!\n");
        }
    }

    // TODO(ilya.a): Find better way to provide payload.
    template<typename T> InternalFunc void 
    _PushRenderCommand(State s, RenderCommandType type, const T &payload) noexcept
    {
        *(RenderCommand<T> *)s.CommandQueue.End = RenderCommand<T>(type, payload);
        s.CommandQueue.End += sizeof(RenderCommand<T>);

        s.CommandCount++;
    }

    void 
    SetClearColor(State s, Color4 c) noexcept 
    {
        s.ClearColor = c;    
    }


    void 
    Clear(State s) noexcept 
    {
        _PushRenderCommand(
            s,
            RenderCommandType::CLEAR, 
            s.ClearColor
        );
    }

    struct _DrawLine_Payload 
    {
        V2U p1;
        V2U p2;
    };

    void 
    DrawLine(State s, U32 x1, U32 y1, U32 x2, U32 y2) noexcept
    {
        _PushRenderCommand(
            s,
            RenderCommandType::LINE, 
            _DrawLine_Payload{V2U(x1, y1), V2U(x2, y2)}
        );
    }


    void
    DrawLine(State s, V2U p1, V2U p2) noexcept
    {
        _PushRenderCommand(
            s,
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
    DrawRect(State s, Rect r, Color4 c) noexcept 
    {
        _PushRenderCommand(
            s,
            RenderCommandType::RECT, 
            _DrawRect_Payload{r, c}
        );
    }

    void 
    DrawRect(
        State s,
        U32 x, U32 y, 
        U32 w, U32 h, 
        Color4 c) noexcept 
    {
        _PushRenderCommand(
            s,
            RenderCommandType::RECT, 
            _DrawRect_Payload{Rect(x, y, w, h), c}
        );
    }

    void 
    DrawGrad(State s, U32 xOffset, U32 yOffset) noexcept 
    {
        _PushRenderCommand(
            s, 
            RenderCommandType::GRADIENT, 
            V2U(xOffset, yOffset)
        );
    }

    void 
    DrawGrad(State s, V2U offset) noexcept 
    {
        _PushRenderCommand(
            s, 
            RenderCommandType::GRADIENT, 
            offset
        );
    }


};  // namespace BMR
