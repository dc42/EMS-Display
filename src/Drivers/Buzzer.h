/*
 * Buzzer.h
 *
 *  Created on: 3 Dec 2022
 *      Author: David
 */

#ifndef SRC_DRIVERS_BUZZER_H_
#define SRC_DRIVERS_BUZZER_H_

#include <cstdint>

namespace Buzzer {
	void Init() noexcept;
	void SetVolume(uint8_t volume) noexcept;
	void Beep(uint32_t frequency, uint32_t milliseconds) noexcept;
	void Tick() noexcept;
}

#endif /* SRC_DRIVERS_BUZZER_H_ */
