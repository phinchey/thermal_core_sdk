#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <stdio.h>
#include <string.h>
#include "data.h"
#include "libircmd.h"
#include "libiruvc.h"
#include "libirparse.h"

#include "display.h"
#include "cmd.h"

#define PID_TYPE1	0x5830 
#define PID_TYPE2	0x5840 
#define PID_TYPE3	0x5831
#define PID_TYPE4	0x10F9
#define VID_TYPE1	0x0BDA 
#define VID_TYPE2	0x20B4

//open the ir camera,and get its parameter(width,height,fps,and so on)
int ir_camera_open(IruvcHandle_t* handle, CameraParam_t* camera_param, int same_idx, int resolution_idx);

//close the ir camera 
int ir_camera_close(IruvcHandle_t* handle);

//stream the ir camera by the stream info
int ir_camera_stream_on(StreamFrameInfo_t* stream_frame_info, int whther_temp_output);

//stop the ir camera streaming by the stream info
int ir_camera_stream_off(StreamFrameInfo_t* stream_frame_info);

//stream thread,get the raw frame, cut to temperature and image, and then send to other thread
void* stream_function(void* threadarg);

//stream the ir camera by the camera parameter, and call the user's callback function.
int ir_camera_stream_on_with_callback(StreamFrameInfo_t* stream_frame_info, void* test_func);

//stop stream
int ir_camera_stream_off_with_callback(StreamFrameInfo_t* stream_frame_info);

#endif
