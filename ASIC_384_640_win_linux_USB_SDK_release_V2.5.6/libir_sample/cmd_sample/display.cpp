#include "display.h"

//PseudocolorYuv_t psdcolor_yuv_mapping_table[] = yuv_8bit_pseudocolor_table;
//PseudocolorRgb_t psdcolor_rgb_mapping_table[] = rgb_8bit_pseudocolor_table;

#if defined(_WIN32)
int gettimeofday(struct timeval* tp, struct timezone* tzp)
{
	// Note: some broken versions only have 8 trailing zero's, the correct epoch has 9 trailing zero's
	// This magic number is the number of 100 nanosecond intervals since January 1, 1601 (UTC)
	// until 00:00:00 January 1, 1970 
	static const uint64_t EPOCH = ((uint64_t)116444736000000000ULL);

	SYSTEMTIME  system_time;
	FILETIME    file_time;
	uint64_t    time;

	GetSystemTime(&system_time);
	SystemTimeToFileTime(&system_time, &file_time);
	time = ((uint64_t)file_time.dwLowDateTime);
	time += ((uint64_t)file_time.dwHighDateTime) << 32;

	tp->tv_sec = (long)((time - EPOCH) / 10000000L);
	tp->tv_usec = (long)(system_time.wMilliseconds * 1000);
	return 0;
}
#endif 

//init the display parameters
void display_init(StreamFrameInfo_t* stream_frame_info)
{
	int pixel_size = stream_frame_info->image_info.width * stream_frame_info->image_info.height;
	if (stream_frame_info->image_tmp_frame1 == NULL)
	{
		stream_frame_info->image_tmp_frame1 = (uint8_t*)malloc(pixel_size * 3);
	}
	if (stream_frame_info->image_tmp_frame2 == NULL)
	{
		stream_frame_info->image_tmp_frame2 = (uint8_t*)malloc(pixel_size * 3);
	}
}

//recyle the display parameters
void display_release(StreamFrameInfo_t* stream_frame_info)
{
	if (stream_frame_info->image_tmp_frame1 != NULL)
	{
		free(stream_frame_info->image_tmp_frame1);
		stream_frame_info->image_tmp_frame1 = NULL;
	}

	if (stream_frame_info->image_tmp_frame2 != NULL)
	{
		free(stream_frame_info->image_tmp_frame2);
		stream_frame_info->image_tmp_frame2 = NULL;
	}
}

//enhance the image frame by the frameinfo
int enhance_image_frame(uint16_t* src_frame, FrameInfo_t* frameinfo, uint16_t* dst_frame)
{
	int lower_limit, upper_limit = 0;
	int temp_enhace_en = 1;
	float lower_temp = 10;
	float upper_temp = 35.5;

	lower_limit = (lower_temp + 273.15) * 16;
	upper_limit = (upper_temp + 273.15) * 16;

	ImgEnhanceParam_t img_enhance_param = { { 0.01,0.01,0.001,0.0001,150,0,0.25,\
					  {temp_enhace_en,lower_limit,upper_limit} }, { 0,6,2,0,10,60,120 } };
	ImageRes_t image_res = { frameinfo->width,frameinfo->height };
	int pix_num = frameinfo->width * frameinfo->height;

	if (frameinfo->img_enhance_status == IMG_ENHANCE_ON)
	{
		y14_image_enhance((uint16_t*)src_frame, image_res, img_enhance_param, (uint16_t*)dst_frame);
	}
	else
	{
		memcpy(dst_frame, src_frame, pix_num * 2);
	}
	return 0;
}

//color the image frame
void color_image_frame(uint8_t* src_frame, FrameInfo_t* frameinfo, uint8_t* dst_frame)
{
	int pix_num = frameinfo->width * frameinfo->height;
	uint8_t* tmp_frame = (uint8_t*)malloc(pix_num * 3);
	free(tmp_frame);
}

