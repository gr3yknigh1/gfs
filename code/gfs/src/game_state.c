/*
 * FILE      gfs_game_state.c
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT Copyright (c) 2024 Ilya Akkuzin
 * */
#include "gfs/game_state.h"
#include "gfs/macros.h"

static volatile bool gGameShouldStop = false;

bool
GameStateShouldStop(void)
{
    return gGameShouldStop == true;
}

void
GameStateStop(void)
{
    gGameShouldStop = true;
}
