/*
 * ECE 315 - Winter 2024
 * Lab 2 : Hashing and verifier System
 * Created By: Antonio Andara, November 2023
 * Modified By:
 * Copyrights @ 2024, Department of Electrical and Computer Engineering, University of Alberta
 */


#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "xgpio.h"
#include "xparameters.h"
#include "xuartps.h"
#include "xil_printf.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "sha256.h"

/****************************** MACROS ******************************/
#define UART_DEVICE_ID  XPAR_XUARTPS_0_DEVICE_ID
#define UART_BASEADDR 	XPAR_XUARTPS_0_BASEADDR
#define HASH_LENGTH 	32
#define BUFFER_SIZE 	256
#define QUEUE_LENGTH 	512
#define QUEUE_ITEM_SIZE sizeof(char)
#define LEDS_DEVICE_ID	XPAR_AXI_LEDS_DEVICE_ID
#define LEDS_CHANNEL	1

// Device declarations
XGpio greenLedsInst;

// Instance of the UART Device
XUartPs uart; // Board side of the UART

// The instance of the UART-PS Config
XUartPs_Config *Config;  // Contains settings about the uart
XUartPs UartPs; // Peripheral side of the UART

// variable declaration for Hashing/Verifying selection
typedef enum {NOP = '0', HASH = '1', VERIFY = '2', POW = '3'} Operation;
Operation choice;

typedef struct ProofOfWork
{
    char hash_string[512];
    BYTE hash[32];
    u8 difficulty;
    int nonce;
} ProofOfWork;

u8 greenLedsValue = 0;
u32 xDelay = 100U;

const char * initMessage =
	"A hash function is a mathematical algorithm that takes an input (or \"message\")\n"
	"and returns a fixed-size string of bytes. The output, often called the hash\n"
	"value or hash code, is unique (within reason) to the given input. In this lab,\n"
	"we use the sha256 algorithm to compute the hash of a given string, to verify\n"
	"a signature or to create a proof of work for the given string.\n";

// Queues
QueueHandle_t xInputQueue;
/*************************** Enter your code here ****************************/
// TODO 1: Declare xOutputQueue
QueueHandle_t xOutputQueue;

// TODO 10: declare xPoWInQueue and xPoWOutQueue
QueueHandle_t xPoWInQueue;
QueueHandle_t xPoWOutQueue;

/*****************************************************************************/


void sha256String(const char* input, BYTE output[32])
{
    SHA256_CTX ctx;
    sha256Init(&ctx);
    sha256Update(&ctx, (BYTE*)input, strlen(input));
    sha256Final(&ctx, output);
}


void hashToString(BYTE *hash, char *hashString)
{
/*************************** Enter your code here ****************************/
	// TODO 5: iterate over the BYTE array and convert its value to its
	// hexadecimal representation.
	// Null terminate the string
	for(int i = 0; i < SHA256_BLOCK_SIZE; i++)
    {
        sprintf(hashString + (i * 2), "%02x", hash[i]);
    }
    hashString[SHA256_BLOCK_SIZE * 2] = '\0';
/*****************************************************************************/
}


void printString(const char *str)
{
/*************************** Enter your code here ****************************/
	// TODO 3: write the body of the printString function
	// to send data to the outputQueue
	if (xQueueSend(xOutputQueue, str, portMAX_DELAY) != pdPASS) {
		xil_printf("Error sending data to queue\n");
	}

/*****************************************************************************/
}

void printMenu(void)
{
	xil_printf("\n*******************************************\n");
	xil_printf("Menu:\n1. Hash a string\n2. Verify hash of a given string\n3. Create a Proof Of Work");
	xil_printf("\nEnter your option: ");

}


void receiveInput(void) {
    char input;

    while (1) {
	/*************************** Enter your code here ****************************/
		// TODO 4: receive data from UART device and store it into input
		if (!XUartPs_IsReceiveData(UART_BASEADDR)) {
			vTaskDelay(xDelay);
			continue;
		}
		input = (char) XUartPs_ReadReg(UART_BASEADDR, XUARTPS_FIFO_OFFSET);

	/*****************************************************************************/


        if (input == '\r') {
            input = '\0'; // Null-terminate the input
        }

		if (xQueueSend(xInputQueue, &input, portMAX_DELAY) != pdPASS) {
			xil_printf("Error sending data to queue\n");
			break;
		} else if (input == '\0') {
			break; // Break on null character
		}
	/*************************** Enter your code here ****************************/
		// TODO 4: Use printString instead of xil_printf to print the character
		// to the UART
		printString(&input);  // this is so we echo the input back to the user for visual confirmation

	/*****************************************************************************/
    }
}


