#if !defined(GFS_GAME_STATE_H_INCLUDED)
#define GFS_GAME_STATE_H_INCLUDED
/*
 * FILE      gfs_game_state.h
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT Copyright (c) 2024 Ilya Akkuzin
 * */

#include "gfs/types.h"
#include "gfs/macros.h"

GFS_API bool GameStateShouldStop(void);
GFS_API void GameStateStop(void);

#endif // GFS_GAME_STATE_H_INCLUDED
