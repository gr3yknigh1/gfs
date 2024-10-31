/*
 * FILE      gfs\code\gfs\src\atlas.c
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */
#include "gfs/assert.h"
#include "gfs/atlas.h"
#include "gfs/bmp.h"
#include "gfs/render_opengl.h"
#include "gfs/memory.h"

Atlas
AtlasFromFile(
    Scratch *scratch, cstring8 filePath, u32 tileWidth, u32 tileHeight,
    ColorLayout colorLayout)
{
    Atlas atlas = INIT_EMPTY_STRUCT(Atlas);

    atlas.picture = ScratchAllocZero(scratch, sizeof(BMPicture));
    ASSERT_NONNULL(atlas.picture);

    ASSERT_ISOK(BMPictureLoadFromFile(atlas.picture, scratch, filePath));
    atlas.texture = GLTextureMakeFromBMPicture(atlas.picture, colorLayout);

    atlas.tileWidth = tileWidth;
    atlas.tileHeight = tileHeight;

    return atlas;
}

u32
AtlasGetXTileCount(Atlas *atlas)
{
    u32 xTileCount = atlas->picture->dibHeader.width / atlas->tileWidth;
    return xTileCount;
}

u32
AtlasGetYTileCount(Atlas *atlas)
{
    u32 yTileCount = atlas->picture->dibHeader.height / atlas->tileHeight;
    return yTileCount;
}

f32
AtlasGetTileUVWidth(Atlas *atlas)
{
    f32 tileUVWidth = MapU32ValueRangeToF32(
        atlas->tileWidth, 0, atlas->picture->dibHeader.width, 0.0f, 1.0f);
    return tileUVWidth;
}

f32
AtlasGetTileUVHeight(Atlas *atlas)
{
    f32 tileUVHeight = MapU32ValueRangeToF32(
        atlas->tileHeight, 0, atlas->picture->dibHeader.height, 0.0f, 1.0f);
    return tileUVHeight;
}

TexCoords
AtlasTileCoordsToUV(Atlas *atlas, Vector2U32 tileCoords)
{
    TexCoords uv;
    uv.t = MapU32ValueRangeToF32(
        tileCoords.x, 0, AtlasGetXTileCount(atlas), 0.0f, 1.0f);
    uv.s = MapU32ValueRangeToF32(
        tileCoords.y, 0, AtlasGetYTileCount(atlas), 0.0f, 1.0f);
    return uv;
}
