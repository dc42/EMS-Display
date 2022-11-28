/*
 * Display.cpp
 *
 *  Created on: 26 Nov 2022
 *      Author: David
 */

#include <Core.h>
#include "Display.h"
#include "DisplayInterface.h"
#include <lvgl.h>
#include <src/hal/lv_hal_disp.h>

#define USE_SSD1963		1

#include <../lv_drivers/display/SSD1963.h>

constexpr unsigned int DISP_HOR_RES = 800;
constexpr unsigned int DISP_VER_RES = 480;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf1[DISP_HOR_RES * DISP_VER_RES / 10];                        /*Declare a buffer for 1/10 screen size*/
static lv_disp_drv_t disp_drv;				/* Descriptor of a display driver*/

// Flush the bugger to the display
void my_disp_flush(lv_disp_drv_t * disp, const lv_area_t * area, lv_color_t * color_p)
{
	ssd1963_flush(disp, area, color_p);
    lv_disp_flush_ready(disp);				/* Indicate you are ready with the flushing*/
}

// Initialise the display
void Display::Init() noexcept
{
	DisplayPortsInit();
	lv_init();
	lv_disp_draw_buf_init(&draw_buf, buf1, nullptr, DISP_HOR_RES * DISP_VER_RES / 10);  /*Initialize the display buffer.*/
	ssd1963_init();
	lv_disp_drv_init(&disp_drv);			/*Basic initialization*/
	disp_drv.flush_cb = my_disp_flush;		/*Set your driver function*/
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
	lv_label_set_text(label, "Hello world");
	lv_obj_set_style_text_color(lv_scr_act(), lv_color_hex(0xffffff), LV_PART_MAIN);
	lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
}

// End
