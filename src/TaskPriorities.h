/*
 * TaskPriorities.h
 *
 *  Created on: 26 Nov 2022
 *      Author: David
 */

#ifndef SRC_TASKPRIORITIES_H_
#define SRC_TASKPRIORITIES_H_

// Task priorities
namespace TaskPriority
{
	static constexpr unsigned int SpinPriority = 1;							// priority for tasks that rarely block
	static constexpr unsigned int UsbPriority = 2;
	static constexpr unsigned int AinPriority = 2;
}

#endif /* SRC_TASKPRIORITIES_H_ */
