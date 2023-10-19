#include "data.h"


//init the semaphore
int init_pthread_sem(StreamFrameInfo_t* stream_frame_info)
{
#if defined(_WIN32)
	stream_frame_info->image_sem = CreateSemaphore(NULL, 1, 1, NULL);
	stream_frame_info->image_done_sem = CreateSemaphore(NULL, 0, 1, NULL);
#elif defined(linux) || defined(unix)
	sem_init(&stream_frame_info->image_sem, 0, 1);
	sem_init(&stream_frame_info->image_done_sem, 0, 0);
#endif 
	return 0;
}

//recycle the semaphore
int destroy_pthread_sem(StreamFrameInfo_t* stream_frame_info)
{
#if defined(_WIN32)
	CloseHandle(stream_frame_info->image_sem);
	CloseHandle(stream_frame_info->image_done_sem);
#elif defined(linux) || defined(unix)
	sem_destroy(&stream_frame_info->image_sem);
	sem_destroy(&stream_frame_info->image_done_sem);
#endif 
	return 0;
}

//create the raw frame/image frame/temperature frame's buffer
int create_data_demo(StreamFrameInfo_t* stream_frame_info)
{
	if (stream_frame_info != NULL)
	{
		if (stream_frame_info->raw_frame == NULL && stream_frame_info->image_frame == NULL && \
			stream_frame_info->temp_frame == NULL)
		{
			stream_frame_info->raw_frame = (uint8_t*)malloc(stream_frame_info->camera_param.frame_size);
			stream_frame_info->image_frame = (uint8_t*)malloc(stream_frame_info->image_byte_size);
			stream_frame_info->temp_frame = (uint8_t*)malloc(stream_frame_info->temp_byte_size);
		}
	}
	return 0;
}

//recycle the raw frame/image frame/temperature frame's buffer
int destroy_data_demo(StreamFrameInfo_t* stream_frame_info)
{
	if (stream_frame_info != NULL)
	{
		if (stream_frame_info->raw_frame != NULL)
		{
			free(stream_frame_info->raw_frame);
			stream_frame_info->raw_frame = NULL;
		}

		if (stream_frame_info->image_frame != NULL)
		{
			free(stream_frame_info->image_frame);
			stream_frame_info->image_frame = NULL;
		}

		if (stream_frame_info->temp_frame != NULL)
		{
			free(stream_frame_info->temp_frame);
			stream_frame_info->temp_frame = NULL;
		}
	}
	return 0;
}