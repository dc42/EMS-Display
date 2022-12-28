/*
 * Display.cpp
 *
 *  Created on: 26 Nov 2022
 *      Author: David
 */

#include <Core.h>
#include "Display.h"
#include <Drivers/SSD1963.h>
#include <Drivers/Buzzer.h>
#include "Pins.h"

#include <lvgl.h>
#include <src/hal/lv_hal_disp.h>

constexpr unsigned int DISP_HOR_RES = SSD1963_HOR_RES;
constexpr unsigned int DISP_VER_RES = SSD1963_VER_RES;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf1[DISP_HOR_RES * DISP_VER_RES / 10];	/*Declare a buffer for 1/10 screen size*/
static lv_disp_drv_t disp_drv;								/* Descriptor of a display driver*/

static lv_obj_t * label;

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
#if 0
	// Test code to do continuous write to the screen
	lv_obj_clean(lv_scr_act());
	lv_timer_handler();
	Start();
#else
	static bool detectedMotion = false;
	if (digitalRead(MotionSensorPin))
	{
		if (!detectedMotion)
		{
			Buzzer::Beep(2000, 200);
			lv_label_set_text(label, "Detected motion");
			detectedMotion = true;
		}
	}
	else if (detectedMotion)
	{
		lv_label_set_text(label, "Idle");
		detectedMotion = false;
	}
#endif
	lv_timer_handler();
}

void Display::Start() noexcept
{
	constexpr lv_coord_t tileWidth = DISP_HOR_RES/3;
	constexpr lv_coord_t tileHeight = DISP_VER_RES/2;
    static constexpr lv_coord_t col_dsc[] = {tileWidth, tileWidth, tileWidth, LV_GRID_TEMPLATE_LAST};
    static constexpr lv_coord_t row_dsc[] = {tileHeight, tileHeight, LV_GRID_TEMPLATE_LAST};

    // Create a grid hat fills the screen
    lv_obj_set_style_grid_column_dsc_array(lv_scr_act(), col_dsc, 0);
    lv_obj_set_style_grid_row_dsc_array(lv_scr_act(), row_dsc, 0);
    lv_obj_set_layout(lv_scr_act(), LV_LAYOUT_GRID);

    for (uint32_t i = 0; i < 6; i++)
    {
        const uint8_t col = i % 3;
        const uint8_t row = i / 3;

        lv_obj_t * const btn = lv_btn_create(lv_scr_act());
        // Stretch the cell horizontally and vertically
        // Set span to 1 to make the cell 1 column/row sized
        lv_obj_set_grid_cell(btn, LV_GRID_ALIGN_STRETCH, col, 1, LV_GRID_ALIGN_STRETCH, row, 1);

        label = lv_label_create(btn);
        lv_label_set_text_fmt(label, "c%u, r%u", col, row);
        lv_obj_center(label);
    }
}

// End
