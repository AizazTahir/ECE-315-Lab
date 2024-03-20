// Include FreeRTOS Libraries
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

// Include xilinx Libraries
#include "xparameters.h"
#include "xgpio.h"
#include "xscugic.h"
#include "xil_exception.h"
#include "xil_printf.h"
#include "xil_cache.h"

// Other miscellaneous libraries
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include "pmodkypd.h"
#include "sleep.h"
#include "PmodOLED.h"
#include "OLEDControllerCustom.h"



#define FRAME_DELAY 50000
Paddle paddle = {58, 31, 10, 1}; // Initial paddle position and size
Ball ball = {64, 16, 1, -1}; // Initial ball position and velocity
XGpio inputGpio;

// Declaring the devices
PmodOLED oledDevice;

// Define Game Elements
typedef struct {
    int x, y;
    int width, height;
} Paddle;

typedef struct {
    int x, y;
    int dx, dy; // Velocity
} Ball;

// Function prototypes
void initializeScreen();
static void oledTask( void *pvParameters );

// To change between PmodOLED and OnBoardOLED is to change Orientation
const u8 orientation = 0x0; // Set up for Normal PmodOLED(false) vs normal
                            // Onboard OLED(true)
const u8 invert = 0x0; // true = whitebackground/black letters
                       // false = black background /white letters

int main()
{
	// Initialize Devices
	initializeScreen();

	xil_printf("Initialization Complete, System Ready!\n");

	xTaskCreate( oledTask					/* The function that implements the task. */
			   , "screen task"				/* Text name for the task, provided to assist debugging only. */
			   , configMINIMAL_STACK_SIZE	/* The stack allocated to the task. */
			   , NULL						/* The task parameter is not used, so set to NULL. */
			   , tskIDLE_PRIORITY			/* The task runs at the idle priority. */
			   , NULL
			   );
	xTaskCreate(gameTask, "Game Task", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);

	vTaskStartScheduler();

   while(1);

   return 0;
}


void initializeScreen()
{
   OLED_Begin(&oledDevice, XPAR_PMODOLED_0_AXI_LITE_GPIO_BASEADDR,
         XPAR_PMODOLED_0_AXI_LITE_SPI_BASEADDR, orientation, invert);

	// Initialize Input GPIO (Adjust the device ID as needed)
    XGpio_Initialize(&inputGpio, XPAR_GPIO_0_DEVICE_ID); // Assuming buttons are connected to GPIO 0
    XGpio_SetDataDirection(&inputGpio, 1, 0xFFFFFFFF); // Configure all pins of channel 1 as inputs
}


