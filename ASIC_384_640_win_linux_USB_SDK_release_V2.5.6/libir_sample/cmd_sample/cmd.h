#ifndef _CMD_H_
#define _CMD_H_

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#if defined(_WIN32)
#include <io.h>
#elif defined(linux) || defined(unix)
#include <unistd.h>
#endif

#include "libiruvc.h"
#include "data.h"

//select the command
//void command_sel(StreamFrameInfo_t* handle1, StreamFrameInfo_t* handle2, int cmd_type);
void command_sel(StreamFrameInfo_t* handle, int cmd_type);

//command thread, get the input and select the command
void* cmd_function(void* threadarg);

#endif


