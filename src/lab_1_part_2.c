


/*
 * KEYPAD AND SEVEN-SEGEMENT DECODER IMPLEMENTATION FOR PART-1 LAB-1
 *
 * The code will read the key pressed from the PMOD kypd module and the corresponding value will be displayed on both
 * the SSDs (i.e., left as well as right side segments)
 * Initially, the delay of 250 ms is used between left and right segment switching so user can clearly see
 * that both segment do not appear to lit at the same time.
 * Try to increase or decrease this delay period and see how you see the segments being displayed
 * Increase the frequency to a point where both digits are lit-up simultaneously. Can you find this approximate value
 * of frequency/time period?
 *
 *
 *  ECE- 315 WINTER 2024 - COMPUTER INTERFACING COURSE
 *
 *  Created on: 	Date:::5 February, 2021
 *  Modified on:    Date:::26 July, 2023
 *
 *      Authors: 	Shyama M. Gandhi
 *                  Antonio Andara
 */

//Include FreeRTOS Libraries
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

//Include xilinx Libraries
#include "xparameters.h"
#include "xgpio.h"
#include "xscugic.h"
#include "xil_exception.h"
#include "xil_printf.h"

//Other miscellaneous libraries
#include "pmodkypd.h"
#include "sleep.h"
#include "xil_cache.h"
#include "stdbool.h"
#include "stdio.h"
#include "stdlib.h"

/* Parameter definitions */
// Device ID declarations
#define SSD_DEVICE_ID   XPAR_AXI_SSD_DEVICE_ID
#define KYPD_DEVICE_ID  XPAR_AXI_KEYPAD_DEVICE_ID
#define RGB_DEVICE_ID	XPAR_AXI_LEDS_DEVICE_ID
#define LEDS_DEVICE_ID	XPAR_AXI_LEDS_DEVICE_ID
#define BTN_DEVICE_ID	XPAR_AXI_GPIO_0_DEVICE_ID
#define SW_DEVICE_ID	XPAR_AXI_GPIO_0_DEVICE_ID

// Device channels
#define SSD_CHANNEL		1
#define KYPD_CHANNEL	1
#define LEDS_CHANNEL	1
#define RGB_CHANNEL		2
#define BTN_CHANNEL		1
#define SW_CHANNEL		2

// Button press values
#define BTN0 1
#define BTN1 2
#define BTN2 4
#define BTN3 8


// keypad key table
#define DEFAULT_KEYTABLE "0FED789C456B123A"

// miscellaneous
#define SSD_DELAY     17
#define COMMAND_DELAY 50
#define DELAY_500 	  500



// Device declarations
XGpio SSDInst, RGBInst, btnInst, swInst, greenLedsInst;
PmodKYPD KYPDInst;

// task declarations
static void keypadTask   (void *pvParameters);
static void sevenSegTask (void *pvParameters);
static void commandTask  (void *pvParameters);
static void RGBLedTask   (void *pvParameters);
static void GreenLedTask (void *pvParameters);

// queue declarations
static QueueHandle_t xSSDQueue 	   = NULL;
static QueueHandle_t xCommandQueue = NULL;
static QueueHandle_t xRGBQueue 	   = NULL;
static QueueHandle_t xLedQueue     = NULL;

// Message struct declaration
// This will be used by the command handlers
typedef struct
{
	char type;
    char action;
} Message;

// Function prototypes
void InitializeKeypad();
u32 SSD_decode(u8 key_value, u8 cathode);
static void HandleECCommand(Message* message);
static void HandleEFCommand(Message* message);
static void HandleD5Command(Message* message);
static void HandleD4Command(Message* message);
static void HandleE7Command(Message* message);
static void HandleA5Command(Message* message);
static void HandleA3Command(Message* message);
static void HandleUnknownCommand(const char* command);

