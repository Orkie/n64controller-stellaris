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
	delayUs(1);
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
	TimerLoadSet(TIMER0_BASE, TIMER_A, SysCtlClockGet() / 1000000);

	TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

	IntMasterEnable();

	IntEnable(INT_TIMER0A);

	GPIOPinTypeGPIOInput(GPIO_PORTB_AHB_BASE, GPIO_PIN_0);
	GPIOIntTypeSet(GPIO_PORTB_AHB_BASE, GPIO_PIN_0, GPIO_FALLING_EDGE);
	GPIOIntEnable(GPIO_PORTB_AHB_BASE, GPIO_PIN_0);

	g_ucHasInitialized = true;
}
