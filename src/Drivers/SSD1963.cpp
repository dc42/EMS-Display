/*
 * SSD1963.cpp
 *
 *  Created on: 28 Nov 2022
 *      Author: David
 */

#include "SSD1963.h"
#include <Pins.h>
#include <CoreIO.h>
#include <hardware/gpio.h>

constexpr bool Is24bit = true;

// SSD1963 timing requirements:
//  CS falling to WR falling >= 2ns
//  CS minimum low time  >= 1.5 PLL clock periods (max PLL clock 110MHz)
//  Data/~CMD to WR falling >= 1ns
//  WR low time >= 12ns
//  Data setup time to trailing edge of ~WR >= 4ns
//  Data hold time from trailing edge of ~WR >= 1ns
// 74HC573 timing requirements:
//  Latch pulse width 100ns @ 2V, 20ns @ 4.5V
//  Data setup time to trailing edge of latch pulse 65ns @ 2V, 13ns @ 3.3V
//  Data hold time from trailing edge of latch pulse 5ns
//  Propagation delay D to Q when LE high, or from LE to Q 190ns @ 2V, 38ns @ 4.5V
// 74AHC573 timing requirements:
//  Latch pulse width 15ns
//  Data setup time to trailing edge of latch pulse 3.5ns
//  Data hold time from trailing edge of latch pulse 1.5ns
//  Propagation delay D to Q when LE high, or from LE to Q 15ns @ 3 to 3.5V

void PulseWritePin() noexcept
{
	fastDigitalWriteLow(DisplayWritePin);
	// Put enough NOPs here to meet the SSD1963 write low time (12ns)
	asm volatile("nop");
	asm volatile("nop");
	asm volatile("nop");
	fastDigitalWriteHigh(DisplayWritePin);
}

inline void LCD_Write_Bus16(uint16_t data) noexcept
{
	fastDigitalWriteHigh(DisplayLatchLowDataPin);
	gpio_put_masked(0x000000FF << DisplayLowestDataPin, data << DisplayLowestDataPin);		// Put the low word on the bus
	// Put enough NOPs here to meet the 74HC573 setup time (13ns)
	asm volatile("nop");
	asm volatile("nop");
	asm volatile("nop");
	fastDigitalWriteLow(DisplayLatchLowDataPin);
	// The hold time is only 5ns so we don't need a delay here
	if constexpr(DisplayLowestDataPin <= 8)
	{
		gpio_put_masked(0x000000FF << DisplayLowestDataPin, data >> (8 - DisplayLowestDataPin));
	}
	else
	{
		gpio_put_masked(0x000000FF << DisplayLowestDataPin, data << (DisplayLowestDataPin - 8));
	}
	PulseWritePin();
}

inline void LCD_Write_Bus8(uint8_t data) noexcept
{
	fastDigitalWriteHigh(DisplayLatchLowDataPin);
	gpio_put_masked(0x000000FF << DisplayLowestDataPin, data << DisplayLowestDataPin);		// Put the low word on the bus
	// Put enough NOPs here to meet the 74HC573 propagation time (15ns)
	asm volatile("nop");
	asm volatile("nop");
	asm volatile("nop");
	fastDigitalWriteLow(DisplayLatchLowDataPin);				// this is not needed but it makes the signal cleaner on the 'scope
	PulseWritePin();
}

inline void LCD_Write_COM(uint8_t VL) noexcept
{
	fastDigitalWriteLow(DisplayDataNotCommandPin);
	LCD_Write_Bus8(VL);
}

inline void LCD_Write_DATA16(uint16_t VHL) noexcept
{
	fastDigitalWriteHigh(DisplayDataNotCommandPin);
	LCD_Write_Bus16(VHL);
}

inline void LCD_Write_DATA8(uint8_t data) noexcept
{
	fastDigitalWriteHigh(DisplayDataNotCommandPin);
	LCD_Write_Bus8(data);
}

inline void LCD_Write_COM_DATA16(uint8_t com1, uint16_t dat1)
{
	LCD_Write_COM(com1);
	LCD_Write_DATA16(dat1);
}

inline void LCD_Write_COM_DATA8(uint8_t com1, uint8_t dat1)
{
	LCD_Write_COM(com1);
	LCD_Write_DATA8(dat1);
}

