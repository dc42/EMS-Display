/*
 * Main.cpp
 *
 *  Created on: 30 Oct 2022
 *      Author: David
 */

#include <Core.h>
#include <RP2040/Devices.h>
#include <TaskPriorities.h>
#include <Display.h>
#include <DisplayInterface.h>
#include <LedDriver.h>
#include <hardware/timer.h>
#include <malloc.h>

#include <FreeRTOS.h>
#include <task.h>
#include <freertos_task_additions.h>

#include <syscalls.h>
#include <hardware/watchdog.h>

// Main task data
constexpr unsigned int MainTaskStackWords = 1000;
static Task<MainTaskStackWords> mainTask;
static Mutex mallocMutex;

// Idle task data
constexpr unsigned int IdleTaskStackWords = 200;				// currently we don't use the idle task for anything, so this can be quite small
static Task<IdleTaskStackWords> idleTask;

// Make malloc/free thread safe. We must use a recursive mutex for it.
extern "C" void __malloc_lock (struct _reent *_r) noexcept
{
	if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING)		// don't take mutex if scheduler not started or suspended
	{
		mallocMutex.Take();
	}
}

extern "C" void __malloc_unlock (struct _reent *_r) noexcept
{
	if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING)		// don't release mutex if scheduler not started or suspended
	{
		mallocMutex.Release();
	}
}

extern "C" void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize) noexcept
{
	*ppxIdleTaskTCBBuffer = idleTask.GetTaskMemory();
	*ppxIdleTaskStackBuffer = idleTask.GetStackBase();
	*pulIdleTaskStackSize = idleTask.GetStackSize();
}

#if configUSE_TIMERS

// Timer task data
constexpr unsigned int TimerTaskStackWords = 60;
static Task<TimerTaskStackWords> timerTask;

extern "C" void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize) noexcept
{
    *ppxTimerTaskTCBBuffer = timerTask.GetTaskMemory();
    *ppxTimerTaskStackBuffer = timerTask.GetStackBase();
    *pulTimerTaskStackSize = timerTask.GetStackSize();
}

#endif

extern "C" [[noreturn]] void MainTask(void*) noexcept
{
	serialUSB.Start();
	LedDriver::Init();
	DisplayPortsInit();
	for (unsigned int i = 0; i < 5; ++i)
	{
		LedDriver::SetColour(0, 0, 255);
		delay(500);
		LedDriver::SetColour(0, 255, 0);
//		serialUSB.printf("Hello!\n");
		delay(500);
	}

	Display::Init();
	Display::HelloWorld();
	for (;;)
	{
		Display::Spin();
	}
}

// Program entry point after initialisation
[[noreturn]] void AppMain() noexcept
{
	// Initialise systick (needed for delayMicroseconds calls to work before FreeRTOS starts up)
	SysTick->LOAD = ((SystemCoreClockFreq/1000) - 1) << SysTick_LOAD_RELOAD_Pos;
	SysTick->CTRL = (1 << SysTick_CTRL_ENABLE_Pos) | (1 << SysTick_CTRL_CLKSOURCE_Pos);

	CoreInit();
	DeviceInit();

	// Initialise the tasks
	idleTask.AddToList();			// add the FreeRTOS internal tasks to the task list

#if configUSE_TIMERS
	timerTask.AddToList();
#endif

	// Create the startup task and memory allocation mutex
	mainTask.Create(MainTask, "MAIN", nullptr, TaskPriority::SpinPriority);
	mallocMutex.Create("Malloc");

	// Initialise watchdog clock
	watchdog_start_tick(XOSC_MHZ);
	watchdog_enable(1000, true);

	vTaskStartScheduler();			// doesn't return
	while (true) { }
}

// System tick hook, called from FreeRTOS tick handler
extern "C" void vApplicationTickHook(void) noexcept
{
	CoreSysTick();
	watchdog_update();
	Display::Tick();
}

// This is called from FreeRTOS to measure the CPU time used by a task. It is not essential.
extern "C" uint32_t StepTimerGetTimerTicks() noexcept
{
	return time_us_32();
}

// Function called by FreeRTOS and internally to reset the run-time counter and return the number of timer ticks since it was last reset
extern "C" uint32_t TaskResetRunTimeCounter() noexcept
{
	static uint32_t whenLastReset = 0;
	const uint32_t now = time_us_32();
	const uint32_t ret = now - whenLastReset;
	whenLastReset = now;
	return ret;
}

// Stack overflow hook, called from FreeRTOS if a stack overflow is detected
extern "C" void vApplicationStackOverflowHook() noexcept
{
//	SetBacklight(false);
	for (;;) { }
}

// This is called if an assertion within FreeRTOS fails
extern "C" void vAssertCalled(uint32_t ulLine, const char *pcFile) noexcept
{
//	SetBacklight(false);
	for (;;) { }
}

// The fault handler implementation calls a function called hardFaultDispatcher()
extern "C" void isr_hardfault() noexcept /*__attribute__((naked))*/;
void isr_hardfault() noexcept
{
	LedDriver::SetColour(255, 0, 0);			// LEDs to red
	for (;;) { }
}

// End
