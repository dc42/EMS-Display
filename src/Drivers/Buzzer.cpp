/*
 * Buzzer.cpp
 *
 *  Created on: 3 Dec 2022
 *      Author: David
 */

#include "Buzzer.h"
#include <CoreIO.h>
#include <Pins.h>
#include <hardware/pwm.h>

// When the buzzer is off we set both output pins low to avoid giving it DC
// When the buzzer is on we drive it in differential mode.

static uint32_t ticksUntilOff = 0;

static void TurnOff() noexcept
{
	pwm_set_enabled(PwmNumber, false);
	pinMode(BuzzerLowPin, OUTPUT_LOW);
	pinMode(BuzzerHighPin, OUTPUT_LOW);
}

void Buzzer::Init() noexcept
{
	TurnOff();
}

void Buzzer::SetVolume(uint8_t volume) noexcept
{
	//TODO
}

void Buzzer::Beep(uint32_t frequency, uint32_t milliseconds) noexcept
{
	// Find a combination of prescaler and top value that approximates the required frequency. We have 16 bits of top and 12 bits of divisor to play with.
	const uint32_t totalDivisor = SystemCoreClock/(2 * frequency);
	uint8_t prescaler, prescalerFraction;
	uint16_t top;
	if (totalDivisor >= 65536 * 256)
	{
		// Lowest frequency we can generate is about 3.75Hz
		top = 65535;
		prescaler = 255;
		prescalerFraction = 15;
	}
	else
	{
		unsigned int shift = 16;
		while ((totalDivisor >> shift) < 128)
		{
			--shift;
		}
		top = (1ul << shift) - 1;
		prescaler = totalDivisor >> shift;
		prescalerFraction = (totalDivisor >> (shift - 4)) & 0x0F;
	}
	pwm_config config = pwm_get_default_config();
	pwm_config_set_clkdiv_int_frac(&config, prescaler, prescalerFraction);
	pwm_config_set_phase_correct(&config, true);
	pwm_config_set_wrap(&config, top);
	pwm_init(PwmNumber, &config, false);
	pwm_set_both_levels(PwmNumber, top/2, top/2);
	pwm_set_output_polarity(PwmNumber, false, true);
	pwm_set_enabled(PwmNumber, true);
	SetPinFunction(BuzzerLowPin, GpioPinFunction::Pwm);
	SetPinFunction(BuzzerHighPin, GpioPinFunction::Pwm);
	ticksUntilOff = milliseconds;
}

void Buzzer::Tick() noexcept
{
	if (ticksUntilOff != 0)
	{
		if (--ticksUntilOff == 0)
		{
			TurnOff();
		}
	}
}

// End