int main(void)
{
	int status;

	/* Device Initialization*/
	// keypad
	InitializeKeypad();

	// SSD
	status = XGpio_Initialize(&SSDInst, SSD_DEVICE_ID);
	if(status != XST_SUCCESS){
		xil_printf("GPIO Initialization for SSD failed.\r\n");
		return XST_FAILURE;
	}

	// RGB led
	status = XGpio_Initialize(&RGBInst, RGB_DEVICE_ID);
	if(status != XST_SUCCESS){
		xil_printf("GPIO Initialization for SSD failed.\r\n");
		return XST_FAILURE;
	}

	// Buttons
	status = XGpio_Initialize(&btnInst, BTN_DEVICE_ID);
	if(status != XST_SUCCESS){
		xil_printf("GPIO Initialization for SSD failed.\r\n");
		return XST_FAILURE;
	}

	// Switches
	status = XGpio_Initialize(&swInst, SW_DEVICE_ID);
	if(status != XST_SUCCESS){
		xil_printf("GPIO Initialization for switches failed.\r\n");
		return XST_FAILURE;
	}

	// Green leds
	status = XGpio_Initialize(&greenLedsInst, LEDS_DEVICE_ID);
	if(status != XST_SUCCESS){
		xil_printf("GPIO Initialization for green leds failed.\r\n");
		return XST_FAILURE;
	}

	/* Device data direction: 0 for output 1 for input */
	XGpio_SetDataDirection(&SSDInst, SSD_CHANNEL, 0x00);
	XGpio_SetDataDirection(&greenLedsInst, LEDS_CHANNEL, 0x00);
	XGpio_SetDataDirection(&RGBInst, RGB_CHANNEL, 0x00);
	XGpio_SetDataDirection(&btnInst, BTN_CHANNEL, 0x0F);
	XGpio_SetDataDirection(&swInst, SW_CHANNEL, 0x0F);

	/* Task creation */
    xTaskCreate( keypadTask,			  // The function that implements the task.
                "main task", 			  // Text name for the task, provided to assist debugging only.
                configMINIMAL_STACK_SIZE, // The stack allocated to the task.
                NULL, 					  // The task parameter is not used, so set to NULL.
                tskIDLE_PRIORITY,		  // The task runs at the idle priority.
                NULL );                   // Optional task's handle

    xTaskCreate( sevenSegTask,
				"SSD task",
				configMINIMAL_STACK_SIZE,
				NULL,
				tskIDLE_PRIORITY,
				NULL );

	xTaskCreate(
		commandTask,                     // Task function
		"command task",                  // Name for the task, for debugging purposes
		configMINIMAL_STACK_SIZE,        // Stack depth in words, not bytes
		NULL,                            // Task input parameters
		tskIDLE_PRIORITY + 1,            // Priority, with 1 level higher than idle
		NULL                             // Pointer to the task handle
	);

    xTaskCreate( RGBLedTask,
                "RGB LED task",
                configMINIMAL_STACK_SIZE,
                NULL,
                tskIDLE_PRIORITY,
                NULL );

    xTaskCreate( GreenLedTask,
                "green LEDs task",
                configMINIMAL_STACK_SIZE,
                NULL,
                tskIDLE_PRIORITY,
                NULL );

    /* Queue creation */
    xSSDQueue     = xQueueCreate(1, sizeof(char));
    xCommandQueue = xQueueCreate(1, sizeof(char[3]));
    xRGBQueue 	  = xQueueCreate(1, sizeof(Message));
    xLedQueue 	  = xQueueCreate(1, sizeof(Message));

    // Assert queue creation
    configASSERT(xSSDQueue);
    configASSERT(xCommandQueue);
	configASSERT(xRGBQueue);
	configASSERT(xLedQueue);

    xil_printf(
        "\n====== App Ready ======\n"
        "Input commands using the 16-key keypad.\n"
        "Press 'BTN0' to execute.\n"
        "========================\n\n"
        );

    /* Start Scheduler */
    vTaskStartScheduler();

    while(1);
    return 0;
}

// Keypad initialization function
void InitializeKeypad()
{
   KYPD_begin(&KYPDInst, XPAR_AXI_KEYPAD_BASEADDR);
   KYPD_loadKeyTable(&KYPDInst, (u8*) DEFAULT_KEYTABLE);
}

