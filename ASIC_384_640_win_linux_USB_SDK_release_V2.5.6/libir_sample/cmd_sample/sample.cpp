#include "sample.h"

#ifdef __cplusplus
extern "C" {
#endif



int frame_idx = 0;

//load the stream frame info
void load_stream_frame_info(StreamFrameInfo_t* stream_frame_info)
{
#if defined(IMAGE_AND_TEMP_OUTPUT)
    {
        stream_frame_info->image_info.width = stream_frame_info->camera_param.width;
        stream_frame_info->image_info.height = stream_frame_info->camera_param.height / 2;
        stream_frame_info->image_info.rotate_side = NO_ROTATE;
        stream_frame_info->image_info.mirror_flip_status = STATUS_NO_MIRROR_FLIP;
        stream_frame_info->image_info.pseudo_color_status = PSEUDO_COLOR_OFF;
        stream_frame_info->image_info.img_enhance_status = IMG_ENHANCE_OFF;
        stream_frame_info->image_info.input_format = INPUT_FMT_YUV422; //only Y14 or Y16 mode can use enhance and pseudo color
        stream_frame_info->image_info.output_format = OUTPUT_FMT_BGR888; //if display on opencv,please select BGR888

        stream_frame_info->temp_info.width = stream_frame_info->camera_param.width;
        stream_frame_info->temp_info.height = stream_frame_info->camera_param.height / 2;
        stream_frame_info->temp_info.rotate_side = NO_ROTATE;
        stream_frame_info->temp_info.mirror_flip_status = STATUS_NO_MIRROR_FLIP;
        stream_frame_info->image_byte_size = stream_frame_info->image_info.width * stream_frame_info->image_info.height * 2;
        stream_frame_info->temp_byte_size = stream_frame_info->image_info.width * stream_frame_info->image_info.height * 2;//no temp frame input
    }
#elif defined(IMAGE_OUTPUT) || defined(TEMP_OUTPUT)
    stream_frame_info->image_info.width = stream_frame_info->camera_param.width;
    stream_frame_info->image_info.height = stream_frame_info->camera_param.height;
    stream_frame_info->image_info.rotate_side = NO_ROTATE;
    stream_frame_info->image_info.mirror_flip_status = STATUS_NO_MIRROR_FLIP;
    stream_frame_info->image_info.pseudo_color_status = PSEUDO_COLOR_OFF;
    stream_frame_info->image_info.img_enhance_status = IMG_ENHANCE_OFF;
    stream_frame_info->image_info.input_format = INPUT_FMT_YUV422; //only Y14 or Y16 mode can use enhance and pseudo color
    stream_frame_info->image_info.output_format = OUTPUT_FMT_BGR888; //if display on opencv,please select BGR888

    stream_frame_info->image_byte_size = stream_frame_info->image_info.width * stream_frame_info->image_info.height * 2;
    stream_frame_info->temp_byte_size = 0;
#endif
    create_data_demo(stream_frame_info);
}

void log_level_register(log_level_t log_level)
{
    switch(log_level)
    {
        case(DEBUG_PRINT):
        {
            ircmd_log_register(IRCMD_LOG_DEBUG);
            iruvc_log_register(IRUVC_LOG_DEBUG);
            irproc_log_register(IRPROC_LOG_DEBUG);
            irparse_log_register(IRPARSE_LOG_DEBUG);
            break;
        }
        case(ERROR_PRINT):
        {
            ircmd_log_register(IRCMD_LOG_ERROR);
            iruvc_log_register(IRUVC_LOG_ERROR);
            irproc_log_register(IRPROC_LOG_ERROR);
            irparse_log_register(IRPARSE_LOG_ERROR);
            break;
        }
        case(NO_PRINT):
        default:
        {
            ircmd_log_register(IRCMD_LOG_NO_PRINT);
            iruvc_log_register(IRUVC_LOG_NO_PRINT);
            irproc_log_register(IRPROC_LOG_NO_PRINT);
            irparse_log_register(IRPARSE_LOG_NO_PRINT);
            break;
        }
    }

}


void print_and_record_version(void)
{
    puts(SAMPLE_VERSION);
    puts(irproc_version());
    puts(irparse_version());
    puts(iruvc_version());
    puts(ircmd_version());
#if defined(_WIN32)
    FILE* fp = fopen(".\\libs_version.txt", "wb");
#elif defined(linux) || defined(unix)
    FILE* fp = fopen("./libs_version.txt", "wb");
#endif
    fputs(SAMPLE_VERSION, fp);fputs("\n", fp);
    fputs(irproc_version(), fp);fputs("\n", fp);
    fputs(irparse_version(), fp);fputs("\n", fp);
    fputs(iruvc_version(), fp);fputs("\n", fp);
    fputs(ircmd_version(), fp); fputs("\n", fp);
    fclose(fp);
}


//user's call back function
void usr_test_func(void* frame, void* usr_param)
{
    //write your own callback code here

    char title[80];
    int same_idx = 0;
    StreamFrameInfo_t* stream_frame_info;
    stream_frame_info = (StreamFrameInfo_t*)usr_param;

    if (stream_frame_info == NULL)
    {
        return;
    }

    if ((frame != NULL) && (stream_frame_info->raw_frame != NULL))
    {
        memcpy(stream_frame_info->raw_frame, frame, stream_frame_info->camera_param.frame_size);
    }

    if (stream_frame_info->raw_frame != NULL)
    {
        raw_data_cut((uint8_t*)stream_frame_info->raw_frame, stream_frame_info->image_byte_size, \
            stream_frame_info->temp_byte_size, (uint8_t*)stream_frame_info->image_frame, \
            (uint8_t*)stream_frame_info->temp_frame);
        same_idx = iruvc_get_same_idx(stream_frame_info->iruvc_handle);
        sprintf(title, "Test%d", same_idx);
        display_one_frame(stream_frame_info, title);
    }

    //printf("test_func:%d\n", ((unsigned short*)frame)[10]);
    printf("frame_idx:%d\n", frame_idx);
    frame_idx++;
}

void* thread_function(void* threadarg)
{
    int rst;
    StreamFrameInfo_t *stream_frame_info = (StreamFrameInfo_t*)threadarg;
    //user function callback mode
#ifdef USER_FUNCTION_CALLBACK
    display_init(stream_frame_info);
    rst = ir_camera_stream_on_with_callback(stream_frame_info, usr_test_func);

    if (rst < 0)
    {
        puts("ir camera stream on failed!\n");
        getchar();
        return 0;
    }
    ir_camera_stream_off_with_callback(stream_frame_info);
    display_release(stream_frame_info);
#else
    //multiple thread function mode
    int whther_temp_output = 0;
#if defined(TEMP_OUTPUT)
    whther_temp_output = 1;
#endif
    rst = ir_camera_stream_on(stream_frame_info, whther_temp_output);
    if (rst < 0)
    {
        puts("ir camera stream on failed!\n");
        getchar();
        return 0;
    }

    pthread_t tid_stream, tid_display, tid_temperature;

    pthread_create(&tid_display, NULL, display_function, stream_frame_info);
    pthread_create(&tid_stream, NULL, stream_function, stream_frame_info);

    pthread_join(tid_display, NULL);
    pthread_join(tid_stream, NULL);
#endif
    return 0;
}

int main(void)
{
    //set priority to highest level
#if defined(_WIN32)
    SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
#elif defined(linux) || defined(unix)
    setpriority(PRIO_PROCESS, 0, -20);
#endif

    //version
    print_and_record_version();
    log_level_register(ERROR_PRINT);

  
    int rst, same_idx, resolution_idx;

#if defined(IMAGE_AND_TEMP_OUTPUT)
    resolution_idx = 1;
#elif defined(IMAGE_OUTPUT) || defined(TEMP_OUTPUT)
    resolution_idx = 0;
#endif

    //open camera 0
    same_idx = 0;
    StreamFrameInfo_t stream_frame_info1 = { 0 };
    IruvcHandle_t* iruvc_handle1 = iruvc_create_handle();
    printf("thread_function same index:%d\n", same_idx);
    rst = ir_camera_open(iruvc_handle1, &stream_frame_info1.camera_param, same_idx, resolution_idx);
    if (rst < 0)
    {
        puts("ir camera open failed!\n");
        getchar();
        return 0;
    }
    load_stream_frame_info(&stream_frame_info1);
    stream_frame_info1.iruvc_handle = iruvc_handle1;
    stream_frame_info1.ircmd_handle = ircmd_create_handle(iruvc_handle1, VDCMD_I2C_USB_VDCMD);
    
    pthread_t tid1, tid2, tid_cmd1, tid_cmd2;
    pthread_create(&tid1, NULL, thread_function, (void*)&stream_frame_info1);

    pthread_create(&tid_cmd1, NULL, cmd_function, (void*)&stream_frame_info1);
    pthread_join(tid1, NULL);
    pthread_cancel(tid_cmd1);
    iruvc_camera_close(iruvc_handle1);

    puts("EXIT");
    getchar();
    return 0;
}

#ifdef __cplusplus
}
#endif
