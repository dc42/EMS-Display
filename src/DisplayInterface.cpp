/*
 * DisplayInterface.cpp
 *
 *  Created on: 26 Nov 2022
 *      Author: David
 */

#include "DisplayInterface.h"
#include "Pins.h"

// Initialise the ports that control the display
void DisplayPortsInit() noexcept
{
	pinMode(DisplayNotResetPin, OUTPUT_HIGH);
	pinMode(DisplayCsPin, OUTPUT_HIGH);
	pinMode(DisplayReadPin, OUTPUT_HIGH);
	pinMode(DisplayWritePin, OUTPUT_HIGH);
	pinMode(DisplayBacklightPin, OUTPUT_HIGH);
	for (unsigned int i = 0; i < 8; ++i)
	{
		pinMode(DisplayLowestDataPin + i, OUTPUT_LOW);
	}
}

// Set the command/data pin to 'val'
void WriteDisplayCommandNotDataPin(bool val) noexcept
{
	digitalWrite(DisplayCommandNotDataPin, val);
}

// Set the ~reset pin to 'val'
void WriteDisplayNotResetPin(bool val) noexcept
{
	digitalWrite(DisplayNotResetPin, val);
}

// Set the Parallel port's Chip select to 'val'
void WriteDisplayCsPin(bool val) noexcept
{
	digitalWrite(DisplayCsPin, val);
}

// Write a word to the parallel port
void WriteDisplayWord(uint16_t data) noexcept
{
	digitalWrite(DisplayLatchHighDataPin, true);
	if constexpr(DisplayLowestDataPin <= 8)
	{
		gpio_put_masked(0x000000FF << DisplayLowestDataPin, data >> (8 - DisplayLowestDataPin));
	}
	else
	{
		gpio_put_masked(0x000000FF << DisplayLowestDataPin, data << (DisplayLowestDataPin - 8));
	}
	digitalWrite(DisplayLatchHighDataPin, false);
	gpio_put_masked(0x000000FF << DisplayLowestDataPin, data << DisplayLowestDataPin);
	digitalWrite(DisplayWritePin, false);
	delayMicroseconds(10);
	digitalWrite(DisplayWritePin, true);
}

// Write 'n' bytes to Parallel ports from 'adr'
void WriteDisplayArray(const uint16_t *adr, unsigned int n) noexcept
{
	while (n != 0)
	{
		WriteDisplayWord(*adr++);
		--n;
	}
}

void SetBacklight(bool on) noexcept
{
	digitalWrite(DisplayBacklightPin, on);
}

// End
