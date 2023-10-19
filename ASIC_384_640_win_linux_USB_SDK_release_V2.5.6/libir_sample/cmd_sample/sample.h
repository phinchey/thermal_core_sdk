#if defined(_WIN32)
#include <Windows.h>
#elif defined(linux) || defined(unix)
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>
#endif

#include <stdio.h>

#include "cmd.h"
#include "camera.h"
#include "display.h"
#include "sample_version.h"

typedef enum {
	DEBUG_PRINT = 0,
	ERROR_PRINT,
	NO_PRINT,
}log_level_t;


//#define USER_FUNCTION_CALLBACK

//#define IMAGE_AND_TEMP_OUTPUT	//normal mode:get 1 image frame and temp frame at the same time eg:640*1024
#define IMAGE_OUTPUT	//only image frame eg:640*512
//#define TEMP_OUTPUT		//only temp frame eg:640*512