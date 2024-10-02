#if !defined(GFS_BMP_H_INCLUDED)
/*
 * FILE      gfs.core\include\gfs\bmp.h
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */
#define GFS_BMP_H_INCLUDED

#include "gfs/types.h"
#include "gfs/memory.h"

typedef enum {
    BMP_HEADER_TYPE_BITMAPCOREHEADER = 12,
    BMP_HEADER_TYPE_OS22XBITMAPHEADER_S = 16,
    BMP_HEADER_TYPE_BITMAPINFOHEADER = 40,
    BMP_HEADER_TYPE_BITMAPV2INFOHEADER = 52,
    BMP_HEADER_TYPE_BITMAPV3INFOHEADER = 56,
    BMP_HEADER_TYPE_OS22XBITMAPHEADER = 64,
    BMP_HEADER_TYPE_BITMAPV4HEADER = 108,
    BMP_HEADER_TYPE_BITMAPV5HEADER = 124,
} BMPictureHeaderType;

typedef u32 BMPictureHeaderTypeTag;

typedef enum {
    BMP_COMPRESSION_METHOD_RGB = 0,
    BMP_COMPRESSION_METHOD_RLE8 = 1,
    BMP_COMPRESSION_METHOD_RLE4 = 2,
    BMP_COMPRESSION_METHOD_BITFIELDS = 3,
    BMP_COMPRESSION_METHOD_JPEG = 4,
    BMP_COMPRESSION_METHOD_PNG = 5,
    BMP_COMPRESSION_METHOD_ALPHABITFIELDS = 6,
    BMP_COMPRESSION_METHOD_CMYK = 11,
    BMP_COMPRESSION_METHOD_CMYKRLE8 = 12,
    BMP_COMPRESSION_METHOD_CMYKRLE4 = 13,
} BMPictureCompressionMethod;

typedef u32 BMPictureCompressionMethodTag;

#pragma pack(push, 1)
typedef struct {
    u16 type;
    u32 fileSize;
    u16 reserved[2];
    u32 dataOffset;
} BMPictureHeader;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct {
    BMPictureHeaderTypeTag headerSize;
    u32 width;
    u32 height;
    u16 planesCount;
    u16 depth;
    BMPictureCompressionMethodTag compressionMethod;
    u32 imageSize;
    u32 xPixelPerMeter;
    u32 yPixelPerMeter;
    u32 colorUsed;
    u32 colorImportant;
} BMPictureDIBHeader;
#pragma pack(pop)

typedef struct {
    BMPictureHeader header;
    BMPictureDIBHeader dibHeader;
    void *data;
} BMPicture;

typedef enum {
    BMP_LOAD_FROM_FILE_OK,
    BMP_LOAD_FROM_FILE_ERR,
} BMPictureLoadFromFileRC;

GFS_EXPORT BMPictureLoadFromFileRC BMPictureLoadFromFile(BMPicture *image, Scratch *scratch, cstring8 filePath);

#endif // GFS_BMP_H_INCLUDED
