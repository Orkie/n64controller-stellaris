/*
 * protocol.h
 *
 *  Created on:
 *      Author: Tom
 */

#ifndef PROTOCOL_H_
#define PROTOCOL_H_

void GCN64InitializeProtocol(void);

void n64Transmit(uint8_t bufer[], int length);
int n64Receive(uint8_t buffer[]);

#endif /* PROTOCOL_H_ */
