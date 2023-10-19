#pragma once
#define HAVE_STRUCT_TIMESPEC

#include <stdio.h>
#include <stdint.h>
#include <malloc.h>
#include <pthread.h>
#include "libircmd.h"
#include "libiruvc.h"
#include "libirprocess.h"

#if defined(_WIN32)
    #include <Windows.h>
#elif defined(linux) || defined(unix)
    #include <unistd.h>
    #include <semaphore.h>
#endif

#define STREAM_TIME 10000  //unit:s
extern uint8_t is_streaming;

typedef enum
{
    NO_ROTATE = 0,
    LEFT_90D,
    RIGHT_90D,
    ROTATE_180D
}RotateSide_t;

typedef enum
{
    STATUS_NO_MIRROR_FLIP = 0,
    STATUS_ONLY_MIRROR,
    STATUS_ONLY_FLIP,
    STATUS_MIRROR_FLIP
}MirrorFlipStatus_t;

typedef enum
{
    INPUT_FMT_Y14 = 0,
    INPUT_FMT_Y16,
    INPUT_FMT_YUV422,
    INPUT_FMT_YUV444,
    INPUT_FMT_RGB888
}InputFormat_t;

typedef enum
{
    OUTPUT_FMT_Y14 = 0,
    OUTPUT_FMT_YUV422,
    OUTPUT_FMT_YUV444,
    OUTPUT_FMT_RGB888,
    OUTPUT_FMT_BGR888,
}OutputFormat_t;

typedef enum
{
    PSEUDO_COLOR_ON=0,
    PSEUDO_COLOR_OFF,
}PseudoColor_t;

typedef enum
{
    IMG_ENHANCE_ON = 0,
    IMG_ENHANCE_OFF,
}ImgEnhance_t;


typedef struct {
    float ems;  // ems: 目标发射率(0.01-1)
    float ta;   // ta:大气温度(单位:摄氏度)
    float tu;   // tu:反射温度(单位:摄氏度)
    float dist; // dist:目标距离(0.25-49.99,单位:m)
    float hum;  // hum: 环境相对湿度(0-1)
}NewCorrectParam_t;

typedef struct {
    uint16_t width;
    uint16_t height;
    uint32_t byte_size;
    RotateSide_t rotate_side;
    MirrorFlipStatus_t mirror_flip_status;
    InputFormat_t  input_format;
    OutputFormat_t  output_format;
    PseudoColor_t  pseudo_color_status;
    ImgEnhance_t   img_enhance_status;
}FrameInfo_t;

typedef struct {
    IruvcHandle_t* iruvc_handle;
    IrcmdHandle_t* ircmd_handle;
    UserCallback_t callback;
    uint8_t* raw_frame;
    uint8_t* image_frame;
    uint32_t image_byte_size;
    uint8_t* temp_frame;
    uint32_t temp_byte_size;
    FrameInfo_t image_info;
    FrameInfo_t temp_info;
    CameraParam_t camera_param;
    timeval timer;
    //thread's semaphore
#if defined(_WIN32)
    HANDLE image_sem;
    HANDLE image_done_sem;
#elif defined(linux) || defined(unix)
    sem_t image_sem;
    sem_t image_done_sem;
#endif
    uint8_t* image_tmp_frame1;
    uint8_t* image_tmp_frame2;
    uint8_t is_streaming;
}StreamFrameInfo_t;

//initial the pthread's cond and mutex
int init_pthread_sem(StreamFrameInfo_t* stream_frame_info);

//release the pthread's cond and mutex
int destroy_pthread_sem(StreamFrameInfo_t* stream_frame_info);

//create space for getting frames
int create_data_demo(StreamFrameInfo_t* stream_frame_info);

//destroy the space
int destroy_data_demo(StreamFrameInfo_t* stream_frame_info);