u8 leadingZeroCount(BYTE* hash)
{
    u8 zeroCount = 0;

    for (u8 i = 0; i < 32; ++i){
        if (hash[i] == 0){
            zeroCount += 8; // If byte is 0, then all 8 bits are 0.
            continue;
        }
        // If byte is not 0, check individual bits
        for (u8 bit = 7; bit >= 0; --bit){
            if ((hash[i] & (1 << bit)) == 0){
                zeroCount++;
            } else {
                return zeroCount;
            }
        }
    }
    return zeroCount;
}


void vInputTask(void *pvParameters)
{
    char option;
	u32 xPollPeriod = 10U;
    xil_printf(initMessage);
    vTaskDelay(xDelay);

    while (1){
        // display menu to select the option of cipher or decipher
    	printMenu();
        while(1){
        	if (!XUartPs_IsReceiveData(UART_BASEADDR)){
        		vTaskDelay(xPollPeriod);
        		continue;
        	}

        	option = (u8) XUartPs_ReadReg(UART_BASEADDR, XUARTPS_FIFO_OFFSET);
        	XUartPs_ReadReg(UART_BASEADDR, XUARTPS_FIFO_OFFSET);

        	switch (option) {
				case HASH:
					xil_printf("%c",option);
					xil_printf("\n*******************************************\n");
					xil_printf("\nEnter string to calculate hash: ");
					receiveInput();
                    choice = HASH;
					break;

				case VERIFY:
					xil_printf("%c",option);
					xil_printf("\n*******************************************\n");
					xil_printf("\nEnter the precomputed hash: ");
					receiveInput();
                    choice = VERIFY;
                    xil_printf("\nEnter string to verify: ");
					receiveInput();
                    choice = VERIFY;
					break;

                case POW:
                	xil_printf("%c",option);
                	xil_printf("\n*******************************************\n");
                	xil_printf("\nEnter string to calculate proof of work: ");
                    receiveInput();
                    choice = POW;
                    xil_printf("\nEnter difficulty: ");
					receiveInput();
                    choice = POW;
					break;

				default:
                    choice = NOP;
                    xil_printf("\rOption not recognized");
					continue; // Explicitly continue the loop
			}
    		vTaskDelay(xPollPeriod);

			break;
        }
    }
}

