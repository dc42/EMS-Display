/*
 * Main.cpp
 *
 *  Created on: 30 Oct 2022
 *      Author: David
 */

#include <Core.h>
#include <syscalls.h>
#include <RP2040/Devices.h>
#include <TaskPriorities.h>
#include <Display.h>
#include <hardware/timer.h>

// Main task data
constexpr unsigned int MainTaskStackWords = 500;
static Task<MainTaskStackWords> mainTask;
static Mutex mallocMutex;

// Idle task data
constexpr unsigned int IdleTaskStackWords = 50;					// currently we don't use the idle talk for anything, so this can be quite small
static Task<IdleTaskStackWords> idleTask;

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

extern "C" void MainTask(void*) noexcept
{
	Display::Init();
	for (;;) { }
}

// Program entry point after initialisation
void AppMain() noexcept
{
	CoreInit();
	DeviceInit();

	// Initialise the tasks
	idleTask.AddToList();			// add the FreeRTOS internal tasks to the task list

#if configUSE_TIMERS
	timerTask.AddToList();
#endif

	// Initialise watchdog clock
	WatchdogInit();

	// Create the startup task and memory allocation mutex
	mainTask.Create(MainTask, "MAIN", nullptr, TaskPriority::SpinPriority);
	mallocMutex.Create("Malloc");

	vTaskStartScheduler();			// doesn't return
	while (true) { }
}

// System tick hook, called from FreeRTOS tick handler
extern "C" void vApplicationTickHook(void) noexcept
{
	CoreSysTick();
	WatchdogReset();							// kick the watchdog
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
	for (;;) { }
}

// This is called if an assertion within FreeRTOS fails
extern "C" void vAssertCalled(uint32_t ulLine, const char *pcFile) noexcept
{
	for (;;) { }
}

// End