//convert the image process  image_tmp_frame2 is the default output frame
void display_image_process(StreamFrameInfo_t* stream_frame_info, int pix_num, FrameInfo_t* frameinfo)
{
	ImageRes_t image_res = { frameinfo->width,frameinfo->height };
	uint8_t* image_frame = stream_frame_info->image_frame;
	if (image_frame == NULL)
	{
		//printf("image is NULL\n");
		return;
	}

	if (frameinfo->input_format == INPUT_FMT_Y14 || frameinfo->input_format == INPUT_FMT_Y16)
	{
		if (frameinfo->input_format == INPUT_FMT_Y16)
		{
			y16_to_y14((uint16_t*)image_frame, pix_num, (uint16_t*)image_frame);
		}

		enhance_image_frame((uint16_t*)image_frame, frameinfo, \
							(uint16_t*)stream_frame_info->image_tmp_frame1);

		switch (frameinfo->output_format)
		{
			case OUTPUT_FMT_Y14:
			{
				frameinfo->byte_size = pix_num * 2;
				memcpy(stream_frame_info->image_tmp_frame2, stream_frame_info->image_tmp_frame1, \
						frameinfo->byte_size);
				break;
			}
			case OUTPUT_FMT_YUV444:
			{
				frameinfo->byte_size = pix_num * 3;
				y14_to_yuv444((uint16_t *)stream_frame_info->image_tmp_frame1, pix_num, \
								(uint8_t*)stream_frame_info->image_tmp_frame2);
				break;
			}
			case OUTPUT_FMT_YUV422:
			{
				frameinfo->byte_size = pix_num * 2;
				if (frameinfo->pseudo_color_status == PSEUDO_COLOR_ON)
				{
					// use IRPROC_COLOR_MODE_3 mode
					y14_map_to_yuyv_pseudocolor((uint16_t*)stream_frame_info->image_tmp_frame1, pix_num, \
												IRPROC_COLOR_MODE_3, (uint8_t*)stream_frame_info->image_tmp_frame2);
				}
				else
				{
					y14_to_yuv444((uint16_t*)stream_frame_info->image_tmp_frame1, pix_num, \
						(uint8_t*)stream_frame_info->image_tmp_frame2);
					memcpy(stream_frame_info->image_tmp_frame1, stream_frame_info->image_tmp_frame2, \
							frameinfo->byte_size);
					yuv444_to_yuv422((uint8_t*)stream_frame_info->image_tmp_frame1, pix_num, \
						(uint8_t*)stream_frame_info->image_tmp_frame2);
				}
				break;
			}
			case OUTPUT_FMT_RGB888:
			{
				frameinfo->byte_size = pix_num * 3;
				if (frameinfo->pseudo_color_status == PSEUDO_COLOR_ON)
				{
					y14_map_to_yuyv_pseudocolor((uint16_t*)stream_frame_info->image_tmp_frame1, pix_num, \
												IRPROC_COLOR_MODE_3, stream_frame_info->image_tmp_frame2);
					memcpy(stream_frame_info->image_tmp_frame1, stream_frame_info->image_tmp_frame2, \
							frameinfo->byte_size);
					yuv422_to_rgb(stream_frame_info->image_tmp_frame1, pix_num, \
								stream_frame_info->image_tmp_frame2);
				}
				else
				{
					y14_to_rgb((uint16_t*)stream_frame_info->image_tmp_frame1, pix_num, \
								stream_frame_info->image_tmp_frame2);
				}
				break;
			}
			case OUTPUT_FMT_BGR888:
			default:
			{
				frameinfo->byte_size = pix_num * 3;
				if (frameinfo->pseudo_color_status == PSEUDO_COLOR_ON)
				{
					y14_map_to_yuyv_pseudocolor((uint16_t*)stream_frame_info->image_tmp_frame1, pix_num, \
												IRPROC_COLOR_MODE_6, stream_frame_info->image_tmp_frame2);
					yuv422_to_rgb(stream_frame_info->image_tmp_frame2, pix_num, \
								stream_frame_info->image_tmp_frame1);
					rgb_to_bgr(stream_frame_info->image_tmp_frame1, pix_num, \
							stream_frame_info->image_tmp_frame2);
				}
				else
				{
					y14_to_rgb((uint16_t*)stream_frame_info->image_tmp_frame1, pix_num, \
								stream_frame_info->image_tmp_frame2);
					rgb_to_bgr(stream_frame_info->image_tmp_frame2, pix_num, \
								stream_frame_info->image_tmp_frame2);
				}
				break;
			}
		}
	}
	else if (frameinfo->input_format == INPUT_FMT_YUV422)
	{
		switch (frameinfo->output_format)
		{
			case OUTPUT_FMT_Y14:
			{
				frameinfo->byte_size = 0;
				printf("convert error!\n");
				break;
			}
			case OUTPUT_FMT_YUV444:
			{
				frameinfo->byte_size = 0;
				printf("convert error!\n");
				break;
			}
			case OUTPUT_FMT_YUV422:
			{
				frameinfo->byte_size = pix_num * 2;
				memcpy(stream_frame_info->image_tmp_frame2, image_frame, pix_num * 2);
				break;
			}
			case OUTPUT_FMT_RGB888:
			{
				frameinfo->byte_size = pix_num * 3;
				yuv422_to_rgb((uint8_t*)image_frame, pix_num, stream_frame_info->image_tmp_frame2);
				break;
			}
			case OUTPUT_FMT_BGR888:
			default:
			{
				frameinfo->byte_size = pix_num * 3;
				yuv422_to_rgb((uint8_t*)image_frame, pix_num, stream_frame_info->image_tmp_frame1);
				rgb_to_bgr(stream_frame_info->image_tmp_frame1, pix_num, stream_frame_info->image_tmp_frame2);
				break;
			}
		}
	}
}

