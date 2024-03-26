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

// Define Game Elements
typedef struct {
    int x, y;
    int width, height;
} Paddle;

typedef struct {
    int x, y;
    int dx, dy; // Velocity
} Ball;

Paddle paddle = {58, 31, 10, 1}; // Initial paddle position and size
Ball ball = {64, 16, 1, -1}; // Initial ball position and velocity
XGpio inputGpio;

// Declaring the devices
PmodOLED oledDevice;
// Declare kEYPAD
PmodKYPD myKeypad;

volatile int gamePaused = 0; // 0 = running, 1 = paused
volatile int gameRestart = 0; // Flag to indicate game needs to be restarted

// Function prototypes
void initializeScreen();
static void oledTask( void *pvParameters );
static void gameTask(void *pvParameters);
static void buttonInputTask(void *pvParameters);

// To change between PmodOLED and OnBoardOLED is to change Orientation
const u8 orientation = 0x0; // Set up for Normal PmodOLED(false) vs normal
                            // Onboard OLED(true)
const u8 invert = 0x0; // true = whitebackground/black letters
                       // false = black background /white letters

int main() {
    // Initialize Devices
    initializeScreen();

    // Initialize Keypad with the correct base address
    KYPD_begin(&myKeypad, XPAR_KEYPAD_BASEADDR);

    xil_printf("Initialization Complete, System Ready!\n");

    // Create OLED display task
    xTaskCreate(oledTask, "screen task", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);

    // Create Game logic task
    xTaskCreate(gameTask, "Game Task", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);

    // Create Button input task
    xTaskCreate(buttonInputTask, "Button Input Task", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);

    // Start the scheduler
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

static void buttonInputTask(void *pvParameters) {
    const TickType_t xDelay = 100 / portTICK_PERIOD_MS; // 100ms delay for debouncing
    u32 lastButtonState = 0;

    while (1) {
        u32 buttonState = XGpio_DiscreteRead(&inputGpio, 1); // Assuming buttons are on channel 1

        // Check for button 1 press (assuming it moves the paddle left)
        if ((buttonState & 0x01) && !(lastButtonState & 0x01)) {
            // Move paddle left
            if (paddle.x > 0) {
                paddle.x -= 1;
            }
        }

        // Check for button 2 press (assuming it moves the paddle right)
        if ((buttonState & 0x02) && !(lastButtonState & 0x02)) {
            // Move paddle right
            if (paddle.x < (OledColMax - paddle.width)) {
                paddle.x += 1;
            }
        }

        lastButtonState = buttonState; // Update the last button state

        vTaskDelay(xDelay); // Delay for debouncing
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

void resetGameState() {
    paddle.x = 58; paddle.y = 31;
    ball.x = 64; ball.y = 16;
    ball.dx = 1; ball.dy = -1;
}


static void gameTask(void *pvParameters) {
    const TickType_t xDelay = 50 / portTICK_PERIOD_MS;
    u16 keys;
    u8 keyChar;
    XStatus status;

    while (1) {
        keys = KYPD_getKeyStates(&myKeypad); // Get current key states
        status = KYPD_getKeyPressed(&myKeypad, keys, &keyChar); // Check if a single key is pressed

        if (status == KYPD_SINGLE_KEY) {
            switch(keyChar) {
                case 'A': // Toggle pause state
                    gamePaused = !gamePaused;
                    break;
                case 'B': // Set restart flag
                    gameRestart = 1;
                    break;
                // Additional cases as needed
            }
        }

        if (gameRestart) {
            resetGameState(); // Reset the game state
            gameRestart = 0; // Clear the restart flag
            gamePaused = 0; // Unpause the game
        }

        if (!gamePaused) {
            updatePaddlePosition(); // Update paddle based on input

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
        }

        // Always clear the buffer and redraw each loop iteration
        OLED_ClearBuffer(&oledDevice);

        // Draw Paddle
        for (int i = 0; i < paddle.width; ++i) {
            OLED_PutPixel(&oledDevice, paddle.x + i, paddle.y);
        }

        // Draw Ball
        OLED_PutPixel(&oledDevice, ball.x, ball.y);

        // Update the display
        OLED_Update(&oledDevice);

        vTaskDelay(xDelay); // Control the game update rate
    }
}
