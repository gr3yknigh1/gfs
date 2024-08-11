/*
 * FILE      gfs_bmp.h
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */

#include "gfs_types.h"

typedef enum {
    BI_BITMAPCOREHEADER = 12,
    BI_OS22XBITMAPHEADER_S = 16,
    BI_BITMAPINFOHEADER = 40,
    BI_BITMAPV2INFOHEADER = 52,
    BI_BITMAPV3INFOHEADER = 56,
    BI_OS22XBITMAPHEADER = 64,
    BI_BITMAPV4HEADER = 108,
    BI_BITMAPV5HEADER = 124,
} BmpDIBHeader;

typedef enum {
    BMPCOMPRESSION_RGB = 0,
    BMPCOMPRESSION_RLE8 = 1,
    BMPCOMPRESSION_RLE4 = 2,
    BMPCOMPRESSION_BITFIELDS = 3,
    BMPCOMPRESSION_JPEG = 4,
    BMPCOMPRESSION_PNG = 5,
    BMPCOMPRESSION_ALPHABITFIELDS = 6,
    BMPCOMPRESSION_CMYK = 11,
    BMPCOMPRESSION_CMYKRLE8 = 12,
    BMPCOMPRESSION_CMYKRLE4 = 13,
} BmpCompression;

#pragma pack(push, 1)
typedef struct {
    char8 type[2]; /* "BM", "BA", "CI", "CP", "IC", "PT" */
    u32 fileSize;
    u16 reserved0;
    u16 reserved1;
    u32 dataOffset;
    BmpDIBHeader dibHeaderSize;
    u32 width;
    u32 height;
    u16 planesCount;
    u16 depth;
    BmpCompression compression;
    u32 imageSize;
    u32 xPixelPerMeter;
    u32 yPixelPerMeter;
    u32 colorUsed;
    u32 colorImportant;
} BmpHeader;
#pragma pack(pop)