irproc_src_fmt_t format_converter(OutputFormat_t output_format)
{
	switch (output_format)
	{
	case OUTPUT_FMT_Y14:
		return IRPROC_SRC_FMT_Y14;
		break;
	case OUTPUT_FMT_YUV422:
		return IRPROC_SRC_FMT_YUV422;
		break;
	case OUTPUT_FMT_YUV444:
		return IRPROC_SRC_FMT_YUV444;
		break;
	case OUTPUT_FMT_RGB888:
		return IRPROC_SRC_FMT_RGB888;
		break;
	case OUTPUT_FMT_BGR888:
		return IRPROC_SRC_FMT_BGR888;
		break;
	default:
		return IRPROC_SRC_FMT_Y14;
		break;
	}
}

//rotate the frame data according to rotate_side
void rotate_demo(uint8_t* frame, FrameInfo_t* frame_info, \
				 RotateSide_t rotate_side, uint8_t* out_frame)
{
	ImageRes_t image_res = { frame_info->width,frame_info->height };
	irproc_src_fmt_t tmp_fmt = format_converter(frame_info->output_format);

	switch (rotate_side)
	{
	case NO_ROTATE:
		memcpy(out_frame, frame, image_res.height * image_res.width * 3);
		break;
	case LEFT_90D:
		rotate_left_90(frame, image_res, tmp_fmt, out_frame);
		break;
	case RIGHT_90D:
		rotate_right_90(frame, image_res, tmp_fmt, out_frame);
		break;
	case ROTATE_180D:
		rotate_180(frame, image_res, tmp_fmt, out_frame);
		break;
	default:
		break;
	}
}

//mirror/flip the frame data according to mirror_flip_status
void mirror_flip_demo(uint8_t* frame, FrameInfo_t* frame_info, \
					  MirrorFlipStatus_t mirror_flip_status, uint8_t* out_frame)
{
	ImageRes_t image_res = { frame_info->width,frame_info->height };
	irproc_src_fmt_t tmp_fmt = format_converter(frame_info->output_format);

	switch (mirror_flip_status)
	{
	case STATUS_NO_MIRROR_FLIP:
		memcpy(out_frame, frame, image_res.height * image_res.width * 3);
		break;
	case STATUS_ONLY_MIRROR:
		mirror(frame, image_res, tmp_fmt, out_frame);
		break;
	case STATUS_ONLY_FLIP:
		flip(frame, image_res, tmp_fmt, out_frame);
		break;
	case STATUS_MIRROR_FLIP:
		mirror(frame, image_res, tmp_fmt, out_frame);
		break;
	default:
		break;
	}
}