static void oledTask( void *pvParameters )
{
	int irow, ib, i, icol;
	// u8 *pat;
	// u32 ticks=0;

	const int horizontalDelay = 10; // Speed of horizontal sweep (lower is faster)
    const int verticalDelay = 5; // Speed of vertical sweep (lower is faster)

	xil_printf("PmodOLED Line Sweep Demo Ready\n\r");

	while (1) {
		//ticks = xTaskGetTickCount();
		//xil_printf("Lab 3 demo started  at: %d\r\n\n", ticks);
		// // Choosing Fill pattern 0
		// pat = OLED_GetStdPattern(0);
		// OLED_SetFillPattern(&oledDevice, pat);
		// // Turn automatic updating off
		// OLED_SetCharUpdate(&oledDevice, 0);

		// // Display text on the screen one pixel row at a time
		// // by sweeping a horizontal line from top to bottom
		// xil_printf("\t1. text and line demo\r\n");
		// for (irow = 0; irow < OledRowMax; irow++) {
		// 	OLED_ClearBuffer(&oledDevice);
		// 	OLED_SetCursor(&oledDevice, 0, 0);
		// 	OLED_PutString(&oledDevice, "ECE 315 Lab");
		// 	OLED_SetCursor(&oledDevice, 0, 1);
		// 	OLED_PutString(&oledDevice, "SPI Demo");
		// 	OLED_SetCursor(&oledDevice, 0, 2);
		// 	OLED_PutString(&oledDevice, "OLED Screen");
		// 	OLED_MoveTo(&oledDevice, 0, irow);
		// 	OLED_FillRect(&oledDevice, 127, 31);
		// 	OLED_MoveTo(&oledDevice, 0, irow);
		// 	OLED_DrawLineTo(&oledDevice, 127, irow);
		// 	OLED_Update(&oledDevice);
		// 	vTaskDelay(5);
		// }

		// vTaskDelay(200);
		// // Blink the display three times.
		// xil_printf("\t2. blinking display");
		// for (i = 0; i < 3; i++) {
		// 	xil_printf(" . ");
		// 	OLED_DisplayOff(&oledDevice);
		// 	vTaskDelay(50);
		// 	OLED_DisplayOn(&oledDevice);
		// 	vTaskDelay(50);
		// }
		// xil_printf("\r\n");
		// vTaskDelay(200);


		// // Now erase the characters from the display
		// // by sweeping horizontal line back up
		// xil_printf("\t3. cleaning display\r\n");
		// for (irow = OledRowMax - 1; irow >= 0; irow--){
		// 	OLED_SetDrawColor(&oledDevice, 1);
		// 	OLED_SetDrawMode(&oledDevice, OledModeSet);
		// 	OLED_MoveTo(&oledDevice, 0, irow);
		// 	OLED_DrawLineTo(&oledDevice, 127, irow);
		// 	OLED_Update(&oledDevice);
		// 	vTaskDelay(10);
		// 	OLED_SetDrawMode(&oledDevice, OledModeXor);
		// 	OLED_MoveTo(&oledDevice, 0, irow);
		// 	OLED_DrawLineTo(&oledDevice, 127, irow);
		// 	OLED_Update(&oledDevice);
		// }
		// vTaskDelay(100);

		// // Draw a rectangle in center of screen
		// xil_printf("\t4. Drawing a rectangle");
		// u8 rectWidth = OledColMax/2;
		// OLED_MoveTo(&oledDevice, OledColMax/2 - rectWidth/2, 0);
		// OLED_RectangleTo(&oledDevice, OledColMax/2 + rectWidth/2, OledRowMax - 1);
		// OLED_Update(&oledDevice);
		// vTaskDelay(300);
		// // Display the 8 different patterns available
		// xil_printf("\t5. showing rectangle fill patterns: ");
		// OLED_SetDrawMode(&oledDevice, OledModeSet);

		// for (ib = 1; ib < 8; ib++) {
		// 	xil_printf("%d ",ib);
		// 	OLED_ClearBuffer(&oledDevice);
		// 	pat = OLED_GetStdPattern(ib);
		// 	OLED_SetFillPattern(&oledDevice, pat);
		// 	OLED_MoveTo(&oledDevice, OledColMax/2 - rectWidth/2, 0);
		// 	OLED_FillRect(&oledDevice, OledColMax/2 + rectWidth/2, OledRowMax - 1);
		// 	OLED_RectangleTo(&oledDevice, OledColMax/2 + rectWidth/2, OledRowMax - 1);
		// 	OLED_Update(&oledDevice);
		// 	vTaskDelay(100);
		// }

		// ticks = xTaskGetTickCount();
		// xil_printf("\n\ndemo finished at: %d\r\n\n", ticks);
		// vTaskDelay(100);

		// Horizontal Line Sweep
        for (irow = 0; irow < OledRowMax; irow++) {
            OLED_ClearBuffer(&oledDevice);
            OLED_MoveTo(&oledDevice, 0, irow);
            OLED_DrawLineTo(&oledDevice, OledColMax - 1, irow); // Draw horizontal line
            OLED_Update(&oledDevice);
            vTaskDelay(horizontalDelay);
        }
        for (irow = OledRowMax - 1; irow >= 0; irow--) {
            OLED_ClearBuffer(&oledDevice);
            OLED_MoveTo(&oledDevice, 0, irow);
            OLED_DrawLineTo(&oledDevice, OledColMax - 1, irow); // Draw horizontal line
            OLED_Update(&oledDevice);
            vTaskDelay(horizontalDelay);
        }

        // Vertical Line Sweep
        for (icol = 0; icol < OledColMax; icol++) {
            OLED_ClearBuffer(&oledDevice);
            OLED_MoveTo(&oledDevice, icol, 0);
            OLED_DrawLineTo(&oledDevice, icol, OledRowMax - 1); // Draw vertical line
            OLED_Update(&oledDevice);
            vTaskDelay(verticalDelay);
        }
        for (icol = OledColMax - 1; icol >= 0; icol--) {
            OLED_ClearBuffer(&oledDevice);
            OLED_MoveTo(&oledDevice, icol, 0);
            OLED_DrawLineTo(&oledDevice, icol, OledRowMax - 1); // Draw vertical line
            OLED_Update(&oledDevice);
            vTaskDelay(verticalDelay);
        }
	}
}

void updatePaddlePosition() {
    u32 buttonState = XGpio_DiscreteRead(&inputGpio, 1); // Read the state of buttons from channel 1

    if (buttonState & 0x01) { // If the first button is pressed, move left
        if (paddle.x > 0) {
            paddle.x -= 1;
        }
    }
    if (buttonState & 0x02) { // If the second button is pressed, move right
        if (paddle.x < (OledColMax - paddle.width)) {
            paddle.x += 1;
        }
    }
}

static void gameTask(void *pvParameters) {
    const TickType_t xDelay = 50 / portTICK_PERIOD_MS; // Game tick rate delay

    // Game Initialization Code Here (if any additional initialization is needed)

    while (1) {
        updatePaddlePosition(); // Update based on input

        // Update Ball Position
        ball.x += ball.dx;
        ball.y += ball.dy;

        // Collision with edges
        if (ball.x <= 0 || ball.x >= OledColMax) ball.dx *= -1;
        if (ball.y <= 0 || ball.y >= OledRowMax) ball.dy *= -1;

        // Collision with paddle
        if (ball.y == paddle.y && ball.x >= paddle.x && ball.x <= (paddle.x + paddle.width)) {
            ball.dy *= -1; // Reflect the ball
        }

        OLED_ClearBuffer(&oledDevice);

        // Draw Paddle
        for (int i = 0; i < paddle.width; ++i) {
            OLED_PutPixel(&oledDevice, paddle.x + i, paddle.y);
        }

        // Draw Ball
        OLED_PutPixel(&oledDevice, ball.x, ball.y);

        OLED_Update(&oledDevice);

        vTaskDelay(xDelay); // Slow down the loop to a reasonable update rate
    }
}
