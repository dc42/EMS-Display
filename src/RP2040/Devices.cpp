/*
 * Devices.cpp
 *
 *  Created on: 28 Jul 2020
 *      Author: David
 */

#include "Devices.h"

#if RP2040

#if 0
#include <AnalogIn.h>
#include <AnalogOut.h>
#endif

#include <TaskPriorities.h>
#include <RTOSIface/RTOSIface.h>
#include <TinyUsbInterface.h>
#include <SerialCDC_tusb.h>
#include <Pins.h>

// Analog input support
#if 0
constexpr size_t AnalogInTaskStackWords = 300;
static Task<AnalogInTaskStackWords> analogInTask;
#endif

constexpr size_t UsbDeviceTaskStackWords = 300;
static Task<UsbDeviceTaskStackWords> usbDeviceTask;

SerialCDC serialUSB;

void DeviceInit() noexcept
{
#if 0
	AnalogIn::Init(DmacChanAdcRx, DmacPrioAdcRx);
	AnalogOut::Init();
	analogInTask.Create(AnalogIn::TaskLoop, "AIN", nullptr, TaskPriority::AinPriority);
#endif

	CoreUsbInit(NvicPriorityUSB);
	usbDeviceTask.Create(CoreUsbDeviceTask, "USBD", nullptr, TaskPriority::UsbPriority);
}

#endif

// End
