/*
 * FILE      gfs_bmp.c
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */

#include "gfs_bmp.h"
#include "gfs_platform.h"
#include "gfs_assert.h"
#include "gfs_render.h"

BMPictureLoadFromFileRC
BMPictureLoadFromFile(BMPicture *picture, Scratch *scratch, cstring8 filePath) {
    FileOpenResult result =
        FileOpenEx(filePath, scratch, PLATFORM_PERMISSION_READ);
    ASSERT_ISOK(result.code);

    FileHandle *fileHandle = result.handle;
    ASSERT_ISTRUE(FileHandleIsValid(fileHandle));

    ASSERT_ISOK(FileLoadToBufferEx(
        fileHandle, &picture->header, sizeof(picture->header), NULL, 0));

    ASSERT_ISOK(FileLoadToBufferEx(
        fileHandle, &picture->dibHeader, sizeof(picture->dibHeader), NULL,
        sizeof(picture->header)));

    picture->data = ScratchAlloc(scratch, picture->dibHeader.imageSize);
    ASSERT_NONNULL(picture->data);

    ASSERT_ISOK(FileLoadToBufferEx(
        fileHandle, picture->data, picture->dibHeader.imageSize, NULL,
        picture->header.dataOffset));

    ASSERT_ISOK(FileClose(fileHandle));

    return BMP_LOAD_FROM_FILE_OK;
}
