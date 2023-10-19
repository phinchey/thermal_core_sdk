#include "camera.h"


//get specific device via pid&vid from all devices
int get_dev_index_with_pid_vid(DevCfg_t devs_cfg[])
{
    int cur_dev_index = 0;
    for (int i = 0; i < 64; i++)
    {
        if(((devs_cfg[i].vid == VID_TYPE1)&& ((devs_cfg[i].pid == PID_TYPE1) || (devs_cfg[i].pid == PID_TYPE2) || (devs_cfg[i].pid == PID_TYPE3))\
            || ((devs_cfg[i].vid == VID_TYPE2) && (devs_cfg[i].pid == PID_TYPE4)) ))
        {
            cur_dev_index = i;
            printf("name=%s\n", devs_cfg[i].name);
            return  cur_dev_index;
        }
    }
    printf("pid or vid is wrong\n");
    return -1;
}

//set the camera_param from camera_stream_info and stream_index
CameraParam_t camera_para_set(DevCfg_t dev_cfg, int stream_index, CameraStreamInfo_t camera_stream_info[])
{
    CameraParam_t camera_param = { 0 };
    camera_param.dev_cfg = dev_cfg;
    camera_param.format = camera_stream_info[stream_index].format;
    //if (camera_stream_info[stream_index].format == NULL)
    //{
    //    //puts(camera_param.format);
    //    printf("is NULL\n");
    //}
    camera_param.width = camera_stream_info[stream_index].width;
    camera_param.height = camera_stream_info[stream_index].height;
    camera_param.frame_size = camera_param.width * camera_param.height * 2;
    camera_param.fps = camera_stream_info[stream_index].fps[0];
    camera_param.timeout_ms_delay = 1000;
    return camera_param;
}

//open camera device by camera_param
int ir_camera_open(IruvcHandle_t* handle, CameraParam_t* camera_param, int same_idx, int resolution_idx)
{
    DevCfg_t devs_cfg[64] = { 0 };
    CameraStreamInfo_t camera_stream_info[32] = { 0 };
    int rst = iruvc_camera_init(handle);
    if (rst < 0)
    {
        printf("uvc_camera_init:%d\n", rst);
        return rst;
    }

    memset(devs_cfg, 0, sizeof(DevCfg_t) * 64); //clear the device list before get list
    rst = iruvc_camera_list(handle, devs_cfg);
    if (rst < 0)
    {
        printf("uvc_camera_list:%d\n", rst);
        return rst;
    }

    int dev_index = 0;

    dev_index = get_dev_index_with_pid_vid(devs_cfg);
    if (dev_index < 0)
    {
        printf("can not get this device!\n");
        return dev_index;
    }

    rst = iruvc_camera_info_get(handle, &(devs_cfg[dev_index]), camera_stream_info);
    if (rst < 0)
    {
        printf("uvc_camera_info_get:%d\n", rst);
        return rst;
    }
    rst = iruvc_camera_open_same(handle, devs_cfg[dev_index], same_idx);
    if (rst < 0)
    {
        printf("uvc_camera_open:%d\n", rst);
        return rst;
    }

    int i = 0;
    while (camera_stream_info[i].width != 0 && camera_stream_info[i].height != 0)
    {
        printf("width: %d,height: %d\n", camera_stream_info[i].width, camera_stream_info[i].height);
        i++;
    }
    *camera_param = camera_para_set(devs_cfg[dev_index], resolution_idx, camera_stream_info);

    return 0;
}

//close the device
int ir_camera_close(IruvcHandle_t* handle)
{
    int rst = 0;

    rst = iruvc_camera_stream_close(handle, CLOSE_CAM_SIDE_PREVIEW);
    if (rst < 0)
        return rst;

    iruvc_camera_close(handle);
    iruvc_camera_release(handle);
    return rst;
}

//stream start by stream_frame_info
int ir_camera_stream_on(StreamFrameInfo_t* stream_frame_info, int whther_temp_output)
{
    int rst;
    init_pthread_sem(stream_frame_info);

    stream_frame_info->callback.iruvc_handle = stream_frame_info->iruvc_handle;
    stream_frame_info->callback.usr_func = NULL;
    stream_frame_info->callback.usr_param = NULL;

    rst = iruvc_camera_stream_start(stream_frame_info->iruvc_handle, stream_frame_info->camera_param, \
                                    &stream_frame_info->callback);
    if (rst < 0)
    {
        printf("uvc_camera_stream_start:%d\n", rst);
        return rst;
    }
    if (whther_temp_output == 1)
    {
        rst = basic_y16_preview(stream_frame_info->ircmd_handle, BASIC_Y16_MODE_TEMPERATURE);
        if (rst < 0)
        {
            printf("y16_preview_start:%d\n", rst);
            return rst;
        }
    }
    stream_frame_info->is_streaming = 1;
    return rst;
}

