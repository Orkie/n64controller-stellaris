/*
 * protocol.c
 *
 *  Created on:
 *      Author: Tom
 */
#define TARGET_IS_BLIZZARD_RA1
#include <stdint.h>
#include <stdbool.h>
#include <hw_types.h>
#include <hw_ints.h>
#include <hw_gpio.h>
#include <hw_memmap.h>
#include <hw_timer.h>
#include <debug.h>
#include <gpio.h>
#include <interrupt.h>
#include <timer.h>
#include <sysctl.h>
#include "protocol.h"
#include "n64usbhid.h"

#define RED_LED   GPIO_PIN_1
#define BLUE_LED  GPIO_PIN_2
#define GREEN_LED GPIO_PIN_3

#define READTIMEOUT 100

volatile uint8_t currentBit;
volatile int currentByte;
volatile uint8_t buffer[256];
volatile int txNumBytes;
volatile int interruptCount;
volatile uint8_t outputSteps[4];
volatile bool doSomething;
volatile uint8_t data;

#define CURRENT_BIT() ((data&currentBit) != 0)

#define TIMERINTCLEAR(ulBase, ulIntFlags) (HWREG((ulBase) + TIMER_O_ICR) = (ulIntFlags))
#define TIMERENABLE(ulBase, ulTimer) (HWREG((ulBase) + TIMER_O_CTL) |= (ulTimer) & (TIMER_CTL_TAEN | TIMER_CTL_TBEN))
#define TIMERDISABLE(ulBase, ulTimer) (HWREG((ulBase) + TIMER_O_CTL) &= ~((ulTimer) & (TIMER_CTL_TAEN | TIMER_CTL_TBEN)))
#define GPIOPINWRITE(ulPort, ucPins, ucVal) (HWREGB((ulPort) + GPIO_O_DATA + ((ucPins) << 2)) = (ucVal))
#define GPIODIRSET(ulPort, ucPins, ulPinIO) (HWREG((ulPort) + GPIO_O_DIR) = (((ulPinIO) & 1) ? (HWREG((ulPort) + GPIO_O_DIR) | (ucPins)) : (HWREG((ulPort) + GPIO_O_DIR) & ~(ucPins))))

inline void prepareOutputSteps() {
	if(CURRENT_BIT()) {
		outputSteps[0] = 0x00;
		outputSteps[1] = 0xFF;
		outputSteps[2] = 0xFF;
		outputSteps[3] = 0xFF;
	} else {
		outputSteps[0] = 0x00;
		outputSteps[1] = 0x00;
		outputSteps[2] = 0x00;
		outputSteps[3] = 0xFF;
	}
}

void n64Transmit(uint8_t bytes[], int length) {
	// prepare the tx buffer
	int i;
	for(i = 0 ; i < length ; i++) {
		buffer[i] = bytes[i];
	}

	currentBit = 0x80; // 0b10000000
	currentByte = 0;
	interruptCount = 4;
	txNumBytes = length;
	doSomething = false;

	prepareOutputSteps();
	GPIOPINWRITE(GPIO_PORTB_AHB_BASE, GPIO_PIN_0, 0xFF);
	GPIODIRSET(GPIO_PORTB_AHB_BASE, GPIO_PIN_0, GPIO_DIR_MODE_OUT);
	TIMERENABLE(TIMER0_BASE, TIMER_A);

	while(1) {
		while(interruptCount != 0);
		// we're now ready to load more data
		if(currentByte == txNumBytes) {
			// we've finished
			TIMERDISABLE(TIMER0_BASE, TIMER_A);
			GPIOPINWRITE(GPIO_PORTB_AHB_BASE, GPIO_PIN_0, 0xFF);
			GPIODIRSET(GPIO_PORTB_AHB_BASE, GPIO_PIN_0, GPIO_DIR_MODE_IN);
			GPIOIntEnable(GPIO_PORTA_BASE, GPIO_PIN_0);
			return;
		}

		if((currentBit >>= 1) == 0) { // we've finished the byte
			data = buffer[currentByte++];
			currentBit = 0x80;
			if(currentByte == txNumBytes) { // should the next transmit be the the stop signal?
				// send stop signal
				outputSteps[0] = 0x00;
				outputSteps[1] = 0xFF;
				outputSteps[2] = 0xFF;
				interruptCount = 3;
			} else { // prepare the next bit to send for the new byte
				prepareOutputSteps();
				interruptCount = 4;
			}
		} else {
			// byte not finished, load next bit
			prepareOutputSteps();
			interruptCount = 4;
		}
	}
}

