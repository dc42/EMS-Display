/*
 * SSD1963.h
 *
 *  Created on: 28 Nov 2022
 *      Author: David
 */

#ifndef SRC_SSD1963_H_
#define SRC_SSD1963_H_

#include <lvgl.h>

constexpr unsigned int SSD1963_HOR_RES = 800;
constexpr unsigned int SSD1963_VER_RES = 480;

namespace SSD1963
{
	void Init() noexcept;
	extern "C" void Flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p) noexcept;
}

#endif /* SRC_SSD1963_H_ */
