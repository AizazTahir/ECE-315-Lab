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

#include "xuartps.h"
#include <stdbool.h>


// Game and display parameters.
#define HORIZONTAL_DELAY 30
#define VERTICAL_DELAY 15
#define PADDLE_HEIGHT 5
#define PADDLE_WIDTH 1
#define BALL_SPEED_INITIAL 2
#define BTN0 1
#define BTN1 2

// Structs for game objects.
typedef struct { int x, y; } Paddle;
typedef struct { int x, y, vx, vy; } Ball;

// Global variables for game objects and peripherals.
Paddle leftPaddle, rightPaddle;
Ball ball;
XGpio Gpio;
PmodOLED oledDevice;

// Function prototypes.
void pongGameTask(void *pvParameters);
void initializePongGame(void);
void updateGame(void);
void drawGame(void);
bool isButtonPressed(u8 buttonPin);
int InitializeGpio(void);

// Function prototypes
void lineSweepTask(void *pvParameters);
TaskHandle_t xLineSweepTaskHandle = NULL;



void lineSweepTask(void *pvParameters) {
    int x = 0, y = 0;
    int xDirection = 1, yDirection = 1; // 1 for forward, -1 for backward

    OLED_ClearBuffer(&oledDevice); // Clear the screen initially

    while (1) {
        OLED_ClearBuffer(&oledDevice); // Clear the screen for each frame

        // Draw horizontal line
        OLED_MoveTo(&oledDevice, 0, y);
        OLED_DrawLineTo(&oledDevice, OledColMax - 1, y); // Draw horizontal line

        // Draw vertical line
        OLED_MoveTo(&oledDevice, x, 0);
        OLED_DrawLineTo(&oledDevice, x, OledRowMax - 1); // Draw vertical line

        OLED_Update(&oledDevice); // Update the screen with the new drawing

        // Update the position for horizontal line
        y += yDirection;
        if (y >= OledRowMax - 1 || y <= 0) {
            yDirection *= -1; // Change direction
        }

        // Update the position for vertical line
        x += xDirection;
        if (x >= OledColMax - 1 || x <= 0) {
            xDirection *= -1; // Change direction
        }

        // Delay to control the speed of the sweep
        vTaskDelay(pdMS_TO_TICKS(HORIZONTAL_DELAY)); // Delay for horizontal line speed
        vTaskDelay(pdMS_TO_TICKS(VERTICAL_DELAY));   // Delay for vertical line speed
    }
}

void initializePongGame() {
    // Initialize paddles and ball positions.
    leftPaddle.x = 0;
    leftPaddle.y = OledRowMax / 2 - PADDLE_HEIGHT / 2;
    rightPaddle.x = OledColMax - PADDLE_WIDTH;
    rightPaddle.y = OledRowMax / 2 - PADDLE_HEIGHT / 2;
    ball.x = OledColMax / 2;
    ball.y = OledRowMax / 2;
    ball.vx = BALL_SPEED_INITIAL;
    ball.vy = BALL_SPEED_INITIAL;
    drawGame();
}


void drawGame() {
    OLED_ClearBuffer(&oledDevice);

    // Draw left paddle
    OLED_MoveTo(&oledDevice, leftPaddle.x, leftPaddle.y);
    OLED_DrawLineTo(&oledDevice, leftPaddle.x, leftPaddle.y + PADDLE_HEIGHT - 1);

    // Draw right paddle
    OLED_MoveTo(&oledDevice, rightPaddle.x, rightPaddle.y);
    OLED_DrawLineTo(&oledDevice, rightPaddle.x, rightPaddle.y + PADDLE_HEIGHT - 1);

    // Draw ball
    OLED_MoveTo(&oledDevice, ball.x, ball.y);
    OLED_DrawPixel(&oledDevice);

    OLED_Update(&oledDevice);
}


bool isButtonPressed(u8 buttonPin) {
    u32 data = XGpio_DiscreteRead(&Gpio, 1); // Assuming buttons are connected to channel 1

    // Shift the read data right by `buttonPin` bits and check if the least significant bit is set
    bool pressed = (data >> buttonPin) & 0x01;
    return pressed;
}

void updateGame() {
    // Read the state of all buttons
    u32 buttonState = XGpio_DiscreteRead(&Gpio, 1); // Ensure channel 1 is correct for your setup

    // Adjust for active high or active low buttons as needed
    // Note: Assuming active high (button press results in a logic 1)
    bool btn0Pressed = buttonState & BTN0;
    bool btn1Pressed = buttonState & BTN1;

    // Handle left paddle movement based on input
    if (btn0Pressed && leftPaddle.y > 0) { // BTN0 to move up
        leftPaddle.y--;
    }
    if (btn1Pressed && (leftPaddle.y + PADDLE_HEIGHT) < OledRowMax) { // BTN1 to move down
        leftPaddle.y++;
    }

    // Right paddle movement logic can be added here if you have right paddle controls

    // Update ball position
    ball.x += ball.vx;
    ball.y += ball.vy;

    // Collision detection with walls
    if (ball.y <= 0 || ball.y >= OledRowMax) {
        ball.vy = -ball.vy;
    }

    // Collision detection with paddles
    if ((ball.x == leftPaddle.x + PADDLE_WIDTH && ball.y >= leftPaddle.y && ball.y <= leftPaddle.y + PADDLE_HEIGHT) ||
        (ball.x == rightPaddle.x - PADDLE_WIDTH && ball.y >= rightPaddle.y && ball.y <= rightPaddle.y + PADDLE_HEIGHT)) {
        ball.vx = -ball.vx;
    }

    // Reset the ball if it goes past a paddle
    if (ball.x <= 0 || ball.x >= OledColMax) {
        ball.x = OledColMax / 2;
        ball.y = OledRowMax / 2;
        ball.vx = BALL_SPEED_INITIAL;
        ball.vy = BALL_SPEED_INITIAL;
    }

    drawGame(); // Redraw the game state
}



void pongGameTask(void *pvParameters) {
    initializePongGame(); // Set up the game elements

    while (1) {
        updateGame(); // Update the game state
        vTaskDelay(pdMS_TO_TICKS(100)); // Game speed, adjust as necessary
    }
}


// Ensure your GPIO initialization and button press handling are correct
int InitializeGpio() {
    int status = XGpio_Initialize(&Gpio, XPAR_GPIO_0_DEVICE_ID);
    if (status != XST_SUCCESS) {
        xil_printf("GPIO Initialization Failed\n");
        return XST_FAILURE;
    }

    XGpio_SetDataDirection(&Gpio, 1, 0xFF); // Set the GPIO direction for input
    return XST_SUCCESS;
}


int main(void) {
    InitializeGpio();
    OLED_Begin(&oledDevice, XPAR_PMODOLED_0_AXI_LITE_GPIO_BASEADDR, XPAR_PMODOLED_0_AXI_LITE_SPI_BASEADDR, 0, 0);
    xil_printf("Initialization Complete, System Ready!\n");

    // Create the line sweep task
    //xTaskCreate(lineSweepTask, "Line Sweep Task", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, &xLineSweepTaskHandle);

    // Create the Pong game task
    xTaskCreate(pongGameTask, "Pong Game Task", configMINIMAL_STACK_SIZE * 2, NULL, tskIDLE_PRIORITY + 1, NULL);

    // Task to check mode switch using the dedicated button
    //xTaskCreate(checkModeSwitch, "Check Mode Switch", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, NULL);
    vTaskStartScheduler(); // Start the FreeRTOS scheduler

    for (;;); // In case the scheduler ends, which should not happen
    return 0;
}