volatile bool trigger = false;
void Timer0IntHandler() {
	TIMERINTCLEAR(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	GPIOPINWRITE(GPIO_PORTB_AHB_BASE, GPIO_PIN_0, outputSteps[interruptCount-1]);
	if(interruptCount != 0) {
		interruptCount--;
	}
}

void GpioBIntHandler() {
	GPIOPinWrite(GPIO_PORTF_BASE, RED_LED|BLUE_LED|GREEN_LED, BLUE_LED);
	GPIOIntDisable(GPIO_PORTA_BASE, GPIO_PIN_0);
}

#define GPIOPINREAD(ulPort, ucPins) \
	(HWREGB((ulPort) + GPIO_O_DATA + ((ucPins) << 2)))

#define TIMERALOADSET(ulBase, ulValue) \
	(HWREG((ulBase) + TIMER_O_TAILR) = (ulValue))


static bool g_ucHasInitialized = false;
static volatile unsigned char g_ucNextDelay;

void GCN64InitializeProtocol(void)
{
	if (g_ucHasInitialized)
	{
		return;
	}

	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);

	TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);

	// 1us resolution
	TimerLoadSet(TIMER0_BASE, TIMER_A, SysCtlClockGet() / 1000000);

	TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

	IntMasterEnable();

	IntEnable(INT_TIMER0A);

	/*
	 *  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
  SysCtlDelay(3);
  GPIOPinTypeGPIOInput(GPIO_PORTA_BASE, GPIO_PIN_2);
  GPIOIntEnable(GPIO_PORTA_BASE, GPIO_PIN_2);
  GPIOIntTypeSet(GPIO_PORTA_BASE, GPIO_PIN_2,GPIO_BOTH_EDGES);
  GPIOIntRegister(GPIO_PORTA_BASE,inputInt);
	 *
	 */

//	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
//	IntEnable(INT_GPIOB);
//	GPIOIntTypeSet(GPIO_PORTB_BASE, GPIO_PIN_0, GPIO_FALLING_EDGE);
//	GPIOPinWrite(GPIO_PORTF_BASE, RED_LED|BLUE_LED|GREEN_LED, BLUE_LED);

	GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_4);
	GPIOIntTypeSet(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_RISING_EDGE);
	GPIOIntEnable(GPIO_PORTF_BASE, GPIO_PIN_4);

	g_ucHasInitialized = true;
}

inline void delayUs(int i) {
	g_ucNextDelay = i;
	while (g_ucNextDelay != 0);
}

