# libirsample示例程序结构以及使用流程介绍



## 一、程序结构

![](./irsample_structure.png)

如图，sample分为了sample、camera、display、cmd等模块，每个模块作用如下：

**sample模块**：在sample.cpp中配置好了相关参数之后，调用camera模块，与红外机芯建立连接，并控制出图。之后sample会创建stream、display、cmd这三个线程用于对应信息的处理。

**camera模块**：用于获取机芯信息，当stream线程获取到原始红外帧信息的时候，会将红外帧信息raw frame切分为图像信息image frame和温度信息temp frame，并发送信号，对image frame进行处理，当image frame处理完成后发送信号给camera线程，camera线程继续下一次循环。

**display模块**：获取图像帧信息之后，根据之前frame_info里参数的设定，做图像数据格式转换、翻转/镜像、旋转等处理，最后调用opencv显示出来图像。

**cmd模块**：控制发送对应的命令给红外机芯。



## 二、程序编译方式

在windows平台，libir_sample文件夹下已经提供了一个完整的VS工程，运行示例需要opencv库和pthreadVC2.dll（已经放入）。

在linux平台，libir_sample文件夹下提供了 `Makefile` 和`CMakeLists.txt`文件，在编译时需要删除opencv2文件夹（Linux需要另行安装）。如果不需要opencv，可以在display.h文件中，注释掉`#define OPENCV_ENABLE`，并在 `Makefile` 或`CMakeLists.txt`中注释掉opencv相关内容，然后再编译。



## 三、程序使用流程

### 1.连接机芯

在sample.cpp的main函数中，通过调用ir_camera_open来选择对应的机芯，并从机芯获取相关的参数信息stream_frame_info。

在获取到参数信息之后，还需要调用load_stream_frame_info函数来补充对display和temperature两个模块的设置，比如宽高信息，旋转/镜像/翻转设置，是否需要调用库中自带的伪彩映射表，输入格式和输出格式，申请buffer空间等。

```c
        stream_frame_info->image_info.width = stream_frame_info->camera_param.width;
        stream_frame_info->image_info.height = stream_frame_info->camera_param.height / 2;
        stream_frame_info->image_info.rotate_side = LEFT_90D;
        stream_frame_info->image_info.mirror_flip_status = STATUS_MIRROR_FLIP;
        stream_frame_info->image_info.pseudo_color_status = PSUEDO_COLOR_ON;
		stream_frame_info->image_info.img_enhance_status = IMG_ENHANCE_OFF;
        stream_frame_info->image_info.input_format = INPUT_FMT_YUV422; 	//only Y14 or Y16 mode can use enhance and pseudo color
        stream_frame_info->image_info.output_format = OUTPUT_FMT_BGR888; //if display on opencv,please select BGR888

        stream_frame_info->temp_info.width = stream_frame_info->camera_param.width;
        stream_frame_info->temp_info.height = stream_frame_info->camera_param.height / 2;
        stream_frame_info->temp_info.rotate_side = NO_ROTATE;
        stream_frame_info->temp_info.mirror_flip_status = STATUS_NO_MIRROR_FLIP;
        stream_frame_info->image_byte_size = stream_frame_info->image_info.width * stream_frame_info->image_info.height * 2;
        stream_frame_info->temp_byte_size = 0;//no temp frame input
```



如下是StreamFrameInfo_t结构体的定义，注意这几项参数：

FrameInfo_t-width/height:宽高参数，需要填写

FrameInfo_t-byte_size:frame_info当前的字节大小，在出图显示的时候，display.cpp的display_image_process函数里会根据数据格式来计算填写

FrameInfo_t-rotate_side/mirror_flip_status:翻转/镜像、旋转等状态，需要填写

FrameInfo_t-input_format/output_format:输入和输出的数据格式，比如Y14数据输入，RGB888输出，需要填写

FrameInfo_t-pseudo_color_status:伪彩色开关，如果打开会调用libirprocess库内的伪彩映射表，需要填写

FrameInfo_t-img_enhance_status:图像拉伸开关，如果打开会调用libirprocess库内的图像拉伸算法，将Y14/Y16数据拉伸，需要填写

StreamFrameInfo_t-image_byte_size/temp_byte_size:输入帧数据的字节大小，填0即收不到数据，根据需要切分的数据大小来填写

