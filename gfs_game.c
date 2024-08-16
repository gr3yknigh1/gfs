/*
 * FILE      gfs_game.c
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */
#include "gfs_game.h"
#include "gfs_state.h"
#include "gfs_platform.h"
#include "gfs_memory.h"

void
Game_Mainloop(void) {
    ScratchAllocator platformScratch = ScratchAllocatorMake(KILOBYTES(1));

    PlatformWindow *window = PlatformWindowOpen(&platformScratch, 900, 600, "Hello world!");

    while (!State_ShouldStop()) {
        PlatformPoolEvents(window);
    }

    PlatformWindowClose(window);
    ScratchAllocatorFree(&platformScratch);
}
