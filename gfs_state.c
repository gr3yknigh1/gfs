/*
 * FILE      gfs_state.c
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT Copyright (c) 2024 Ilya Akkuzin
 * */
#include "gfs_state.h"
#include "gfs_macros.h"

global_var no_optimize bool gGameShouldStop = false;

bool
StateShouldStop(void) {
    return gGameShouldStop == true;
}

void
StateStop(void) {
    gGameShouldStop = true;
}