```c
typedef struct {
    uint32_t width;
    uint32_t height;
    uint32_t byte_size;
    RotateSide_t rotate_side;
    MirrorFlipStatus_t mirror_flip_status;
    InputFormat_t  input_format;
    OutputFormat_t  output_format;
    PsuedoColor_t  pseudo_color_status;
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
    time_t timer;
}StreamFrameInfo_t;
```



### 2.控制出图

在打开设备，获取到相关的参数信息，并补充完对display模块的参数设置之后，就可以调用ir_camera_stream_on或ir_camera_stream_on_with_callback（打开宏USER_FUNCTION_CALLBACK，就可以调用自定义回调函数）来出图了。display.cpp的display_function中，在等待到一帧的信号之后调用display_one_frame。

```c
    while (is_streaming && (i <= STREAM_TIME * stream_frame_info->camera_param.fps))//display stream_time seconds
    {
#if defined(_WIN32)
        WaitForSingleObject(image_done_sem, INFINITE);
#elif defined(linux) || defined(unix)
        sem_wait(&image_done_sem);
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
        while (r < 0 && overtime_cnt >= overtime_threshold)
        {
#if defined(_WIN32)
            ReleaseSemaphore(image_sem, 1, NULL);
#elif defined(linux) || defined(unix)
            sem_post(&image_sem);
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
        ReleaseSemaphore(image_sem, 1, NULL);
#elif defined(linux) || defined(unix)
        sem_post(&image_sem);
#endif
        //printf("raw data\n");
        i++;
    }
```



在display_one_frame函数中，会调用display_image_process来处理图像数据格式，根据stream_frame_info->image_info的输入输出的数据格式input_format、output_format，以及伪彩设置pseudo_color_status，来调用libirparse库做对应的转换。然后再根据stream_frame_info->image_info的mirror_flip_status和rotate_side等状态，做对应的旋转、镜像、翻转操作。最后用opencv的imshow函数显示出来。

```c
	display_image_process(stream_frame_info->image_frame, pix_num, &stream_frame_info->image_info);
	if ((stream_frame_info->image_info.rotate_side == LEFT_90D)|| \
		(stream_frame_info->image_info.rotate_side == RIGHT_90D))
	{
		width = stream_frame_info->image_info.height;
		height = stream_frame_info->image_info.width;
	}

	mirror_flip_demo(&stream_frame_info->image_info, image_tmp_frame2, \
					stream_frame_info->image_info.mirror_flip_status);
	rotate_demo(&stream_frame_info->image_info, image_tmp_frame2, \
				stream_frame_info->image_info.rotate_side);

#ifdef OPENCV_ENABLE
	cv::Mat image = cv::Mat(height, width, CV_8UC3, image_tmp_frame2);
	putText(image, frameText, cv::Point(11, 11), cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar::all(0), 1, 8);
	putText(image, frameText, cv::Point(10, 10), cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar::all(255), 1, 8);
	cv::imshow(title, image);
	cvWaitKey(5);
#endif
```



在stream_function函数中，可以通过设置stream_time的数值来控制出图的时长，这边是循环出图100*fps帧，在没有丢帧、超时的时候，一般出图时长就是100秒。

        int stream_time = 100;  //unit:s
        while (is_streaming && (i <= stream_time * fps))//display stream_time seconds
        {
    #if defined(_WIN32)
            WaitForSingleObject(image_done_sem, INFINITE);
    #elif defined(linux) || defined(unix)
            sem_wait(&image_done_sem);
    #endif
            r = uvc_frame_get(stream_frame_info->raw_frame);



### 3.发送命令

在cmd.cpp的command_sel函数中，在输入不同数字后，触发对应的命令。

```c
//command thread function
void* cmd_function(void* threadarg)
{
	int cmd = 0;
	while (cmd != 999)
	{
		scanf("%d", &cmd);
		command_sel(((StreamFrameInfo_t*)threadarg), cmd);
	}
	printf("cmd thread exit!!\n");
	return NULL;
}
```



### 4.结束程序

在sample的`ir_camera_stream_off`函数里，调用`destroy_pthread_sem`以关闭信号。调用`iruvc_camera_close`来关闭设备连接。

