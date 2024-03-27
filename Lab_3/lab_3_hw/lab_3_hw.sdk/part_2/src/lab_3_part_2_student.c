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


#define HORIZONTAL_DELAY 30 // Adjust for horizontal sweep speed
#define VERTICAL_DELAY 15   // Adjust for vertical sweep speed
#define SNAKE_MAX_LENGTH 20

#define UP_BUTTON_PIN    0 // Example pin numbers, adjust to your hardware
#define DOWN_BUTTON_PIN  1
#define LEFT_BUTTON_PIN  2
#define RIGHT_BUTTON_PIN 3

XGpio Gpio;
PmodOLED oledDevice;


typedef struct {
    int x;
    int y;
} Point;
Point snake[SNAKE_MAX_LENGTH]; // Snake body represented as points
int snakeLength = 1; // Initial snake length
Point food; // Food position
int direction = 0; // 0=Up, 1=Right, 2=Down, 3=Left

// Function prototypes
void lineSweepTask(void *pvParameters);

volatile int currentMode = 0;


// UART interrupt handler
void UartISR(void *CallBackRef, u32 Event, unsigned int EventData) {
    if (Event == XUARTPS_EVENT_RECV_DATA) {
        u8 data;
        XUartPs_ReadReg(XPAR_XUARTPS_0_BASEADDR, XUARTPS_FIFO_OFFSET, &data, 1);
        if (data == 's') { // Assuming 's' key switches modes
            currentMode = !currentMode; // Toggle mode
        }
    }
}

// Initialize UART
void InitializeUART() {
    XUartPs_Config *Config = XUartPs_LookupConfig(XPAR_XUARTPS_0_DEVICE_ID);
    XUartPs UartPs;
    XUartPs_CfgInitialize(&UartPs, Config, Config->BaseAddress);
    // Set up UART interrupt system here (not fully detailed, depends on your system)
    XUartPs_SetHandler(&UartPs, UartISR, &UartPs);
    XUartPs_SetInterruptMask(&UartPs, XUARTPS_IXR_RXOVR);
}

void initializeGame() {
    // Initialize snake at starting position
    snake[0].x = OledColMax / 2;
    snake[0].y = OledRowMax / 2;
    snakeLength = 1;

    // Place initial food
    // Note: Implement a function to ensure food doesn't spawn on the snake
    placeFoodRandomly();

    // Set initial direction
    direction = 0; // Start moving up

    // Clear display
    OLED_ClearBuffer(&oledDevice);

    // Draw initial snake and food
    drawSnake();
    drawFood();
}

void placeFoodRandomly() {
    // Random placement of food
    // Ensure food is not placed on the snake
    do {
        food.x = rand() % OledColMax;
        food.y = rand() % OledRowMax;
    } while (isFoodOnSnake(food.x, food.y));
}

bool isFoodOnSnake(int x, int y) {
    for (int i = 0; i < snakeLength; i++) {
        if (snake[i].x == x && snake[i].y == y)
            return true;
    }
    return false;
}

void drawSnake() {
    for (int i = 0; i < snakeLength; i++) {
        OLED_SetPixel(&oledDevice, snake[i].x, snake[i].y, 1); // Assume OLED_SetPixel draws a pixel
    }
    OLED_Update(&oledDevice);
}

void drawFood() {
    OLED_SetPixel(&oledDevice, food.x, food.y, 1); // Draw food
    OLED_Update(&oledDevice);
}


void updateDirection() {
    // Pseudocode for reading input and updating direction
    // This will depend on your input mechanism
    // Example:
    if (isButtonPressed(UP_BUTTON) && direction != 2) {
        direction = 0; // Set direction to up
    } else if (isButtonPressed(RIGHT_BUTTON) && direction != 3) {
        direction = 1; // Set direction to right
    } else if (isButtonPressed(DOWN_BUTTON) && direction != 0) {
        direction = 2; // Set direction to down
    } else if (isButtonPressed(LEFT_BUTTON) && direction != 1) {
        direction = 3; // Set direction to left
    }
}

void moveSnake() {
    // Create a temporary point for the new head position
    Point newHead = snake[0];

    // Update the head position based on the current direction
    switch (direction) {
        case 0: // Up
            newHead.y--;
            break;
        case 1: // Right
            newHead.x++;
            break;
        case 2: // Down
            newHead.y++;
            break;
        case 3: // Left
            newHead.x--;
            break;
    }

    // Move the snake body
    for (int i = snakeLength; i > 0; i--) {
        snake[i] = snake[i - 1];
    }

    // Update the head position
    snake[0] = newHead;
}


bool detectCollision() {
    // Check boundary collisions
    if (snake[0].x < 0 || snake[0].x >= OledColMax || snake[0].y < 0 || snake[0].y >= OledRowMax) {
        return true;
    }

    // Check self-collision
    for (int i = 1; i < snakeLength; i++) {
        if (snake[0].x == snake[i].x && snake[0].y == snake[i].y) {
            return true;
        }
    }

    return false;
}


void growSnake() {
    if (snakeLength < SNAKE_MAX_LENGTH) {
        // Increase snake length by adding a segment at the current tail position
        snakeLength++;
    }
}


void updateDisplay() {
    OLED_ClearBuffer(&oledDevice); // Clear the screen before drawing

    drawSnake(); // Draw the snake on the OLED
    drawFood();  // Draw the food on the OLED
}


void displayGameOverScreen() {
    OLED_ClearBuffer(&oledDevice); // Clear the OLED display

    // Assuming OLED_PutString or a similar function is available
    // The positioning might need adjustment based on your OLED's resolution and text size
    OLED_SetCursor(&oledDevice, 1, 1);
    OLED_PutString(&oledDevice, "Game Over!");

    OLED_Update(&oledDevice);
}



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


// Snake game task
void snakeGameTask(void *pvParameters) {
    initializeGame();
    while (!gameOver) {
        if (hasDirectionChanged()) {
            updateDirection();
        }
        moveSnake();
        if (detectCollision()) {
            gameOver = true;
            displayGameOverScreen();
        } else if (detectFoodCollision()) {
            growSnake();
            placeFood();
        }
        updateDisplay();
        vTaskDelay(gameSpeed); // Control game speed
    }
}

bool isButtonPressed(u8 buttonPin) {
    u32 data = XGpio_DiscreteRead(&Gpio, 1); // Assuming buttons are connected to channel 1

    // Shift the read data right by `buttonPin` bits and check if the least significant bit is set
    bool pressed = (data >> buttonPin) & 0x01;
    return pressed;
}

int InitializeGpio() {
    int status;
    status = XGpio_Initialize(&Gpio, XPAR_GPIO_0_DEVICE_ID);
    if (status != XST_SUCCESS) {
        return XST_FAILURE;
    }

    // Assuming buttons are inputs, set them as such.
    // You need to adjust the channel and direction mask based on your hardware configuration.
    XGpio_SetDataDirection(&Gpio, 1, 0xFF);

    return XST_SUCCESS;
}

// Main function or task
void main(void) {
    InitializeUART();
    InitializeGpio();
    OLED_Begin(&oledDevice, XPAR_PMODOLED_0_AXI_LITE_GPIO_BASEADDR, XPAR_PMODOLED_0_AXI_LITE_SPI_BASEADDR, 0, 0);
    while (1) {
        if (currentMode == 0) {
            // Run line sweep task
        } else {
            // Run Snake game task
        }
    }
}