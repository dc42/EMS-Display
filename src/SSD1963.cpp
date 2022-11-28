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
//  Propagation delay D to Q when LE high 190ns @ 2V, 38ns @ 4.5V

inline void PulseWritePin() noexcept
{
	fastDigitalWriteLow(DisplayWritePin);
	asm volatile("nop");
	asm volatile("nop");
	asm volatile("nop");
	asm volatile("nop");
	fastDigitalWriteHigh(DisplayWritePin);
}

inline void LCD_Write_Bus16(uint16_t data) noexcept
{
	digitalWrite(DisplayLatchLowDataPin, true);
	gpio_put_masked(0x000000FF << DisplayLowestDataPin, data << DisplayLowestDataPin);		// Put the low word on the bus
	asm volatile("nop");
	asm volatile("nop");
	asm volatile("nop");
	asm volatile("nop");
	digitalWrite(DisplayLatchLowDataPin, false);
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
	digitalWrite(DisplayLatchLowDataPin, true);
	gpio_put_masked(0x000000FF << DisplayLowestDataPin, data << DisplayLowestDataPin);		// Put the low word on the bus
	asm volatile("nop");
	asm volatile("nop");
	asm volatile("nop");
	asm volatile("nop");
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

void SSD1963::Init() noexcept
{
	// Set up the output pins
	pinMode(DisplayNotResetPin, OUTPUT_HIGH);
	pinMode(DisplayCsPin, OUTPUT_HIGH);
	pinMode(DisplayDataNotCommandPin, OUTPUT_HIGH);
	pinMode(DisplayReadPin, OUTPUT_HIGH);
	pinMode(DisplayWritePin, OUTPUT_HIGH);
	pinMode(DisplayLatchLowDataPin, OUTPUT_LOW);
	pinMode(DisplayBacklightPin, OUTPUT_HIGH);
	for (unsigned int i = 0; i < 8; ++i)
	{
		pinMode(DisplayLowestDataPin + i, OUTPUT_LOW);
	}

	// Hardware reset the display
	fastDigitalWriteHigh(DisplayNotResetPin);
	delay(5);
	fastDigitalWriteLow(DisplayNotResetPin);
	delay(15);
	fastDigitalWriteHigh(DisplayNotResetPin);
	delay(15);

	// Initialise the display
	fastDigitalWriteLow(DisplayCsPin);

	LCD_Write_COM(0xE2);		//PLL multiplier, set PLL clock to 120M
	LCD_Write_DATA8(0x1E);	    //N=0x36 for 6.5M, 0x23 for 10M crystal (ER 5": 0x23)
	LCD_Write_Bus8(0x02);
	LCD_Write_Bus8(0x54);
	LCD_Write_COM(0xE0);		// PLL enable
	LCD_Write_DATA8(0x01);
	delay(10);

	LCD_Write_COM(0xE0);
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

//	setOrientation(orient, isER);

	LCD_Write_COM(0xF0);		//pixel data interface
	LCD_Write_DATA8(0x03);		//0x03: 16-bit (565 format), 0x02: 16-bit packed

	delay(1);

//	setXY(0, 0, 799, 479);
	LCD_Write_COM(0x29);		//display on

	LCD_Write_COM(0xBE);		//set PWM for B/L
	LCD_Write_DATA8(0x06);
	LCD_Write_Bus8(0xf0);
	LCD_Write_Bus8(0x01);
	LCD_Write_Bus8(0xf0);
	LCD_Write_Bus8(0x00);
	LCD_Write_Bus8(0x00);

	LCD_Write_COM(0xd0);
	LCD_Write_DATA8(0x0d);

	LCD_Write_COM(0x2C);

	fastDigitalWriteHigh(DisplayCsPin);
}

void SSD1963::Flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p) noexcept
{
    // Return if the area is out the screen
    if (area->x2 < 0) return;
    if (area->y2 < 0) return;
    if (area->x1 > SSD1963_HOR_RES - 1) return;
    if (area->y1 > SSD1963_VER_RES - 1) return;

    // Truncate the area to the screen
    int32_t act_x1 = area->x1 < 0 ? 0 : area->x1;
    int32_t act_y1 = area->y1 < 0 ? 0 : area->y1;
    int32_t act_x2 = area->x2 > SSD1963_HOR_RES - 1 ? SSD1963_HOR_RES - 1 : area->x2;
    int32_t act_y2 = area->y2 > SSD1963_VER_RES - 1 ? SSD1963_VER_RES - 1 : area->y2;

    // Set the rectangular area
	fastDigitalWriteLow(DisplayCsPin);
	LCD_Write_COM_DATA16(0x2A, act_x1 >> 8);
	LCD_Write_Bus16(0x00FF & act_x1);
	LCD_Write_Bus16(act_x2 >> 8);
	LCD_Write_Bus16(0x00FF & act_x2);

	LCD_Write_COM_DATA16(0x002B, act_y1 >> 8);
	LCD_Write_Bus16(0x00FF & act_y1);
	LCD_Write_Bus16(act_y2 >> 8);
	LCD_Write_Bus16(0x00FF & act_y2);

	LCD_Write_COM(0x2c);
    const uint16_t full_w = area->x2 - area->x1 + 1;

	fastDigitalWriteHigh(DisplayDataNotCommandPin);
    const uint16_t act_w = act_x2 - act_x1 + 1;

    for (int16_t i = act_y1; i <= act_y2; i++)
    {
    	const uint16_t *p = (uint16_t*)color_p;
    	for (uint16_t j = 0; j < act_w; ++j)
    	{
    		LCD_Write_Bus16(*p++);
    	}
        color_p += full_w;
    }
	fastDigitalWriteHigh(DisplayCsPin);

    lv_disp_flush_ready(disp_drv);
}

// End
