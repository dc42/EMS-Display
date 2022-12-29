/*
 * TouchPanel.cpp
 *
 *  Created on: 3 Dec 2022
 *      Author: David
 *
 *  Uses the approach described in TI app note http://www.ti.com/lit/pdf/sbaa036.
 */

#include "TouchPanel.h"
#include <CoreIO.h>
#include <Pins.h>

static DisplayOrientation orientAdjust;
static uint16_t disp_x_size, disp_y_size;
static uint16_t scaleX, scaleY;
static int16_t offsetX, offsetY;
static bool pressed = false;

// Send the first command in a chain. The chip latches the data bit on the rising edge of the clock. We have already set CS low.
static void WriteCommand(uint8_t command) noexcept
{
	for (uint8_t count = 0; count < 8; count++)
	{
		digitalWrite(TouchDinPin, command & 0x80);
		command <<= 1;
		delayNanoseconds(100);					// need 100ns setup time from writing data to clock rising edge
		fastDigitalWriteHigh(TouchClkPin);
		delayNanoseconds(200);					// minimum 200ns clock high width
		fastDigitalWriteLow(TouchClkPin);
		delayNanoseconds(100);					// need 200ns clock low time, but we will delay 100ns at the start of the next iteration
	}
}

// Read the data, and write another command at the same time. We have already set CS low.
// The chip produces its data bit after the falling edge of the clock. After sending 8 clocks, we can send a command again.
static uint16_t ReadData(uint8_t command) noexcept
{
	uint16_t cmd = (uint16_t)command;
	uint16_t data = 0;

	for (uint8_t count=0; count<16; count++)
	{
		digitalWrite(TouchDinPin, command & 0x8000);
		cmd <<= 1;
		delayNanoseconds(100);					// need 100ns setup time from writing data to clock rising edge
		digitalWrite(TouchClkPin, true);
		delayNanoseconds(200);					// minimum 200ns clock high width
		digitalWrite(TouchClkPin, false);
		if (count < 12)
		{
			delayNanoseconds(200);				// need 200ns setup time from clock falling edge to reading data
			data <<= 1;
			if (digitalRead(TouchDoutPin))
			{
				data++;
			}
		}
	}

	return(data);
}

static uint16_t diff(uint16_t a, uint16_t b) noexcept { return (a < b) ? b - a : a - b; }

// Get data from the touch chip. CS has already been set low.
// We need to allow the touch chip ADC input to settle. See TI app note http://www.ti.com/lit/pdf/sbaa036.
static bool getTouchData(bool wantY, uint16_t &rslt) noexcept
{
	uint8_t command = (wantY) ? 0xD3 : 0x93;		// start, channel 5 (y) or 1 (x), 12-bit, differential mode, don't power down between conversions
	WriteCommand(command);							// send the command
	ReadData(command);								// discard the first result and send the same command again

	const size_t numReadings = 8;
	const uint16_t maxDiff = 40;					// needs to be big enough to handle jitter.
													// 8 was OK for the 4.3 and 5 inch displays but not the 7 inch.
													// 25 is OK for most 7" displays.
	const unsigned int maxAttempts = 16;

	uint16_t ring[numReadings];
	uint32_t sum = 0;

	// Take enough readings to fill the ring buffer
	for (size_t i = 0; i < numReadings; ++i)
	{
		const uint16_t val = ReadData(command);
		ring[i] = val;
		sum += val;
	}

	// Test whether every reading is within 'maxDiff' of the average reading.
	// If it is, return the average reading.
	// If not, take another reading and try again, up to 'maxAttempts' times.
	uint16_t avg;
	size_t last = 0;
	bool ok;
	for (unsigned int i = 0; i < maxAttempts; ++i)
	{
		avg = (uint16_t)(sum/numReadings);
		ok = true;
		for (size_t i = 0; ok && i < numReadings; ++i)
		{
			if (diff(avg, ring[i]) > maxDiff)
			{
				ok = false;
				break;
			}
		}
		if (ok)
		{
			break;
		}

		// Take another reading
		sum -= ring[last];
		const uint16_t val = ReadData(command);
		ring[last] = val;
		sum += val;
		last = (last + 1) % numReadings;
	}

	ReadData(command & 0xF8);			// tell it to power down between conversions
	ReadData(0);						// read the final data
	rslt = avg;
	return ok;
}

