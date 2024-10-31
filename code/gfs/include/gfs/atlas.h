#if !defined(GFS_ATLAS_H_INCLUDED)
/*
 * FILE      gfs\code\gfs\include\gfs\atlas.h
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */
#define GFS_ATLAS_H_INCLUDED

#include "gfs/render_opengl.h"
#include "gfs/physics.h"
#include "gfs/types.h"

typedef struct {
    BMPicture *picture;
    GLTexture texture;

    u32 tileWidth;
    u32 tileHeight;
} Atlas;

GFS_API Atlas AtlasFromFile(
    Scratch *scratch, cstring8 filePath, u32 tileWidth, u32 tileHeight,
    ColorLayout colorLayout);

GFS_API u32 AtlasGetXTileCount(Atlas *atlas);
GFS_API u32 AtlasGetYTileCount(Atlas *atlas);

GFS_API f32 AtlasGetTileUVWidth(Atlas *atlas);
GFS_API f32 AtlasGetTileUVHeight(Atlas *atlas);

GFS_API TexCoords AtlasTileCoordsToUV(Atlas *atlas, Vector2U32 tileCoords);

#endif // GFS_ATLAS_H_INCLUDED
