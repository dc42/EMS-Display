/*
 * LedDriver.h
 *
 *  Created on: 27 Nov 2022
 *      Author: David
 */

#ifndef SRC_LEDDRIVER_H_
#define SRC_LEDDRIVER_H_

#include <cstdint>

namespace LedDriver {
void Init() noexcept;
void SetColour(uint8_t red, uint8_t green, uint8_t blue) noexcept;
}

#endif /* SRC_LEDDRIVER_H_ */