inline void SetXY(uint16_t xLow, uint16_t xHigh, uint16_t yLow, uint16_t yHigh) noexcept
{
	LCD_Write_COM_DATA8(0x2A, xLow >> 8);
	LCD_Write_Bus8(0x00FF & xLow);
	LCD_Write_Bus8(xHigh >> 8);
	LCD_Write_Bus8(0x00FF & xHigh);

	LCD_Write_COM_DATA8(0x002B, yLow >> 8);
	LCD_Write_Bus8(0x00FF & yLow);
	LCD_Write_Bus8(yHigh >> 8);
	LCD_Write_Bus8(0x00FF & yHigh);
}

void SSD1963::Init() noexcept
{
	// Set up the output pins
	pinMode(DisplayNotResetPin, OUTPUT_HIGH);
	pinMode(DisplayCsPin, OUTPUT_HIGH);
	pinMode(DisplayDataNotCommandPin, OUTPUT_HIGH);
	pinMode(DisplayReadPin, OUTPUT_HIGH);
	pinMode(DisplayWritePin, OUTPUT_HIGH);
	SetDriveStrength(DisplayWritePin, 2);
	pinMode(DisplayLatchLowDataPin, OUTPUT_LOW);
	SetDriveStrength(DisplayLatchLowDataPin, 2);
	pinMode(DisplayBacklightPin, OUTPUT_LOW);
	for (unsigned int i = 0; i < 8; ++i)
	{
		pinMode(DisplayLowestDataPin + i, OUTPUT_LOW);
	}

	// Hardware reset the display
	delay(15);
	fastDigitalWriteLow(DisplayNotResetPin);
	delay(15);
	fastDigitalWriteHigh(DisplayNotResetPin);
	delay(15);

	// Initialise the display
	fastDigitalWriteLow(DisplayCsPin);

	LCD_Write_COM(0xE2);		// PLL multiplier, set PLL clock to 100M (M=35 N=2 for 10MHz crystal)
	LCD_Write_DATA8(0x1D);	    // N=0x36 for 6.5M, 0x23 for 10M crystal (ER 5": 0x23)
	LCD_Write_DATA8(0x22);
	LCD_Write_DATA8(0x04);

	LCD_Write_COM(0xE0);		// PLL enable
	LCD_Write_DATA8(0x01);
	delay(10);					// need 100us here according to datasheet

	LCD_Write_COM(0xE0);		// use PLL as system clock
	LCD_Write_DATA8(0x03);
	delay(10);

	LCD_Write_COM(0x01);		// software reset
	delay(100);

	LCD_Write_COM(0xE6);		//PLL setting for PCLK, depends on resolution
	LCD_Write_DATA8(0x03);
	LCD_Write_Bus8(0xFF);		// ER 5": 0x33
	LCD_Write_Bus8(0xFF);		// ER 5": 0x33

	LCD_Write_COM(0xB0);		//LCD SPECIFICATION
	LCD_Write_DATA8((Is24bit) ? 0x20		// other 5" displays are 24-bit, data latched on falling edge I assume (setting 0x24 for rising edge works too)
						: 0x00);			// other 7" displays are 18-bit, data latched on falling edge (works better than setting rising edge)
	LCD_Write_Bus8(0x00);
	LCD_Write_Bus8(((SSD1963_HOR_RES - 1) >> 8) & 0X00FF); //Set HDP
	LCD_Write_Bus8((SSD1963_HOR_RES - 1) & 0X00FF);
	LCD_Write_Bus8(((SSD1963_VER_RES - 1) >> 8) & 0X00FF); //Set VDP
	LCD_Write_Bus8((SSD1963_VER_RES - 1) & 0X00FF);
	LCD_Write_Bus8(0x00);

	LCD_Write_COM(0xB4);		//HSYNC
	LCD_Write_DATA8(0x03);		//Set HT	928 (ER 5": 0x04 = 1055)
	LCD_Write_Bus8(0xA0);		// ER 5": 0x1f
	LCD_Write_Bus8(0x00);		//Set HPS	46
	LCD_Write_Bus8(0x2E);		// ER 5": 0xD2 = 210
	LCD_Write_Bus8(0x30);		//Set HPW	48
	LCD_Write_Bus8(0x00);		//Set LPS	15 (ER 5": 0)
	LCD_Write_Bus8(0x0F);		// ER 5": 0
	LCD_Write_Bus8(0x00);

	LCD_Write_COM(0xB6);		//VSYNC
	LCD_Write_DATA8(0x02);		//Set VT	525
	LCD_Write_Bus8(0x0D);		// ER 5": 0x0c = 524
	LCD_Write_Bus8(0x00);		//Set VPS	16
	LCD_Write_Bus8(0x10);		// ER 5": 0x22 = 34
	LCD_Write_Bus8(0x10);		//Set VPW	16 (ER 5": 0x00 = 0)
	LCD_Write_Bus8(0x00);		//Set FPS	8
	LCD_Write_Bus8(0x08);		// ER 5": 0x00 = 0

	LCD_Write_COM(0xBA);
	LCD_Write_DATA8(0x0F);		//GPIO[3:0] out 1

	LCD_Write_COM(0xB8);
	LCD_Write_DATA8(0x07);	    //GPIO3=input, GPIO[2:0]=output
	LCD_Write_Bus8(0x01);		//GPIO0 normal

	LCD_Write_COM(0xF0);		//pixel data interface
	LCD_Write_DATA8(0x03);		//0x03: 16-bit (565 format), 0x02: 16-bit packed

	delay(1);

	// Clear the display
	SetXY(0, SSD1963_HOR_RES - 1, 0, SSD1963_VER_RES - 1);
	LCD_Write_COM(0x2C);
	LCD_Write_DATA16(0);		// write first pixel
	for (unsigned int i = 0; i < SSD1963_HOR_RES * SSD1963_VER_RES - 1; ++i)
	{
		asm volatile("nop");
		asm volatile("nop");
		PulseWritePin();		// write remaining pixels
	}

	LCD_Write_COM(0x29);		// display on

	LCD_Write_COM(0xBE);		// set PWM for B/L
	LCD_Write_DATA8(0x06);		// PWM frequency = PLL clock / (256 * (6 + 1) /256
	LCD_Write_Bus8(0xf0);		// PWm duty cycle
	LCD_Write_Bus8(0x01);		// PWM enabled and controlled by host
	LCD_Write_Bus8(0xf0);		// Manual brightness value
	LCD_Write_Bus8(0x00);		// Minimum brightness
	LCD_Write_Bus8(0x00);		// Brightness prescaler for transition effects

	LCD_Write_COM(0xd0);		// Dynamic brightness configuration
	LCD_Write_DATA8(0x0d);		// DNC enable, aggressive mode

	fastDigitalWriteHigh(DisplayCsPin);

	delay(1000);
	fastDigitalWriteHigh(DisplayBacklightPin);
}