// This function translates key value codes to their binary representation
u32 SSD_decode(u8 key_value, u8 cathode)
{
    u32 result;

    // key_value is the ASCII code of the pressed key
	// The switch statement maps each ASCII code to the corresponding
	// 7-segment display encoding. The 7-segment display encoding is
	// represented as a binary number where each bit corresponds to a segment.
	// A bit value of 1 means the segment is on, and 0 means it's off.
    switch(key_value){
        case 48: result = 0b00111111; break; // 0
        case 49: result = 0b00110000; break; // 1
        case 50: result = 0b01011011; break; // 2
        case 51: result = 0b01111001; break; // 3
        case 52: result = 0b01110100; break; // 4
        case 53: result = 0b01101101; break; // 5
        case 54: result = 0b01101111; break; // 6
        case 55: result = 0b00111000; break; // 7
        case 56: result = 0b01111111; break; // 8
        case 57: result = 0b01111100; break; // 9
        case 65: result = 0b01111110; break; // A
        case 66: result = 0b01100111; break; // B
        case 67: result = 0b00001111; break; // C
        case 68: result = 0b01110011; break; // D
        case 69: result = 0b01001111; break; // E
        case 70: result = 0b01001110; break; // F
        default: result = 0b00000000; break; // Undefined, all segments are OFF
    }

    // cathode determines which of the two 7-segment displays is active.
	// - A cathode value of 1 activates the right display
	// - A cathode value of 0 activates the left display
	// The Most Significant Bit (MSB) is used as the control bit to select the display.
	// The MSB is set to 1 for the right display and left as 0 for the left display.
    if(cathode==0){
    	return result; // MSB is 0, left display active
    }
    else {
    	return result | 0b10000000; // MSB is set to 1, right display active
    }
}

/**
 * This task is responsible for continuously monitoring the state of a keypad
 * and sending the detected key presses to a queue for further processing.
 **/
static void keypadTask( void *pvParameters )
{
   u16 keystate;
   XStatus status, last_status = KYPD_NO_KEY;
   u8 new_key='0';

   while (1){
	  // Reading the keypad state
	  keystate = KYPD_getKeyStates(&KYPDInst);
	  status = KYPD_getKeyPressed(&KYPDInst, keystate, &new_key);

	  // Sending key presses using the queue
	  if(status == KYPD_SINGLE_KEY && last_status == KYPD_NO_KEY){
		  xQueueOverwrite(xSSDQueue, &new_key);
	  } else if (status == KYPD_MULTI_KEY && status != last_status){
		  xil_printf("Error: Multiple keys pressed\r\n");
	  }

	  // updating last_status
      last_status = status;

      // Delay to throttle the loop
      vTaskDelay(pdMS_TO_TICKS(COMMAND_DELAY));
   }
}


/**
 * This task is responsible for displaying characters on a seven-segment display (SSD)
 * based on key presses received from a queue and sending them to another command task.
 */
static void sevenSegTask( void *pvParameters )
{
    u8 current_key = 'x';
    u32 ssd_value = 0; // Value to be displayed on the SSD
    char command[3] = {'x', 'x', '\0'}; // Array to hold the command for the command task

    while(1){
        // Attempt to receive a key press from the queue
        if(xQueueReceive(xSSDQueue, &current_key, 0) == pdTRUE){
            if(current_key == 'r') {
                // If 'r' is received, reset the current and previous keys
                command[0] = 'x';
                command[1] = 'x';
            } else {
                // Update the command for the command task
                command[0] = command[1];
                command[1] = current_key;
            }

            // Send the command to the command task queue
            xQueueOverwrite(xCommandQueue, &command);

        }

        // Display the current key on the SSD
		ssd_value = SSD_decode(command[1], 1);
		XGpio_DiscreteWrite(&SSDInst, SSD_CHANNEL, ssd_value);
		vTaskDelay(pdMS_TO_TICKS(SSD_DELAY)); // Delay for persistence of vision

		// Display the previous key (now the current key after shifting) on the SSD
        ssd_value = SSD_decode(command[0], 0);
        XGpio_DiscreteWrite(&SSDInst, SSD_CHANNEL, ssd_value);
        vTaskDelay(pdMS_TO_TICKS(SSD_DELAY)); // Delay for persistence of vision
    }
}


