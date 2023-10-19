#include "cmd.h"

//command selection
void command_sel(StreamFrameInfo_t* handle, int cmd_type)
{
	int rst = 0;
	uint8_t id_data[100] = { 0 };
	float zoom_factor = 0;
	uint16_t temp_data = 0;
	IrcmdPoint_t point_1 = { 320,256 };
	IrcmdLine_t line1 = { 100, 100, 300, 300 };
	IrcmdRect_t rect1 = { 0, 0, 383, 287 };
	TpdLineRectTempInfo_t line1_temp_info = { 0 };
	TpdLineRectTempInfo_t rect1_temp_info = { 0 };
	FrameTempInfo_t frame_temp_info = { 0 };
	IrcmdPoint_t point_pos = { 100,100 };
	uint16_t point_temp_value = 0;
	uint16_t value=0;
	uint8_t* pbyData = NULL;
	FILE* fp = NULL;
	uint8_t oem_data[480] = { 0 };
	switch (cmd_type)
	{
	case 0:
		basic_get_device_info(handle->ircmd_handle, BASIC_DEV_INFO_GET_PN, id_data);
		printf("get device PN:");
		for (int j = 0; j < 8; j++)
		{
			printf("0x%x ", id_data[j]);
		}
		printf("\n");
		break;
	case 1:
		basic_restore_def_algorithm_param(handle->ircmd_handle);
		printf("restore default algorithm params\n");
		break;
	case 2:
		basic_algorithm_param_save(handle->ircmd_handle);
		printf("save algorithm params\n");
		break;
	case 3:
		basic_y16_preview(handle->ircmd_handle, BASIC_Y16_MODE_YUV);
		printf("y16 preview mode YUV\n");
		break;
	case 4:
		basic_preview_yuv_format_set(handle->ircmd_handle, BASIC_PREVIEW_UYVY);
		printf("preview yuv format set:YUYV\n");
		break;
	case 5:
		zoom_factor = 1.5;
		basic_zoom_center_factor_set(handle->ircmd_handle, zoom_factor);
		printf("zoom_center_up\n");
		break;
	case 6:
		value = 3;
		basic_image_mirror_flip(handle->ircmd_handle, SET_PARAMS_STA, &value);
		printf("image_mirror_flip\n");
		break;
	case 7:
		value = 3;
		basic_image_params_dde(handle->ircmd_handle, SET_PARAMS_STA, &value);
		printf("image_params_dde\n");
		break;
	case 8:
		value = 200;
		basic_image_params_brightness(handle->ircmd_handle, SET_PARAMS_STA, &value);
		printf("image_params_dde\n");
		break;
	case 9:
		value = 200;
		basic_image_params_contrast(handle->ircmd_handle, SET_PARAMS_STA, &value);
		printf("image_params_contrast\n");
		break;
	case 10:
		basic_auto_shutter_onoff(handle->ircmd_handle, GET_PARAMS_STA, &value);
		printf("auto_shutter_onoff\n");
		printf("value =%d\n", value);
		break;
	case 11:
		value = 6;
		basic_auto_shutter_min_interval(handle->ircmd_handle, GET_PARAMS_STA, &value);
		printf("auto_shutter_min_interval\n");
		printf("value =%d\n", value);
		break;
	case 12:
		value = 6;
		basic_auto_shutter_max_interval(handle->ircmd_handle, GET_PARAMS_STA, &value);
		printf("auto_shutter_max_interval\n");
		printf("value =%d\n", value);
		break;
	case 13:
		value = 15;
		basic_auto_shutter_vtemp_threshold(handle->ircmd_handle, SET_PARAMS_STA, &value);
		printf("basic_auto_shutter_vtemp_threshold\n");
		break;
	case 14:
		basic_shutter_update(handle->ircmd_handle);
		printf("Manual shutter\n");
		break;
	case 15:
		value = 6;
		basic_pseudo_color(handle->ircmd_handle, SET_PARAMS_STA, &value);
		printf("pseudo color set:6\n");
		break;
	case 16:
		basic_tpd_get_point_temp_info(handle->ircmd_handle, point_pos, &point_temp_value);
		printf("tpd_get_point_temp_info:%.2f\n", (point_temp_value / 16 - 273.15));
		break;
	case 17:
		basic_tpd_get_line_temp_info(handle->ircmd_handle, line1, &line1_temp_info);
		printf("tpd_get_line_temp_info:min(%d,%d):%.2f, max(%d,%d):%.2f\n", \
			line1_temp_info.min_temp_point.x, line1_temp_info.min_temp_point.y, \
			(line1_temp_info.temp_info_value.min_temp / 16 - 273.15), \
			line1_temp_info.max_temp_point.x, line1_temp_info.max_temp_point.y, \
			(line1_temp_info.temp_info_value.max_temp / 16 - 273.15));
		break;
	case 18:
		basic_tpd_get_rect_temp_info(handle->ircmd_handle, rect1, &rect1_temp_info);
		printf("tpd_get_rect_temp_info:min(%d,%d):%.2f, max(%d,%d):%.2f\n", \
			rect1_temp_info.min_temp_point.x, rect1_temp_info.min_temp_point.y, \
			(rect1_temp_info.temp_info_value.min_temp / 16 - 273.15), \
			rect1_temp_info.max_temp_point.x, rect1_temp_info.max_temp_point.y, \
			(rect1_temp_info.temp_info_value.max_temp / 16 - 273.15));
		break;
	case 19:
		basic_tpd_get_frame_temp_info(handle->ircmd_handle, &frame_temp_info);
		printf("tpd_get_frame_temp_info:min(%d,%d):%.2f, max(%d,%d):%.2f\n", \
			frame_temp_info.min_temp_point.x, frame_temp_info.min_temp_point.y, \
			(frame_temp_info.min_temp / 16 - 273.15), \
			frame_temp_info.max_temp_point.x, frame_temp_info.max_temp_point.y, \
			(frame_temp_info.max_temp / 16 - 273.15));
		break;
	case 20:
		value = 1;
		basic_prop_tpd_sel_gain(handle->ircmd_handle, SET_PARAMS_STA, &value);
		printf("basic_prop_tpd_sel_gain\n");
		break;
	case 21:
		adv_image_params_agc_mode(handle->ircmd_handle, GET_PARAMS_STA, &value);
		printf("advanced_image_params_agc_mode\n");
		printf("value =%d\n", value);
		break;
	case 22:
		adv_rmcover_clear_data(handle->ircmd_handle);
		printf("clear rmcover data\n");
		break;
	case 23:
		adv_dpc_auto_calc(handle->ircmd_handle);
		printf("dpc auto calc\n");
		break;
	case 24:
		adv_dpc_add_point(handle->ircmd_handle, point_1);
		printf("add point as dpc\n");
		break;
	case 25:
		adv_dpc_remove_point(handle->ircmd_handle, point_1);
		printf("remove point from dpc\n");
		break;
	case 26:
		adv_dpc_save(handle->ircmd_handle);
		printf("dpc save to flash\n");
		break;
	case 27:
		adv_dpc_clear_data(handle->ircmd_handle);
		printf("clean dpc data from flash\n");
		break;
	case 28:
		temp_data = 300;
		adv_tpd_ktbt_recal_1point(handle->ircmd_handle, temp_data);
		printf(" recalculate the kt_bt by 1 point's temperature\n");
		break;
	case 29:
		temp_data = 300;
		adv_tpd_ktbt_recal_2point(handle->ircmd_handle, TPD_KTBT_RECAL_P1, temp_data);
		printf(" recalculate the kt_bt by 2 point's temperature\n");
		break;
	case 30:
		adv_restore_default_cfg(handle->ircmd_handle, ADV_CFG_TPD);
		printf("restore default tpd params\n");
		break;
	case 31:
		pbyData = (uint8_t*)malloc(8192);
		adv_read_error_log(handle->ircmd_handle, pbyData);
		printf("read fw error log\n");
		fp = fopen("aim_image.bin", "wb");
		fwrite(pbyData, 1, 8192, fp);
		fclose(fp);
		break;
	case 32:
		adv_oem_erase(handle->ircmd_handle);
		printf("erase data in flash's OEM section\n");
		break;
	case 33:
		rst = adv_oem_read(handle->ircmd_handle, oem_data);
		printf("rst = %d\n", rst);
		printf("read data in flash's OEM section\n");
		for (int i = 0; i < 8; i++)
		{
			printf("oem_data[%d] = %d\n", i, oem_data[i]);
		}
		break;
	case 34:
		for (int i = 0; i < 8; i++)
		{
			oem_data[i]= i;
		}
		adv_oem_write(handle->ircmd_handle, oem_data);
		printf("write data in flash's OEM section\n");
		break;
	case 35:
		basic_preview_mode_select(handle->ircmd_handle, BASIC_USB_MODE, BASIC_FRAME_COMPOSITE_DATA);
		printf("basic_preview_mode_select\n");
		break;
	case 36:
		basic_preview_mode_select(handle->ircmd_handle, BASIC_USB_MODE, BASIC_SINGLE_IMAGE_OR_TEMP);
		printf("basic_preview_mode_select\n");
		break;
	case 37:
		adv_rmcover_auto_calc(handle->ircmd_handle);
		printf("auto calibrate cover\n");
		break;
	default:
		break;
	}
}

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
