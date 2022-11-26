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
constexpr Pin DisplayCommandNotDataPin = GpioPin(11);
constexpr Pin DisplayCsPin = GpioPin(12);
constexpr Pin DisplayLatchHighDataPin = GpioPin(8);
constexpr Pin DisplayReadPin = GpioPin(9);
constexpr Pin DisplayWritePin = GpioPin(10);
constexpr unsigned int DisplayLowestDataPin = 0;

#endif /* SRC_PINS_H_ */