static void commandTask( void *pvParameters )
{
	char command[3] = {'x', 'x', '\0'};
	const char RESET_CHAR = 'r';
	unsigned int buttonVal=0, lastButtonVal=0;
	Message message = {.type = 'x', .action = 'x'};

	while(1){

        xQueueReceive(xCommandQueue, &command, 0);
        buttonVal = XGpio_DiscreteRead(&btnInst, 1);

        if(lastButtonVal == 0 && buttonVal == BTN0){
            if(strcmp(command, "E7") == 0){
            	HandleE7Command(&message);
            } else if(strcmp(command, "EC") == 0){
            	HandleECCommand(&message);
            } else if(strcmp(command, "EF") == 0){
            	HandleEFCommand(&message);
			} else if(strcmp(command, "A5") == 0){
				HandleA5Command(&message);
            } else if(strcmp(command, "D5") == 0){
            	HandleD5Command(&message);
            } else if(strcmp(command, "D4") == 0){
				HandleD4Command(&message);
            } else if(strcmp(command, "A3") == 0){
/*************************** Enter your code here ****************************/
			// TODO: Implement handling for the command "A3".
            	HandleA3Command(&message);

/*****************************************************************************/
            } else {
            	HandleUnknownCommand(command);
            }

            xQueueOverwrite(xSSDQueue, &RESET_CHAR);
            vTaskDelay(pdMS_TO_TICKS(DELAY_500)); // Delay after finish
        }

        lastButtonVal = buttonVal;
        // Delay to throttle the loop
        vTaskDelay(pdMS_TO_TICKS(COMMAND_DELAY));
	}
}


static void GreenLedTask( void *pvParameters )
{
    u8 greenLedsValue = 0;
    Message message = { .type = 'x', .action = 'x'};

    while(1){
        xQueueReceive(xLedQueue, &message, portMAX_DELAY);

        // Update green LEDs values
        switch(message.type){
            case 'a': // set the green LEDs to the values of the switches
                // Read the switch values into greenLedsValue
                greenLedsValue = XGpio_DiscreteRead(&swInst, SW_CHANNEL);
                break;

            case 's': // shift values
                if(message.action=='L'){
                    greenLedsValue = (greenLedsValue >> 1) | (greenLedsValue << (8 - 1));
                    xil_printf("shift left\n");
                } else if(message.action=='R'){
                    greenLedsValue = (greenLedsValue << 1) | (greenLedsValue >> (8 - 1));
                    xil_printf("shift right\n");
                }
                greenLedsValue &= 0xF; // Assuming we are using only 4 LEDs
                break;

            case 'c': // Toggle all green LEDs on or off
                if(greenLedsValue == 0xF) { // If all LEDs are currently on
                    greenLedsValue = 0x0; // Turn all LEDs off
                    xil_printf("All green LEDs are OFF\n");
                } else { // If any or all LEDs are currently off
                    greenLedsValue = 0xF; // Turn all LEDs on
                    xil_printf("All green LEDs are ON\n");
                }
                XGpio_DiscreteWrite(&greenLedsInst, 1, greenLedsValue); // Apply the change
                break;


            case 'r': // rotate values
                if(message.action=='L'){
                    // Perform bitwise rotation to the left
                    greenLedsValue = ((greenLedsValue << 1) & 0xF) | ((greenLedsValue >> (4 - 1)) & 0x1);
                } else if(message.action=='R'){
                    // Perform bitwise rotation to the right
                    greenLedsValue = ((greenLedsValue >> 1) & 0xF) | ((greenLedsValue << (4 - 1)) & 0x8);
                }
                break;

        }
        // Write new green LEDs values
        XGpio_DiscreteWrite(&greenLedsInst, 1, greenLedsValue);
    }
}


