#if !defined(GFS_STATE_H_INCLUDED)
#define GFS_STATE_H_INCLUDED
/*
 * FILE      gfs_state.h
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT Copyright (c) 2024 Ilya Akkuzin
 * */

#include "gfs_types.h"

bool State_ShouldStop(void);
void State_Stop(void);

#endif // GFS_STATE_H_INCLUDED