void SSD1963::Flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p) noexcept
{
	// Truncate the area to the screen
	int32_t act_x1 = max<lv_coord_t>(area->x1, 0);
	int32_t act_y1 = max<lv_coord_t>(area->y1, 0);
	int32_t act_x2 = min<lv_coord_t>(area->x2, SSD1963_HOR_RES - 1);
	int32_t act_y2 = min<lv_coord_t>(area->y2, SSD1963_VER_RES - 1);
	if (act_x1 <= act_x2 && act_y1 <= act_y2)
	{
		// Set the rectangular area
		fastDigitalWriteLow(DisplayCsPin);

#if 1
		SetXY(act_x1, act_x2, act_y1, act_y2);
		LCD_Write_COM(0x2c);
		fastDigitalWriteHigh(DisplayDataNotCommandPin);
#endif

		const uint16_t full_w = area->x2 - area->x1 + 1;
		const uint16_t act_w = act_x2 - act_x1 + 1;

		for (int16_t i = act_y1; i <= act_y2; i++)
		{
#if 0
			SetXY(act_x1, act_x2, i, i);
			LCD_Write_COM(0x2c);
			fastDigitalWriteHigh(DisplayDataNotCommandPin);
#endif
			const uint16_t *p = (uint16_t*)color_p;
			uint16_t lastPixel = *p++;
			LCD_Write_Bus16(lastPixel);
			for (uint16_t j = 1; j < act_w; ++j)
			{
				const uint16_t newPixel = *p++;
				if (newPixel == lastPixel)
				{
					PulseWritePin();
				}
				else
				{
					lastPixel = newPixel;
					LCD_Write_Bus16(newPixel);
				}
			}
			color_p += full_w;
		}
		fastDigitalWriteHigh(DisplayCsPin);
	}

	lv_disp_flush_ready(disp_drv);
}

// End