unsigned long GCN64Send(unsigned long ulPort, unsigned char ucPin, unsigned char *pucBuffer, unsigned long ulToSend)
{
	unsigned long i;
	unsigned char j;

	GPIOPINWRITE(ulPort, ucPin, 0xFF);

	GPIODIRSET(ulPort, ucPin, GPIO_DIR_MODE_OUT);

	g_ucNextDelay = 1;

	TIMERENABLE(TIMER0_BASE, TIMER_A);

	for (i = 0; i < ulToSend; i++)
	{
		for (j = 0x80; j != 0x00; j >>= 1)
		{
			while (g_ucNextDelay != 0)
			{
			}

			GPIOPINWRITE(ulPort, ucPin, 0);

			if (pucBuffer[i] & j)
			{
				g_ucNextDelay = 1;
			}
			else
			{
				g_ucNextDelay = 3;
			}

			while (g_ucNextDelay != 0)
			{
			}

			GPIOPINWRITE(ulPort, ucPin, 0xFF);

			if (pucBuffer[i] & j)
			{
				g_ucNextDelay = 3;
			}
			else
			{
				g_ucNextDelay = 1;
			}
		}
	}

	while (g_ucNextDelay != 0)
	{
	}

	// Stop bit
	GPIOPINWRITE(ulPort, ucPin, 0);

	g_ucNextDelay = 1;

	while (g_ucNextDelay != 0)
	{
	}

	GPIOPINWRITE(ulPort, ucPin, 0xFF);

	TIMERDISABLE(TIMER0_BASE, TIMER_A);

	return i;
	/*unsigned long i;
	unsigned char j;

	GPIOPINWRITE(ulPort, ucPin, 0xFF);

	GPIODIRSET(ulPort, ucPin, GPIO_DIR_MODE_OUT);

	g_ucNextDelay = 0;

	TIMERENABLE(TIMER0_BASE, TIMER_A);

	for (i = 0; i < ulToSend; i++)
	{
		for (j = 0x80; j != 0x00; j >>= 1)
		{
			if(pucBuffer[i] & j) {
				// we want to send a 1, low 1us, high 3us
				GPIOPINWRITE(ulPort, ucPin, 0);
				delayUs(1);
				GPIOPINWRITE(ulPort, ucPin, 0xFF);
				delayUs(3);
			} else {
				// we want to send a 0, low 3us, high 1us
				GPIOPINWRITE(ulPort, ucPin, 0);
				delayUs(3);
				GPIOPINWRITE(ulPort, ucPin, 0xFF);
				delayUs(1);
			}
		}
	}

	// Stop bit
	GPIOPINWRITE(ulPort, ucPin, 0);
	delayUs(1);
	GPIOPINWRITE(ulPort, ucPin, 0xFF);
	delayUs(2);

	GPIODIRSET(ulPort, ucPin, GPIO_DIR_MODE_IN);

	delayUs(20);

	TIMERDISABLE(TIMER0_BASE, TIMER_A);

	return i;*/
}

unsigned long GCN64Receive(unsigned long ulPort, unsigned char ucPin, unsigned char *pucBuffer)
{
	unsigned long i, l1, l2;
	unsigned char j, k;

	GPIODIRSET(ulPort, ucPin, GPIO_DIR_MODE_IN);

	for (l1 = 0; GPIOPINREAD(ulPort, ucPin) && l1 < READTIMEOUT; l1++)
	{
	}

	// Timeout
	if (l1 == READTIMEOUT)
	{
		return 0;
	}

	for (i = 0; ; i++)
	{
		for (j = 0x80, k = 0; j != 0x00; j >>= 1)
		{
			for (l1 = 0; !GPIOPINREAD(ulPort, ucPin) && l1 < READTIMEOUT; l1++)
			{
			}

			for (l2 = 0; GPIOPINREAD(ulPort, ucPin) && l2 < READTIMEOUT; l2++)
			{
			}

			// Stop bit
			if (l1 != READTIMEOUT && l2 == READTIMEOUT)
			{
				return i;
			}

			// Timeout
			if (l1 == READTIMEOUT || l2 == READTIMEOUT)
			{
				return 0;
			}

			if (l1 < l2)
			{
				k |= j;
			}
		}

		pucBuffer[i] = k;
	}
/*	IntDisable(INT_USB0);
	GPIODIRSET(ulPort, ucPin, GPIO_DIR_MODE_IN);

	unsigned long byteNo = 0;
	unsigned char bitNo = 0;
	while(1) {
		unsigned char byte = 0x00;
		for (bitNo = 0x80; bitNo != 0x00; bitNo >>= 1) {
			unsigned long lowTicks = 0;
			unsigned long highTicks = 0;

			while(!GPIOPINREAD(ulPort, ucPin)) {
				lowTicks++;
			}
			while(GPIOPINREAD(ulPort, ucPin) && highTicks < READTIMEOUT) {
				highTicks++;
			}

			if(highTicks == READTIMEOUT) {
				// stop bit has been and gone
				IntEnable(INT_USB0);
				return byteNo;
			} else if(lowTicks < highTicks) {
				byte |= bitNo;
			}
		}

		pucBuffer[byteNo++] = byte;
	}*/
}