//display the frame by opencv 
void display_one_frame(StreamFrameInfo_t* stream_frame_info, const char* title)
{
	if (stream_frame_info == NULL)
	{
		return;
	}

	char key_press = 0;
	int rst = 0;
	struct timeval now_time;
	gettimeofday(&now_time, NULL);
	float frame = 1000000 / (double)((now_time.tv_sec - stream_frame_info->timer.tv_sec)*1000000+\
					(now_time.tv_usec - stream_frame_info->timer.tv_usec));
	memcpy(&stream_frame_info->timer, &now_time, sizeof(now_time));

	char frameText[10] = { " " };
	sprintf(frameText, "%.2f", frame);

	int pix_num = stream_frame_info->image_info.width * stream_frame_info->image_info.height;
	int width = stream_frame_info->image_info.width;
	int height = stream_frame_info->image_info.height;

	display_image_process(stream_frame_info, pix_num, &stream_frame_info->image_info);
	if ((stream_frame_info->image_info.rotate_side == LEFT_90D)|| \
		(stream_frame_info->image_info.rotate_side == RIGHT_90D))
	{
		width = stream_frame_info->image_info.height;
		height = stream_frame_info->image_info.width;
	}

	mirror_flip_demo(stream_frame_info->image_tmp_frame2, &stream_frame_info->image_info, \
					stream_frame_info->image_info.mirror_flip_status, stream_frame_info->image_tmp_frame1);
	memcpy(stream_frame_info->image_tmp_frame2, stream_frame_info->image_tmp_frame1, \
			stream_frame_info->image_info.byte_size);

	rotate_demo(stream_frame_info->image_tmp_frame2, &stream_frame_info->image_info, \
				stream_frame_info->image_info.rotate_side, stream_frame_info->image_tmp_frame1);
	memcpy(stream_frame_info->image_tmp_frame2, stream_frame_info->image_tmp_frame1, \
			stream_frame_info->image_info.byte_size);

#ifdef OPENCV_ENABLE
	cv::Mat image = cv::Mat(height, width, CV_8UC3, stream_frame_info->image_tmp_frame2);
	putText(image, frameText, cv::Point(11, 11), cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar::all(0), 1, 8);
	putText(image, frameText, cv::Point(10, 10), cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar::all(255), 1, 8);
	//cv::namedWindow(title, CV_WINDOW_AUTOSIZE);   // 窗口大小自动适应图片大小，并且不可手动更改。
	//cv::namedWindow(title, CV_WINDOW_NORMAL);	// 用户可以改变这个窗口大小
	cv::imshow(title, image);
	cvWaitKey(5);
#endif
}

//display thread function
void* display_function(void* threadarg)
{
	StreamFrameInfo_t* stream_frame_info;
	stream_frame_info = (StreamFrameInfo_t*)threadarg;
	if (stream_frame_info == NULL)
	{
		return NULL;
	}

	display_init(stream_frame_info);
	int same_idx = iruvc_get_same_idx(stream_frame_info->iruvc_handle);

	int i = 0;
	int timer = 0;
	char title[80];
	struct timeval now_time;
	while (stream_frame_info->is_streaming || (i <= STREAM_TIME * stream_frame_info->camera_param.fps))
	{
#if defined(_WIN32)
		WaitForSingleObject(stream_frame_info->image_done_sem, INFINITE);	//waitting for image singnal 
#elif defined(linux) || defined(unix)
		sem_wait(&stream_frame_info->image_done_sem);
#endif
		sprintf(title, "Test%x %d", stream_frame_info->camera_param.dev_cfg.pid, same_idx);
		display_one_frame(stream_frame_info, title);
#if defined(_WIN32)
		ReleaseSemaphore(stream_frame_info->image_sem, 1, NULL);
#elif defined(linux) || defined(unix)
		sem_post(&stream_frame_info->image_sem);
#endif
		i++;
	}
	display_release(stream_frame_info);
#ifdef OPENCV_ENABLE
	cv::destroyAllWindows();
#endif
	printf("display thread exit!!\n");
	return NULL;
}