void vHashingTask(void *pvParameters)
{
	BYTE hash[32];
	u32 xPollPeriod = 10U;
	char nonceString[14];
    u8 verify_flag = 0, pow_flag = 0;
    ProofOfWork hashingProof = {.hash_string = {0}, .hash = {0}, .difficulty = 0, .nonce = 0};
    ProofOfWork newProof = {.hash_string = {0}, .hash = {0}, .difficulty = 0, .nonce = 0};
	char hashString[65];
	int no_of_characters_read=0;
    static char receivedString[QUEUE_LENGTH];

    static char precomputedHashString[65]; // Add this line to store the precomputed hash string


    while (1)
    {
        if (choice != NOP){
        	// if choice != NOP means that there's data to be received
            // Receive input string from input queue
        	memset(receivedString, 0, sizeof(receivedString));
        	no_of_characters_read=0;
    	    while (xQueueReceive(xInputQueue, &receivedString[no_of_characters_read], 0) == pdPASS){
                if (receivedString[no_of_characters_read] == '\0'){
                	break;
                }
                no_of_characters_read++;
            }

    	    switch  (choice){
				case HASH:
			/*************************** Enter your code here ****************************/
					// TODO 6: Convert hash from BYTE to string using hashToString
					// Calculate SHA-256 hash of received string using shaString
					sha256String(receivedString, hash);
					hashToString(hash, receivedString);

			/*****************************************************************************/
			/*************************** Enter your code here ****************************/
					// TODO 7: print hash using printString
					xil_printf("\n\nSHA256 Hash of \"%s\" is: %s\n", receivedString);
					printString(hashString);


			/*****************************************************************************/
					break;

				case VERIFY:
				    if (verify_flag == 0){
				        // Store the precomputed hash string
				        strncpy(precomputedHashString, receivedString, sizeof(precomputedHashString)-1);
				        precomputedHashString[sizeof(precomputedHashString)-1] = '\0'; // Ensure null-termination
				        verify_flag = 1;
				    } else if (verify_flag == 1){
				        // Calculate SHA-256 hash of the new input string
				        sha256String(receivedString, hash);
				        char newHashString[65]; // To store the new hash as a string
				        hashToString(hash, newHashString); // Convert the new hash to a string

				        // Now compare the new hash string with the precomputed hash string
				        if (strcmp(newHashString, precomputedHashString) == 0){
				            xil_printf("\n\nHashes match!\n");
				            printString("Hashes match!\n");
				        } else {
				            xil_printf("\n\nHashes do not match!\n");
				            printString("Hashes do not match!\n");
				        }

				        verify_flag = 0; // Reset for next operation
				    }
				    // Clear the buffers to be ready for the next input
				    memset(receivedString, 0, sizeof(receivedString));
				    memset(hashString, 0, sizeof(hashString));
				    break;

				case POW:
					if (pow_flag == 0){
						sprintf(newProof.hash_string, receivedString);
						pow_flag = 1;

					} else if (pow_flag == 1){
						for (int i = 0; receivedString[i] != '\0'; i++){
							if(receivedString[i] >= '0' && receivedString[i] <= '9' ){
								newProof.difficulty = newProof.difficulty * 10 + (receivedString[i] - 48); // Converting from ASCII to integer
							} else {
								xil_printf("wrong input value.");
								pow_flag = 0;
								break;



















							}
						}
				/*************************** Enter your code here ****************************/
						// TODO: Send data to the PoW input queue
						if (xQueueSend(xPoWInQueue, &newProof, portMAX_DELAY) != pdPASS) {
							xil_printf("Error sending data to queue\n");
						}
				/*****************************************************************************/
						memset(&newProof, 0, sizeof(newProof)); // Reset newProof
						pow_flag = 0;
					}
					break;
				case NOP:
					break;
			}
            choice = NOP;
        }
		/*************************** Enter your code here ****************************/
			// TODO 12: Receive data from the PoW output queue
			if (xQueueReceive(xPoWOutQueue, &hashingProof, 0) != pdPASS){
				// Delay
				vTaskDelay(xPollPeriod);
				continue;
			}
		/*****************************************************************************/
			snprintf(nonceString, 14, "=%d", hashingProof.nonce);
			strcpy(receivedString, hashingProof.hash_string);
			strcat(receivedString, nonceString);
			sha256String(receivedString, hashingProof.hash);
		/*************************** Enter your code here ****************************/
				// TODO 12: Send data to the PoW input queue
				if (xQueueSend(xPoWInQueue, &hashingProof, portMAX_DELAY) != pdPASS) {
					xil_printf("Error sending data to queue\n");
				}
		/*****************************************************************************/
        vTaskDelay(xPollPeriod);
    }
}
void vPoWTask(void *pvParameters) {
    u8 zeros = 0;
    u32 xPollPeriod = 10U;
    char hashString[65];
    ProofOfWork powProof;

    while(1) {
        memset(&powProof, 0, sizeof(powProof)); // Reset powProof at the start of each loop

        // Receive data from the PoW input queue
        if (xQueueReceive(xPoWInQueue, &powProof, portMAX_DELAY) == pdPASS) {
            // Assume valid data received; proceed with processing

            // Flashing lights logic: light changes every 5 nonces, additional light at every 10 nonces
            if (powProof.nonce % 5 == 0) {
                XGpio_DiscreteWrite(&greenLedsInst, LEDS_CHANNEL, 0x08);
            }
            if (powProof.nonce % 10 == 0) {
                XGpio_DiscreteWrite(&greenLedsInst, LEDS_CHANNEL, 0x01);
            }

            zeros = leadingZeroCount(powProof.hash);

            // Validate difficulty and hash correctness
            if (powProof.difficulty > 0 && !(zeros == 32 * 8)) {
                if (zeros >= powProof.difficulty) {
                    xil_printf("\n*******************************************\n");
                    xil_printf("\nPOW for \"%s\" found\n", powProof.hash_string);
                    hashToString(powProof.hash, hashString);
                    xil_printf("\n\tstring:\t\t%s\n\tnonce:\t%d\n\thash:\t\t%s\n", powProof.hash_string, powProof.nonce, hashString);
                    XGpio_DiscreteWrite(&greenLedsInst, LEDS_CHANNEL, 0); // Turn off LEDs on success
                    powProof.nonce = 0; // Reset nonce for next round
                } else {
                    // Increment nonce and send back for further processing if not successful
                    powProof.nonce += 1;
                    xQueueSend(xPoWOutQueue, &powProof, 0); // Non-blocking send, handle error as needed
                }
            }
        }
        // Delay to allow other tasks to run and to avoid tight loop in case of queue being empty
        vTaskDelay(xPollPeriod);
    }
}