void TouchPanel::AdjustOrientation(DisplayOrientation a) noexcept
{
	orientAdjust = (DisplayOrientation) (orientAdjust ^ a);
}

DisplayOrientation TouchPanel::GetOrientation() noexcept
{
	return orientAdjust;
}

void TouchPanel::Init(uint16_t xp, uint16_t yp, DisplayOrientation orientationAdjust) noexcept
{
	orientAdjust			= orientationAdjust;
	disp_x_size				= xp;
	disp_y_size				= yp;
	offsetX					= 0;
	scaleX					= (uint16_t)(((uint32_t)(disp_x_size - 1) << 16)/4095);
	offsetY					= 0;
	scaleY					= (uint16_t)(((uint32_t)(disp_y_size - 1) << 16)/4095);

	pressed = false;

	pinMode(TouchClkPin, OUTPUT_LOW);
	pinMode(TouchCsPin, OUTPUT_HIGH);
	pinMode(TouchDinPin, OUTPUT_HIGH);
	pinMode(TouchDoutPin, INPUT);
	pinMode(TouchIrqPin, INPUT_PULLUP);

	delay(10);
	uint16_t dummy16;
	getTouchData(false, dummy16);
}

// If the panel is touched, return the coordinates in x and y and return true; else return false
bool TouchPanel::Read(uint16_t &px, uint16_t &py, bool &repeat, uint16_t * null rawX, uint16_t * null rawY) noexcept
{
	bool ret = false;

	repeat = false;

	if (!digitalRead(TouchIrqPin))			// if screen is touched
	{
		fastDigitalWriteLow(TouchCsPin);
		delayMicroseconds(100);				// allow the screen to settle
		uint16_t tx;
		if (getTouchData(false, tx))
		{
			uint16_t ty;
			if (getTouchData(true, ty))
			{
				if (!digitalRead(TouchIrqPin))
				{
					int16_t valx = (orientAdjust & DisplayOrientation::SwapXY) ? ty : tx;
					if (orientAdjust & DisplayOrientation::ReverseX)
					{
						valx = 4095 - valx;
					}

					int16_t cx = (int16_t)(((uint32_t)valx * (uint32_t)scaleX) >> 16) - offsetX;
					px = (cx < 0) ? 0 : (cx >= disp_x_size) ? disp_x_size - 1 : (uint16_t)cx;

					int16_t valy = (orientAdjust & DisplayOrientation::SwapXY) ? tx : ty;
					if (orientAdjust & DisplayOrientation::ReverseY)
					{
						valy = 4095 - valy;
					}

					int16_t cy = (int16_t)(((uint32_t)valy * (uint32_t)scaleY) >> 16) - offsetY;
					py = (cy < 0) ? 0 : (cy >= disp_y_size) ? disp_y_size - 1 : (uint16_t)cy;
					if (rawX != nullptr)
					{
						*rawX = valx;
					}
					if (rawY != nullptr)
					{
						*rawY = valy;
					}
					ret = true;
				}
			}
		}
		fastDigitalWriteHigh(TouchCsPin);
	}


	if (ret && pressed)
	{
		repeat = true;
	}

	pressed = ret;
	return ret;
}

void TouchPanel::Calibrate(uint16_t xlow, uint16_t xhigh, uint16_t ylow, uint16_t yhigh, uint16_t margin) noexcept
{
	scaleX = (uint16_t)(((uint32_t)(disp_x_size - 1 - 2 * margin) << 16)/(xhigh - xlow));
	offsetX = (int16_t)(((uint32_t)xlow * (uint32_t)scaleX) >> 16) - (int16_t)margin;
	scaleY = (uint16_t)(((uint32_t)(disp_y_size - 1 - 2 * margin) << 16)/(yhigh - ylow));
	offsetY = (int16_t)(((uint32_t)ylow * (uint32_t)scaleY) >> 16) - (int16_t)margin;
}

// End
