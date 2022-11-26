/*
 * DisplayInterface.h
 *
 *  Created on: 26 Nov 2022
 *      Author: David
 */

#ifndef SRC_DISPLAYINTERFACE_H_
#define SRC_DISPLAYINTERFACE_H_

#ifdef __cplusplus
# include <cstdint>
extern "C" {
#else
# include <stdint.h>
# include <stdbool.h>
# define noexcept
#endif

void DisplayPortsInit() noexcept;										// Initialise the ports that control the display
void WriteDisplayCommandNotDataPin(bool val) noexcept;					// Set the command/data pin to 'val'
void WriteDisplayNotResetPin(bool val) noexcept; 						// Set the ~reset pin to 'val'
void WriteDisplayCsPin(bool val) noexcept;								// Set the Parallel port's Chip select to 'val'
void WriteDisplayWord(uint16_t data) noexcept;							// Write a word to the parallel port
void WriteDisplayArray(const uint16_t *adr, unsigned int n) noexcept;	// Write 'n' bytes to Parallel ports from 'adr'

#ifdef __cplusplus
}
#endif

#endif /* SRC_DISPLAYINTERFACE_H_ */