static void RGBLedTask( void *pvParameters )
{
	// Define a structure to hold the state of the RGB LED.
	typedef struct
	{
		u8 color:3;   // 3-bit color value representing RGB
		u8 frequency; // Blink frequency of the LED
		bool state;   // State of the LED: ON or OFF
	} RGBLedState;

	TickType_t xLastWakeTime, blinkDelayTicks = 0;
	xLastWakeTime = xTaskGetTickCount(); // Initialize with the current tick count

	// Set initial LED state
	RGBLedState RGBState = { .color = 1, .frequency = 0, .state = false };
	Message message = {.type = 'x', .action = 'x'};


	while(1)
	{
		// Handle incoming messages to change LED state
		switch(message.type){

            case 't': // Toggle LED state
                RGBState.state = !RGBState.state;
                break;

            case 'c': // Change LED color
                RGBState.state = true;
/*************************** Enter your code here ****************************/
                switch(message.action) {

				   case '+':
					   RGBState.color++;
					   break;
				   case '-':
					   RGBState.color--;
					   break;
}
                   // Now we update the actual hardware LED with the new color
                   XGpio_DiscreteWrite(&RGBInst, RGB_CHANNEL, RGBState.color);
/*****************************************************************************/

                xil_printf("color: %d\n", RGBState.color);
                break;

            case 'f': // Adjust LED blink frequency
                RGBState.state = true;
                if(message.action=='+'){
                    if(RGBState.frequency < 30){
                        RGBState.frequency++;
                    } else {
                    	RGBState.frequency = 0;
                    }
                } else if(message.action=='-'){
                    if(RGBState.frequency > 0){
                        RGBState.frequency--;
                    } else {
                    	RGBState.frequency = 30;
                    }
                }
                // Calculate the delay in ticks after the frequency has been updated
				if (RGBState.frequency != 0) {
					blinkDelayTicks = pdMS_TO_TICKS(1000 / (2 * RGBState.frequency));
				} else {
					// If the frequency is 0, we set the delay to a default value,
					blinkDelayTicks = portMAX_DELAY; // This will stop the blinking
				}
                xil_printf("frequency: %d\n", RGBState.frequency);
                break;
            default:
                    break;
		}

		// Loop to process LED control until a new message is received
		while (xQueueReceive(xRGBQueue, &message, 0) == pdFALSE){
			if(RGBState.state){
			    if (RGBState.frequency == 0){
			        // If frequency is 0, write the color without blinking.
			        XGpio_DiscreteWrite(&RGBInst, RGB_CHANNEL, RGBState.color);
			    } else { // Blink the LED on and off according to the specified frequency
			        XGpio_DiscreteWrite(&RGBInst, RGB_CHANNEL, RGBState.color);
			        vTaskDelayUntil(&xLastWakeTime, blinkDelayTicks);
			        XGpio_DiscreteWrite(&RGBInst, RGB_CHANNEL, 0);
			        vTaskDelayUntil(&xLastWakeTime, blinkDelayTicks);
			    }
			} else {
			    // Turn off the LED if the state is false
			    XGpio_DiscreteWrite(&RGBInst, RGB_CHANNEL, 0);
			}
		}
	}
}


/****************************************
 *These are the command handler functions
 ****************************************/
static void HandleE7Command(Message* message)
{
    message->type = 't';
    xQueueSend(xRGBQueue, message, 0);
    xil_printf("\n----------E7----------\nRGB LED state changed\n");
    xil_printf("-------Finished-------\n");
}


static void HandleECCommand(Message* message)
{
    unsigned int buttonVal = 0, lastButtonVal = BTN0;

    message->type = 'c';
    xil_printf("\n----------EC----------\n");
    xil_printf("change RGB LED color");
    xil_printf("\n----------------------\n");
    xil_printf("BTN2: Color down\nBTN3: Color up\n");
    xil_printf("BTN0: Finish");
    xil_printf("\n----------------------\n");

    while (1){
        buttonVal = XGpio_DiscreteRead(&btnInst, 1);

        if (buttonVal == BTN3 && lastButtonVal == 0){
            message->action = '+';
        } else if (buttonVal == BTN2 && lastButtonVal == 0){
            message->action = '-';
        } else if (buttonVal == BTN0 && lastButtonVal == 0){
        	xil_printf("-------Finished-------\n");
            break;
        } else {
            message->action = 'x';
        }

        if (message->action == '+' || message->action == '-'){
            xQueueSend(xRGBQueue, message, 0);
        }

        lastButtonVal = buttonVal;
        vTaskDelay(pdMS_TO_TICKS(COMMAND_DELAY));
    }
}