//stream stop
int ir_camera_stream_off(StreamFrameInfo_t* stream_frame_info)
{
    int rst = 0;

    rst = iruvc_camera_stream_close(stream_frame_info->iruvc_handle, CLOSE_CAM_SIDE_PREVIEW);
    if (rst < 0)
    {
        return rst;
    }

    destroy_data_demo(stream_frame_info);
    destroy_pthread_sem(stream_frame_info);
    stream_frame_info->is_streaming = 0;

    return rst;
}

//stream start by stream_frame_info and callback the user's function while receiving the frame
int ir_camera_stream_on_with_callback(StreamFrameInfo_t* stream_frame_info, void* test_func)
{
    int rst;

    stream_frame_info->callback.iruvc_handle = stream_frame_info->iruvc_handle;
    stream_frame_info->callback.usr_func = test_func;
    stream_frame_info->callback.usr_param = (void*)stream_frame_info;

    rst = iruvc_camera_stream_start(stream_frame_info->iruvc_handle, stream_frame_info->camera_param, \
                                    & stream_frame_info->callback);
    if (rst < 0)
    {
        printf("uvc_camera_stream_start:%d\n", rst);
        return rst;
    }
    stream_frame_info->is_streaming = 1;

    //display 100s
#if defined(_WIN32)
    Sleep(100000);
#elif defined(linux) || defined(unix)
    sleep(100);
#endif

#ifdef OPENCV_ENABLE
    cv::destroyAllWindows();
#endif

    return rst;
}

//stop stream
int ir_camera_stream_off_with_callback(StreamFrameInfo_t* stream_frame_info)
{
    int rst;
    rst = iruvc_camera_stream_close(stream_frame_info->iruvc_handle, KEEP_CAM_SIDE_PREVIEW);
    if (rst < 0)
    {
        return rst;
    }
    stream_frame_info->is_streaming = 0;

    destroy_data_demo(stream_frame_info);
    return rst;
}



//stream thread.this function can get the raw frame and cut it to image frame and temperature frame
//and send semaphore to image/temperature thread
void* stream_function(void* threadarg)
{
    StreamFrameInfo_t* stream_frame_info;
    stream_frame_info = (StreamFrameInfo_t*)threadarg;
    if (stream_frame_info == NULL)
    {
        return NULL;
    }

    printf("fps=%d\n", stream_frame_info->camera_param.fps);
    int i = 0;
    int r = 0;
    int overtime_cnt = 0;
    int overtime_threshold = 2;

    while (stream_frame_info->is_streaming && (i <= STREAM_TIME * stream_frame_info->camera_param.fps))//display stream_time seconds
    {
#if defined(_WIN32)
        WaitForSingleObject(stream_frame_info->image_sem, INFINITE);
#elif defined(linux) || defined(unix)
        sem_wait(&stream_frame_info->image_sem);
#endif
        r = iruvc_frame_get(stream_frame_info->iruvc_handle, stream_frame_info->raw_frame);
        if (r < 0)
        {
            overtime_cnt++;
        }
        else
        {
            overtime_cnt = 0;
        }
        if (r < 0 && overtime_cnt >= overtime_threshold)
        {
#if defined(_WIN32)
            ReleaseSemaphore(stream_frame_info->image_done_sem, 1, NULL);
#elif defined(linux) || defined(unix)
            sem_post(&stream_frame_info->image_done_sem);
#endif
            ir_camera_stream_off(stream_frame_info);
            printf("uvc_frame_get failed\n ");
            return NULL;
        }
        if (stream_frame_info->raw_frame != NULL)
        {
            raw_data_cut((uint8_t*)stream_frame_info->raw_frame, stream_frame_info->image_byte_size, \
                        stream_frame_info->temp_byte_size, (uint8_t*)stream_frame_info->image_frame, \
                        (uint8_t*)stream_frame_info->temp_frame);
        }
#if defined(_WIN32)
        ReleaseSemaphore(stream_frame_info->image_done_sem, 1, NULL);
#elif defined(linux) || defined(unix)
        sem_post(&stream_frame_info->image_done_sem);
#endif
        //printf("raw data\n");
        i++;
    }

#if defined(_WIN32)
    ReleaseSemaphore(stream_frame_info->image_sem, 1, NULL);
#elif defined(linux) || defined(unix)
    sem_post(&stream_frame_info->image_sem);

#endif

    ir_camera_stream_off(stream_frame_info);
    
    printf("stream thread exit!!\n");
    return NULL;
}
