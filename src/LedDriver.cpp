/*
 * LedDriver.cpp
 *
 *  Created on: 27 Nov 2022
 *      Author: David
 */

#include "LedDriver.h"
#include <Pins.h>
#include <WS2812.h>

static WS2812 *driver = nullptr;

void LedDriver::Init() noexcept
{
	driver = new WS2812(WS2812Pin, false, DmacChanWS2812);
}

void LedDriver::SetColour(uint8_t red, uint8_t green, uint8_t blue) noexcept
{
	driver->SetColour(((uint32_t)green << 24) | ((uint32_t)red << 16) | ((uint32_t)blue << 8), NumLeds);
}

// End