void vOutputTask(void *pvParameters)
{
    u8 write_to_console; // Assuming characters are being sent over UART. Change the data type if otherwise.
    u32 xPollPeriod = 1U;

    while(1){
        /*************************** Enter your code here ****************************/
        // TODO 2: poll xOutputQueue
        if (xQueueReceive(xOutputQueue, &write_to_console, portMAX_DELAY) == pdPASS) {
            // Check if the UART transmitter is full, wait if necessary
            while (XUartPs_IsTransmitFull(UART_BASEADDR)) {
                vTaskDelay(xPollPeriod); // Small delay to yield to other tasks
            }
            /*****************************************************************************/
            // If the transmitter is not full, send the data...
            XUartPs_WriteReg(UART_BASEADDR, XUARTPS_FIFO_OFFSET, write_to_console);
        }
        /*****************************************************************************/
    }
}



int Intialize_UART(u16 DeviceId)
{
	int status;
	// Green leds
		status = XGpio_Initialize(&greenLedsInst, LEDS_DEVICE_ID);
		if(status != XST_SUCCESS){
			xil_printf("GPIO Initialization for green leds failed.\r\n");
			return XST_FAILURE;
		}

		/* Device data direction: 0 for output 1 for input */
		XGpio_SetDataDirection(&greenLedsInst, LEDS_CHANNEL, 0x00);
	/*
	 * Initialize the UART driver so that it's ready to use.
	 * Look up the configuration in the config table, then initialize it.
	 */
	Config = XUartPs_LookupConfig(DeviceId);
	if (NULL == Config) {
		return XST_FAILURE;
	}

	status = XUartPs_CfgInitialize(&uart, Config, Config->BaseAddress);
	if (status != XST_SUCCESS){
		return XST_FAILURE;
	}

	/* Use NORMAL UART mode. */
	XUartPs_SetOperMode(&uart, XUARTPS_OPER_MODE_NORMAL);

	return XST_SUCCESS;
}


int main()
{

	int status;

	xTaskCreate( vInputTask
			   , "Task_Input"
			   , configMINIMAL_STACK_SIZE*5
			   , NULL
			   , tskIDLE_PRIORITY + 1
			   , NULL
			   );

	xTaskCreate( vHashingTask
			   , "Task_Hashing_Engine"
			   , configMINIMAL_STACK_SIZE*5
			   , NULL
			   , tskIDLE_PRIORITY
			   , NULL
			   );

/*************************** Enter your code here ****************************/
	// TODO 12: create the xPoWTask with tskIDLE_PRIORITY priority
	xTaskCreate( vPoWTask
			   , "Task_PoW"
			   , configMINIMAL_STACK_SIZE*5
			   , NULL
			   , tskIDLE_PRIORITY
			   , NULL
			   );

/*****************************************************************************/

	xTaskCreate( vOutputTask
			   , "Task_Display"
			   , configMINIMAL_STACK_SIZE*5
			   , NULL
			   , tskIDLE_PRIORITY + 2
			   , NULL
			   );

	//function call for UART initialization
	status = Intialize_UART(UART_DEVICE_ID);
	if (status != XST_SUCCESS) { xil_printf("UART Polled Mode Example Test Failed\r\n"); }


	//Input queue - shared between vInputTask vHashingTask and vOutputTask
	xInputQueue = xQueueCreate( QUEUE_LENGTH, QUEUE_ITEM_SIZE );
	/* Check the queue was created. */
	configASSERT(xInputQueue);

/*************************** Enter your code here ****************************/
	// TODO 1: Create and assert xOutputQueue
	xOutputQueue = xQueueCreate( QUEUE_LENGTH, QUEUE_ITEM_SIZE );
	/* Check the queue was created. */
	configASSERT(xOutputQueue);
	

/*****************************************************************************/

/*************************** Enter your code here ****************************/
	// TODO 10: Create and assert xPoWInQueue and xPoWOutQueue
	//PoW Input queue - shared between vHashingTask and vPoWTask
	xPoWInQueue = xQueueCreate( 10, sizeof(ProofOfWork) );
	/* Check the queue was created. */
	configASSERT(xPoWInQueue);

    //PoW Output queue - shared between vPoWTask and vHashingTask
	xPoWOutQueue = xQueueCreate( 10, sizeof(ProofOfWork) );
	/* Check the queue was created. */
	configASSERT(xPoWOutQueue);

/*****************************************************************************/


	vTaskStartScheduler();

	while(1);

	return 0;

	// testing my commtis
}
