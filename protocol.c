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
volatile int currentByteNumber;
volatile uint8_t buffer[256];
volatile int txNumBytes;
volatile int interruptCount;
volatile uint8_t outputSteps[4];
volatile uint8_t data;
volatile bool newBit;
volatile uint32_t lowTime;
volatile uint32_t highTime;

#define CURRENT_BIT() ((data&currentBit) != 0)

#define TIMERINTCLEAR(ulBase, ulIntFlags) (HWREG((ulBase) + TIMER_O_ICR) = (ulIntFlags))
#define TIMERENABLE(ulBase, ulTimer) (HWREG((ulBase) + TIMER_O_CTL) |= (ulTimer) & (TIMER_CTL_TAEN | TIMER_CTL_TBEN))
#define TIMERDISABLE(ulBase, ulTimer) (HWREG((ulBase) + TIMER_O_CTL) &= ~((ulTimer) & (TIMER_CTL_TAEN | TIMER_CTL_TBEN)))
#define GPIOPINWRITE(ulPort, ucPins, ucVal) (HWREGB((ulPort) + GPIO_O_DATA + ((ucPins) << 2)) = (ucVal))
#define GPIODIRSET(ulPort, ucPins, ulPinIO) (HWREG((ulPort) + GPIO_O_DIR) = (((ulPinIO) & 1) ? (HWREG((ulPort) + GPIO_O_DIR) | (ucPins)) : (HWREG((ulPort) + GPIO_O_DIR) & ~(ucPins))))
#define GPIOPINREAD(ulPort, ucPins) (HWREGB((ulPort) + GPIO_O_DATA + ((ucPins) << 2)))

volatile uint8_t currentByte;
volatile uint8_t currentBitMask;
volatile int currentByteNumber;

#define OUTPUT_HIGH()	GPIOPINWRITE(GPIO_PORTB_AHB_BASE, GPIO_PIN_0, 0xFF)
#define OUTPUT_LOW()	GPIOPINWRITE(GPIO_PORTB_AHB_BASE, GPIO_PIN_0, 0x00)

inline void delayUs(int us) {
	interruptCount = us;
	while(interruptCount != 0);
}

inline void transmit0() {
	OUTPUT_LOW();
	delayUs(3);
	OUTPUT_HIGH();
	delayUs(1);
}

inline void transmit1() {
	OUTPUT_LOW();
	delayUs(1);
	OUTPUT_HIGH();
	delayUs(3);
}

inline void transmitStop() {
	OUTPUT_LOW();
	delayUs(1);
	OUTPUT_HIGH();
	delayUs(2);
}

void n64Transmit(uint8_t buffer[], int length) {
	currentByteNumber = 0;
	interruptCount = 0;

	GPIOPINWRITE(GPIO_PORTB_AHB_BASE, GPIO_PIN_0, 0xFF);
	GPIODIRSET(GPIO_PORTB_AHB_BASE, GPIO_PIN_0, GPIO_DIR_MODE_OUT);
	TIMERENABLE(TIMER0_BASE, TIMER_A);

	while(currentByteNumber < length) {
		// load byte
		currentByte = buffer[currentByteNumber];
		currentBitMask = 0x80;

		// transmit a byte
		while(currentBitMask != 0) {
			if(currentByte & currentBitMask) {
				// transmit 1
				transmit1();
			} else {
				// transmit 0
				transmit0();
			}

			// shift the mask
			currentBitMask >>= 1;
		}

		currentByteNumber++;
	}

	// write stop bit
	transmitStop();

	// done!
	TIMERDISABLE(TIMER0_BASE, TIMER_A);
	GPIOPINWRITE(GPIO_PORTB_AHB_BASE, GPIO_PIN_0, 0xFF);
}

int n64Receive(uint8_t buffer[]) {
	int bytesRead = 0;
	bool firstLoop = false;
	newBit = false;
	currentByteNumber = 0;
	currentBit = 0x80;
	lowTime = 0;
	highTime = 0;
	data = 0x00;

	GPIODIRSET(GPIO_PORTB_AHB_BASE, GPIO_PIN_0, GPIO_DIR_MODE_IN);
	IntEnable(INT_GPIOB);

	while(!newBit);
	newBit = false;

	while(1) {
		if(newBit && !firstLoop) {
			if(highTime > lowTime) {
				data |= currentBit;
			}

			lowTime = 0;
			highTime = 0;
			newBit = false;
			if((currentBit >>= 1) == 0) {
				// new byte!
				currentBit = 0x80;
				buffer[currentByteNumber++] = data;
				data = 0;
				bytesRead++;
			}
		}
		firstLoop = false;

		while(!GPIOPINREAD(GPIO_PORTB_AHB_BASE, GPIO_PIN_0)) { lowTime++; }
		while(GPIOPINREAD(GPIO_PORTB_AHB_BASE, GPIO_PIN_0)) {
			if(highTime++ > 2000) { // we've reached the end
				IntDisable(INT_GPIOB);
				return bytesRead;
			}
		}
	}
}

void Timer0IntHandler() {
	TIMERINTCLEAR(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	if(interruptCount != 0) {
		interruptCount--;
	}
}

void GpioBIntHandler() {
	GPIOIntClear(GPIO_PORTB_AHB_BASE,GPIO_PIN_0);
	newBit = true;
}

#define TIMERALOADSET(ulBase, ulValue) \
	(HWREG((ulBase) + TIMER_O_TAILR) = (ulValue))


static bool g_ucHasInitialized = false;
static volatile unsigned char g_ucNextDelay;

void GCN64InitializeProtocol(void)
{
	if (g_ucHasInitialized) {
		return;
	}

	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);

	TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);

	// 1us resolution
//	TimerLoadSet(TIMER0_BASE, TIMER_A, SysCtlClockGet() / 1000000);
	TimerLoadSet(TIMER0_BASE, TIMER_A, SysCtlClockGet() / 1000000);

	TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

	IntMasterEnable();

	IntEnable(INT_TIMER0A);

	GPIOPinTypeGPIOInput(GPIO_PORTB_AHB_BASE, GPIO_PIN_0);
	GPIOIntTypeSet(GPIO_PORTB_AHB_BASE, GPIO_PIN_0, GPIO_FALLING_EDGE);
	GPIOIntEnable(GPIO_PORTB_AHB_BASE, GPIO_PIN_0);

	g_ucHasInitialized = true;
}

//inline void delayUs(int i) {
//	g_ucNextDelay = i;
//	while (g_ucNextDelay != 0);
//}

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
