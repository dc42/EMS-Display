/*
 * TouchPanel.h
 *
 *  Created on: 3 Dec 2022
 *      Author: David
 */

#ifndef SRC_DRIVERS_TOUCHPANEL_H_
#define SRC_DRIVERS_TOUCHPANEL_H_

#include <cstdint>
#include "DisplayOrientation.h"
#include <ecv_duet3d.h>

namespace TouchPanel
{
	void Init(uint16_t xp, uint16_t yp, DisplayOrientation orientationAdjust = DisplayOrientation::Default) noexcept;
	bool Read(uint16_t &x, uint16_t &y, bool &repeat, uint16_t * null rawX = nullptr, uint16_t * null rawY = nullptr) noexcept;
	void Calibrate(uint16_t xlow, uint16_t xhigh, uint16_t ylow, uint16_t yhigh, uint16_t margin) noexcept;
	void AdjustOrientation(DisplayOrientation a) noexcept;
	DisplayOrientation GetOrientation() noexcept;
};

#endif /* SRC_DRIVERS_TOUCHPANEL_H_ */
