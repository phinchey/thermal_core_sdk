#ifndef _DISPALAY_H_
#define _DISPALAY_H_

#include <stdint.h>
#include <data.h>
#include "libirparse.h"
#include "libirprocess.h"
#include "cmd.h"

#if defined(linux) || defined(unix)
#include <sys/time.h>
#include <unistd.h>
#endif

#define OPENCV_ENABLE
#ifdef OPENCV_ENABLE
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp> 
#include <opencv2/highgui/highgui_c.h> 
//using namespace cv;
#endif

#if defined(_WIN32)
int gettimeofday(struct timeval* tp, struct timezone* tzp);
#endif

//initial the parameters for displaying
void display_init(StreamFrameInfo_t* stream_frame_info);

//release the parameters
void display_release(StreamFrameInfo_t* stream_frame_info);

//display one frame
void display_one_frame(StreamFrameInfo_t* stream_frame_info, const char* title);

//display thread
void* display_function(void* threadarg);

#endif