static void HandleEFCommand(Message* message)
{
    unsigned int buttonVal = 0, lastButtonVal = BTN0;

    message->type = 'f';
    xil_printf("\n----------EF----------\n");
	xil_printf("change RGB LED frequency");
	xil_printf("\n----------------------\n");
	xil_printf("BTN2: Decrease frequency \nBTN3: Increase frequency\n");
	xil_printf("BTN0: Finish");
	xil_printf("\n----------------------\n");

    while(1){
        buttonVal = XGpio_DiscreteRead(&btnInst, 1);

        if(buttonVal == BTN3 && lastButtonVal == 0){
            message->action = '+';
        } else if (buttonVal == BTN2 && lastButtonVal == 0){
            message->action = '-';
        } else if (buttonVal == BTN0  && lastButtonVal == 0){
        	xil_printf("-------Finished-------\n");
            break;
        } else {
            message->action = 'x';
        }

        if(message->action == '+' || message->action == '-'){
            xQueueSend(xRGBQueue, message, 0);
        }

        lastButtonVal = buttonVal;
        vTaskDelay(pdMS_TO_TICKS(COMMAND_DELAY));
    }
}


static void HandleA5Command(Message* message)
{
    message->type = 'a';
    xQueueSend(xLedQueue, message, 0);
    xil_printf("\n----------A5----------\ngreen LEDs values set\n");
    xil_printf("-------Finished-------\n");
}


static void HandleD5Command(Message* message)
{
	unsigned int buttonVal = 0, lastButtonVal = BTN0;

    message->type = 's';
    xil_printf("\n----------D5----------\n");
	xil_printf("Shift green LEDs values");
	xil_printf("\n----------------------\n");
	xil_printf("BTN2: Shift left\nBTN3: Shift right\n");
	xil_printf("BTN0: Finish");
	xil_printf("\n----------------------\n");

    while(1){
        buttonVal = XGpio_DiscreteRead(&btnInst, 1);

        if(buttonVal == BTN2 && lastButtonVal == 0){
            message->action = 'L';
        } else if (buttonVal == BTN3 && lastButtonVal == 0){
            message->action = 'R';
        } else if (buttonVal == BTN0  && lastButtonVal == 0){
        	xil_printf("-------Finished-------\n");
            break;
        } else {
            message->action = 'x';
        }

        if(message->action == 'L' || message->action == 'R'){
            xQueueSend(xLedQueue, message, 0);
        }

        lastButtonVal = buttonVal;
        vTaskDelay(pdMS_TO_TICKS(COMMAND_DELAY));
    }
}


static void HandleD4Command(Message* message)
{
	unsigned int buttonVal = 0, lastButtonVal = BTN0;

    message->type = 'r';
    xil_printf("\n----------D4----------\n");
	xil_printf("Rotate green LEDs values");
	xil_printf("\n----------------------\n");
	xil_printf("BTN2: Rotate left\nBTN3: Rotate right\n");
	xil_printf("BTN0: Finish");
	xil_printf("\n----------------------\n");

    while(1){
        buttonVal = XGpio_DiscreteRead(&btnInst, 1);

        if(buttonVal == BTN2 && lastButtonVal == 0){
            message->action = 'L';
        } else if (buttonVal == BTN3 && lastButtonVal == 0){
            message->action = 'R';
        } else if (buttonVal == BTN0  && lastButtonVal == 0){
        	xil_printf("-------Finished-------\n");
            break;
        } else {
            message->action = 'x';
        }

        if(message->action == 'L' || message->action == 'R'){
            xQueueSend(xLedQueue, message, 0);
        }

        lastButtonVal = buttonVal;
        vTaskDelay(pdMS_TO_TICKS(COMMAND_DELAY));
    }
}



static void HandleA3Command(Message* message)
{
	message->type = 'c'; // Let's use 'c' to denote cycling through LEDs
	xQueueSend(xLedQueue, message, 0);
	xil_printf("\nCycling green LEDs...\n");
}


static void HandleUnknownCommand(const char* command)
{
    xil_printf("\n***Command %s is not implemented***\n", command);
}
