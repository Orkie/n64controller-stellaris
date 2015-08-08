/*
 * n64usbhid.h
 *
 *  Created on: 26 Jul 2015
 *      Author: adan
 */

#ifndef N64USBHID_H_
#define N64USBHID_H_
#include <stdbool.h>

extern void initN64USB();
extern bool isRxDataAvailable();
extern bool isTxDataAvailable();
extern bool isConnected();
extern unsigned long isDataReadyToRead();
extern uint8_t* receiveUsbData(unsigned long length);
extern void sendString(char *pcString, ...);
extern void sendUsbData(uint8_t* buffer, int length);

#endif /* N64USBHID_H_ */
