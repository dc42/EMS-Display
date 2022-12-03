/*
 * Pins.h
 *
 *  Created on: 26 Nov 2022
 *      Author: David
 *
 */

#ifndef SRC_PINS_H_
#define SRC_PINS_H_

typedef uint8_t Pin;

#include <CoreIO.h>

constexpr Pin DisplayNotResetPin = GpioPin(13);
constexpr Pin DisplayDataNotCommandPin = GpioPin(11);
constexpr Pin DisplayCsPin = GpioPin(12);
constexpr Pin DisplayLatchLowDataPin = GpioPin(8);
constexpr Pin DisplayReadPin = GpioPin(9);
constexpr Pin DisplayWritePin = GpioPin(10);
constexpr Pin DisplayBacklightPin = GpioPin(14);
constexpr Pin DisplayLowestDataPin = 0;

constexpr Pin MotionSensorPin = GpioPin(15);

constexpr Pin WS2812Pin = GpioPin(16);
constexpr unsigned int NumLeds = 2;

constexpr Pin BuzzerLowPin = GpioPin(26);
constexpr Pin BuzzerHighPin = GpioPin(27);
constexpr unsigned int PwmNumber = 5;						// the PWM number to which both buzzer pins are connected

constexpr DmaChannel DmacChanWS2812 = 0;
constexpr DmaChannel DmacChanAdcRx = 1;

// DMA priorities, higher is better. RP2040 has only 0 and 1.
constexpr DmaPriority DmacPrioAdcRx = 1;

#endif /* SRC_PINS_H_ */
