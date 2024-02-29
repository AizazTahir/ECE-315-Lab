/*
 * uart_driver.h
 *
 * Created on: July 27, 2021
 * Author: Shyama M. Gandhi
 * Modified by : Antonio Andara
 * Modified on : January 24, 2023
 */

#ifndef SRC_UART_DRIVER_H_
#define SRC_UART_DRIVER_H_

#include "xil_io.h"
#include "xuartps.h" // UART definitions header file
#include "xscugic.h" // Interrupt controller header file

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "queue.h"

// Macros
#define INTC               	XScuGic
#define UART_DEVICE_ID     	XPAR_XUARTPS_0_DEVICE_ID
#define INTC_DEVICE_ID     	XPAR_SCUGIC_SINGLE_DEVICE_ID
#define UART_INT_IRQ_ID    	XPAR_XUARTPS_1_INTR
#define UART_RX_BUFFER_SIZE 3U
#define RECEIVED_DATA       XUARTPS_EVENT_RECV_DATA
#define SENT_DATA           XUARTPS_EVENT_SENT_DATA
#define SIZE_OF_QUEUE      	100

// External variable declarations
XUartPs UART;
XUartPs_Config *Config;
INTC InterruptController;
u32 IntrMask;

// Queues
QueueHandle_t xTxQueue;
QueueHandle_t xRxQueue;

// Interrupt counters
int countRxIrq;
int countTxIrq;

// Function prototypes
void interruptHandler(void *CallBackRef, u32 Event, unsigned int EventData);
void handleReceiveEvent();
void handleSentEvent();
void transmitDataFromQueue(u8 *data, BaseType_t *taskToSwitch);
void disableTxEmpty();
int initializeUART(void);
int setupInterruptSystem(INTC *IntcInstancePtr, XUartPs *UartInstancePtr, u16 UartIntrId);
BaseType_t myReceiveData(void);
u8 myReceiveByte(void);
BaseType_t myTransmitFull(void);
void mySendByte(u8 Data);

#endif /* SRC_UART_DRIVER_H_ */
