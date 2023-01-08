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
#include <Drivers/TouchPanel.h>
#include "Pins.h"

#include <lvgl.h>
#include <src/hal/lv_hal_disp.h>

constexpr unsigned int DISP_HOR_RES = SSD1963_HOR_RES;
constexpr unsigned int DISP_VER_RES = SSD1963_VER_RES;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf1[DISP_HOR_RES * DISP_VER_RES / 10];	// Declare a buffer for 1/10 screen size
static lv_disp_drv_t disp_drv;								// Descriptor of a display driver

static lv_indev_drv_t indev_drv;							// Descriptor of an input device
static lv_indev_t * my_indev = nullptr;

static lv_obj_t * label;

static void ReadTouchPanel(lv_indev_drv_t *drv, lv_indev_data_t*data) noexcept
{
	uint16_t x, y;
	static bool repeat = false;
	if (TouchPanel::Read(x, y, repeat))
	{
		data->point.x = x;
		data->point.y = y;
		data->state = LV_INDEV_STATE_PRESSED;
	}
	else
	{
		data->state = LV_INDEV_STATE_RELEASED;
	}
}

static void TouchPanelFeedback(lv_indev_drv_t *drv, uint8_t inEvent) noexcept
{
	if (inEvent == LV_EVENT_PRESSED)
	{
		Buzzer::Beep(3000, 50);
	}
}

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

	TouchPanel::Init(SSD1963_HOR_RES, SSD1963_VER_RES, DisplayOrientation::SwapXY | DisplayOrientation::ReverseY);
	lv_indev_drv_init(&indev_drv);      	/*Basic initialization*/
	indev_drv.type = LV_INDEV_TYPE_POINTER;	/*Device type*/
	indev_drv.read_cb = ReadTouchPanel;		/*See below.*/
	indev_drv.feedback_cb = TouchPanelFeedback;
	my_indev = lv_indev_drv_register(&indev_drv);	/*Register the driver in LVGL and save the created input device object*/
}

void Display::Tick() noexcept
{
	lv_tick_inc(1);
}

void Display::Spin() noexcept
{
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
	lv_timer_handler();
}

static void event_cb(lv_event_t * e)
{
    LV_LOG_USER("Clicked");

    static uint32_t cnt = 1;
    lv_obj_t * btn = lv_event_get_target(e);
    lv_obj_t * label = lv_obj_get_child(btn, 0);
    lv_label_set_text_fmt(label, "%" LV_PRIu32, cnt);
    cnt++;
}

void Display::Start() noexcept
{
	constexpr lv_coord_t tileWidth = DISP_HOR_RES/3 - 21;
	constexpr lv_coord_t tileHeight = DISP_VER_RES/2 - 28;
    static constexpr lv_coord_t col_dsc[] = {tileWidth, tileWidth, tileWidth, LV_GRID_TEMPLATE_LAST};
    static constexpr lv_coord_t row_dsc[] = {tileHeight, tileHeight, LV_GRID_TEMPLATE_LAST};

    // Create a grid that fills the screen
    lv_obj_t * cont = lv_obj_create(lv_scr_act());
    lv_obj_set_style_grid_column_dsc_array(cont, col_dsc, 0);
    lv_obj_set_style_grid_row_dsc_array(cont, row_dsc, 0);
    lv_obj_set_style_pad_row(cont, 10, 0);
    lv_obj_set_style_pad_column(cont, 10, 0);
    lv_obj_set_size(cont, DISP_HOR_RES, DISP_VER_RES);
    lv_obj_center(cont);
    lv_obj_set_layout(cont, LV_LAYOUT_GRID);

    for (uint32_t i = 0; i < 6; i++)
    {
        const uint8_t col = i % 3;
        const uint8_t row = i / 3;

        lv_obj_t * const btn = lv_btn_create(cont);
        lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICK_FOCUSABLE);
        lv_obj_add_event_cb(btn, event_cb, LV_EVENT_PRESSED, nullptr);

        // Stretch the cell horizontally and vertically
        // Set span to 1 to make the cell 1 column/row sized
        lv_obj_set_grid_cell(btn, LV_GRID_ALIGN_STRETCH, col, 1, LV_GRID_ALIGN_STRETCH, row, 1);

        label = lv_label_create(btn);
        lv_label_set_text_fmt(label, "c%u, r%u", col, row);
        lv_obj_center(label);
    }
}

// End
