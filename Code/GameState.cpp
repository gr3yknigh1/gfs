/*
 * FILE      Code\GameState.cpp
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT Copyright (c) 2024 Ilya Akkuzin
 * */
#include "GameState.hpp"
#include "Macros.hpp"
#include "Types.hpp"

static volatile bool gGameShouldStop = false;

bool
GameStateShouldStop(void) {
    return gGameShouldStop == true;
}

void
GameStateStop(void) {
    gGameShouldStop = true;
}
