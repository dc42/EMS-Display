/*
 * Main.cpp
 *
 *  Created on: 30 Oct 2022
 *      Author: David
 */

#include <Core.h>
#include <syscalls.h>
#include <RP2040/Devices.h>
#include <lvgl.h>

constexpr unsigned int DISP_HOR_RES = 800;
constexpr unsigned int DISP_VER_RES = 480;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf1[DISP_HOR_RES * DISP_VER_RES / 10];                        /*Declare a buffer for 1/10 screen size*/


void AppMain() noexcept
{
	CoreInit();
	DeviceInit();

	lv_disp_draw_buf_init(&draw_buf, buf1, nullptr, DISP_HOR_RES * DISP_VER_RES / 10);  /*Initialize the display buffer.*/
	//TODO
	for (;;) { }
}

extern "C" void vApplicationTickHook()
{

}

extern "C" void vApplicationStackOverflowHook()
{

}

extern "C" uint32_t StepTimerGetTimerTicks()
{
	return 0;
}

extern "C" void vAssertCalled(uint32_t ulLine, const char *pcFile) noexcept
{
	for (;;) { }
}

// End
