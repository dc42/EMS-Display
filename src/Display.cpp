/*
 * Display.cpp
 *
 *  Created on: 26 Nov 2022
 *      Author: David
 */

#include <Core.h>
#include "Display.h"
#include "SSD1963.h"

#include <lvgl.h>
#include <src/hal/lv_hal_disp.h>

constexpr unsigned int DISP_HOR_RES = SSD1963_HOR_RES;
constexpr unsigned int DISP_VER_RES = SSD1963_VER_RES;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf1[DISP_HOR_RES * DISP_VER_RES / 10];                        /*Declare a buffer for 1/10 screen size*/
static lv_disp_drv_t disp_drv;				/* Descriptor of a display driver*/

// Initialise the display
void Display::Init() noexcept
{
	SSD1963::Init();
	lv_init();
	lv_disp_draw_buf_init(&draw_buf, buf1, nullptr, DISP_HOR_RES * DISP_VER_RES / 10);  /*Initialize the display buffer.*/
	lv_disp_drv_init(&disp_drv);			/*Basic initialization*/
	disp_drv.flush_cb = SSD1963::Flush;		/*Set your driver function*/
	disp_drv.draw_buf = &draw_buf;			/*Assign the buffer to the display*/
	disp_drv.hor_res = DISP_HOR_RES;		/*Set the horizontal resolution of the display*/
	disp_drv.ver_res = DISP_VER_RES;		/*Set the vertical resolution of the display*/
	lv_disp_drv_register(&disp_drv);		/*Finally register the driver*/
}

void Display::Tick() noexcept
{
	lv_tick_inc(1);
}

void Display::Spin() noexcept
{
	lv_timer_handler();
}

void Display::HelloWorld() noexcept
{
	/*Change the active screen's background color*/
	lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x003a57), LV_PART_MAIN);

	/*Create a white label, set its text and align it to the center*/
	lv_obj_t * label = lv_label_create(lv_scr_act());
	lv_label_set_text(label, "Hello newer world");
	lv_obj_set_style_text_color(lv_scr_act(), lv_color_hex(0xffffff), LV_PART_MAIN);
	lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
}

// End
