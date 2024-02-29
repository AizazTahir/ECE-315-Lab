/*
 * uart_driver.c
 * Created on: July 27, 2021
 * Author: Shyama M. Gandhi
 * Modified by : Antonio Andara
 * Modified on : January 24, 2023
 * Implementation of UART driver functions declared in uart_driver.h
 */

#include "uart_driver.h"


// Function definitions
void interruptHandler(void *CallBackRef, u32 event, unsigned int EventData)
{
	if (event == RECEIVED_DATA){
    	handleReceiveEvent();
    }
    else if (event == SENT_DATA){
    	handleSentEvent();
    } else {
    	xil_printf("Neither a RECEIVE event nor a SEND event\n");
    }
}


void handleReceiveEvent()
{
    u8 receive_buffer;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
/*************************** Enter your code here ****************************/
	// TODO 1: increment the receive interrupt counter.
	// implement the logic to check if there are received bytes from the UART.
	// If yes, then read them and send it to the receive queue.
	// the received byte will be stored inside the "receive_buffer" variable.

/*****************************************************************************/
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}


void handleSentEvent()
{
    u8 transmit_data;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
/*************************** Enter your code here ****************************/
		// TODO 2: increment the receive interrupt counter

/*****************************************************************************/
    transmitDataFromQueue(&transmit_data, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    disableTxEmpty();
}

void transmitDataFromQueue(u8 *data, BaseType_t *taskToSwitch)
{
    while (uxQueueMessagesWaitingFromISR(xTxQueue) > 0 && !XUartPs_IsTransmitFull(XPAR_XUARTPS_0_BASEADDR)){
/*************************** Enter your code here ****************************/
		// TODO 3: Read the data from the transmit queue.
		// store the data into the "transmit_data" variable

/*****************************************************************************/
    }
}

void disableTxEmpty()
{
    u32 mask;
    if (uxQueueMessagesWaitingFromISR(xTxQueue) <= 0){
        mask = XUartPs_GetInterruptMask(&UART);
/*************************** Enter your code here ****************************/
		// TODO 4: use XUartPs_SetInterruptMask() to diable the TEMPTY interrupt

/*****************************************************************************/
    }
}


int initializeUART(void)
{
	int Status;

	Config = XUartPs_LookupConfig(UART_DEVICE_ID);
	if (NULL == Config){
		return XST_FAILURE;
		xil_printf("UART PS Config failed\n");
	}

	//Initialize UART
	Status = XUartPs_CfgInitialize( &UART
								  , Config
								  , Config->BaseAddress
								  );

	if (Status != XST_SUCCESS){
		return XST_FAILURE;
		xil_printf("UART PS init failed\n");
	}
	return XST_SUCCESS;
}


int setupInterruptSystem(INTC *IntcInstancePtr, XUartPs *UartInstancePtr, u16 UartIntrId)
{
	int Status;
	XScuGic_Config *IntcConfig; // Config pointer for interrupt controller

	//Lookup the config information for interrupt controller
	IntcConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);
	if (NULL == IntcConfig){
		return XST_FAILURE;
	}

	//Initialize interrupt controller
	Status = XScuGic_CfgInitialize( IntcInstancePtr
								  , IntcConfig
								  , IntcConfig->CpuBaseAddress
								  );

	if (Status != XST_SUCCESS){
		return XST_FAILURE;
	}

	//Connect the interrupt controller interrupt handler
	Xil_ExceptionRegisterHandler( XIL_EXCEPTION_ID_INT
								, (Xil_ExceptionHandler) XScuGic_InterruptHandler
								, IntcInstancePtr
								);

	//Connect the PS UART interrupt handler
	//The interrupt handler which handles the interrupts for the UART peripheral is connected to it's unique ID number (82 in this case)
	Status = XScuGic_Connect( IntcInstancePtr
							, UartIntrId
							, (Xil_ExceptionHandler) XUartPs_InterruptHandler
							, (void *) UartInstancePtr
							);

	if (Status != XST_SUCCESS){
		return XST_FAILURE;
	}

	//Enable the UART interrupt input on the interrupt controller
	XScuGic_Enable(IntcInstancePtr, UartIntrId);

	//Enable the processor interrupt handling on the ARM processor
	Xil_ExceptionEnable();


	//Setup the UART Interrupt handler function
	XUartPs_SetHandler( UartInstancePtr
					  , (XUartPs_Handler)interruptHandler
					  , UartInstancePtr
					  );

	//Create mask for UART interrupt, Enable the interrupt when the receive buffer has reached a particular threshold
	IntrMask = XUARTPS_IXR_TOUT | XUARTPS_IXR_PARITY  | XUARTPS_IXR_FRAMING |
	           XUARTPS_IXR_OVER | XUARTPS_IXR_TXEMPTY | XUARTPS_IXR_RXFULL  |
	           XUARTPS_IXR_RXOVR;

	//Setup the UART interrupt Mask
	XUartPs_SetInterruptMask(UartInstancePtr, IntrMask);

	//Setup the PS UART to Work in Normal Mode
	XUartPs_SetOperMode(UartInstancePtr, XUARTPS_OPER_MODE_NORMAL);

	return XST_SUCCESS;
}

/******************************************************************************
 * Be sure to protect any critical sections by using the FreeRTOS macros
 * taskENTER_CRITICAL() and taskEXIT_CRITICAL().
 *****************************************************************************/

BaseType_t myReceiveData(void)
{
	if (uxQueueMessagesWaiting( xRxQueue ) > 0){
		return pdTRUE;
	} else {
		return pdFALSE;
	}
}


u8 myReceiveByte(void)
{
	u8 recv = 0;
/*************************** Enter your code here ****************************/
	// TODO 5: use myReceiveData() to check if there's data waiting to be received
	// receive a byte from the receive queue and store it into "recv"

/*****************************************************************************/
}


BaseType_t myTransmitFull(void)
{
	if (uxQueueSpacesAvailable(xTxQueue) <= 0){
		return pdTRUE;
	} else {
		return pdFALSE;
	}
}


void mySendByte(u8 data)
{
	u32 mask;
/*************************** Enter your code here ****************************/
	// TODO 6: Add the code to enable TEMPTY interrupt bit.


	if (XUartPs_IsTransmitEmpty(&UART)){
		// use XUartPs_WriteReg() to write a byte to the UART FIFO

/*****************************************************************************/
	} else if (xQueueSend( xTxQueue, &data, 0UL) != pdPASS ){
		xil_printf("Failed to send the data\n");
	}
}
