/*
 * Display.cpp
 *
 *  Created on: 26 Nov 2022
 *      Author: David
 */

#include "Display.h"
#include <lvgl.h>

constexpr unsigned int DISP_HOR_RES = 800;
constexpr unsigned int DISP_VER_RES = 480;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf1[DISP_HOR_RES * DISP_VER_RES / 10];                        /*Declare a buffer for 1/10 screen size*/

void InitDisplay() noexcept
{
	lv_disp_draw_buf_init(&draw_buf, buf1, nullptr, DISP_HOR_RES * DISP_VER_RES / 10);  /*Initialize the display buffer.*/
}

// End
