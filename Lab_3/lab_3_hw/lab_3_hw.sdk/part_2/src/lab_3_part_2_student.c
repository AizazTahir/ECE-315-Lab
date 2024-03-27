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

int player1Score = 0;
int player2Score = 0;

// Structs for game objects.
typedef struct { int x, y; } Paddle;
typedef struct { int x, y, vx, vy; } Ball;

// Global variables for game objects and peripherals.
Paddle leftPaddle, rightPaddle;
Ball ball;
XGpio Gpio;
PmodOLED oledDevice;
PmodKYPD myKeypad;

// Function prototypes.
void pongGameTask(void *pvParameters);
void initializePongGame(void);
void updateGame(void);
int InitializeGpio(void);

// Function prototypes
void lineSweepTask(void *pvParameters);
TaskHandle_t xLineSweepTaskHandle = NULL;


bool isPaused = false;
bool isGameOver = false;

XUartPs Uart_PS; // Declare a UART Device Instance
XUartPs_Config *Config;

bool getKeyPressed(PmodKYPD* myKeypad, char* key) {
    u16 keys = KYPD_getKeyStates(myKeypad);

    // Assuming keys state 0x0001 means '1', 0x0002 means '2', etc., adjust accordingly
    if (keys != 0) {
        // Map the raw keys value to a character directly for simplicity
        // This is a simplified approach for debugging purposes.
        switch (keys) {
        case 0x1000: *key = '1'; break;  // Hypothetical raw key state for '1'
            case 0x2000: *key = '2'; break;
            // Map other keys similarly...
            case 0x0400: *key = '6'; break; // Example: Adjust based on your key mapping
            case 0x0800: *key = 'B'; break; // Example: Adjust based on your key mapping
            default: return false; // Key not mapped
        }
        xil_printf("Detected key: %c\n", *key);
        return true;
    }
    return false;
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

    // Draw scores at the top of the screen
    char scoreDisplay[20]; // Buffer for score string
    sprintf(scoreDisplay, "P1: %d P2: %d", player1Score, player2Score);
    OLED_SetCursor(&oledDevice, 0, 0); // You might need to adjust this depending on your display
    OLED_PutString(&oledDevice, scoreDisplay);

    OLED_Update(&oledDevice); // Refresh the display with the new drawing
}


void resetBall(bool resetTowardsPlayer1) {
    ball.x = OledColMax / 2; // Center horizontally
    ball.y = OledRowMax / 2; // Center vertically
    ball.vx = resetTowardsPlayer1 ? -BALL_SPEED_INITIAL : BALL_SPEED_INITIAL; // Direction based on scoring
    ball.vy = (rand() % 2) == 0 ? -BALL_SPEED_INITIAL : BALL_SPEED_INITIAL; // Randomize vertical direction
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

void initializePongGame() {
xil_printf("Initializing game...\n"); // Debug: Confirm game updates
    // Reset scores to zero
    player1Score = 0;
    player2Score = 0;

    // Initialize paddles
    leftPaddle.x = 1; // Position paddle on the left edge
    leftPaddle.y = (OledRowMax / 2) - (PADDLE_HEIGHT / 2); // Center vertically
    rightPaddle.x = OledColMax - 2; // Position paddle on the right edge, minus width
    rightPaddle.y = (OledRowMax / 2) - (PADDLE_HEIGHT / 2); // Center vertically

    // Initialize ball at the center
    ball.x = OledColMax / 2;
    ball.y = OledRowMax / 2;
    ball.vx = BALL_SPEED_INITIAL; // Initial speed in x direction
    ball.vy = BALL_SPEED_INITIAL; // Initial speed in y direction

    // Draw initial game state
    drawGame();
    xil_printf("game initialized...\n"); // Debug: Confirm game updates
}


void handlePaddleMovement(char key, Paddle* leftPaddle, Paddle* rightPaddle) {
xil_printf("Handling key: %c\n", key); // Debug: Identify the key being handled
    switch (key) {
        case '1': if (leftPaddle->y > 0) leftPaddle->y--; break; // Move left paddle up
        case '2': if (leftPaddle->y < OledRowMax - PADDLE_HEIGHT) leftPaddle->y++; break; // Move left paddle down
        case '6': if (rightPaddle->y > 0) rightPaddle->y--; break; // Move right paddle up
        case 'B': if (rightPaddle->y < OledRowMax - PADDLE_HEIGHT) rightPaddle->y++; break; // Move right paddle down
    }
}


void checkPaddleCollisions() {
    // Check collision with left paddle
    if (ball.x <= leftPaddle.x + 1 && ball.y >= leftPaddle.y && ball.y <= leftPaddle.y + PADDLE_HEIGHT) {
        ball.vx = -ball.vx; // Reverse horizontal direction
    }
    // Check collision with right paddle
    if (ball.x >= rightPaddle.x - 1 && ball.y >= rightPaddle.y && ball.y <= rightPaddle.y + PADDLE_HEIGHT) {
        ball.vx = -ball.vx; // Reverse horizontal direction
    }
}

void checkForPause() {
    u8 RxBuffer[1] = {0};
    // Non-blocking read from UART
    int RecievedCount = XUartPs_Recv(&Uart_PS, RxBuffer, 1);
    if (RecievedCount == 1 && RxBuffer[0] == 'p') {
        isPaused = !isPaused; // Toggle pause state
        xil_printf("Game %s\n", isPaused ? "Paused" : "Resumed");
    }
}

void displayGameOver(int winner) {
    OLED_ClearBuffer(&oledDevice);
    char gameOverMessage[30]; // Ensure this is large enough for your message
    sprintf(gameOverMessage, "Game Over. Player %d Wins!", winner);
    OLED_SetCursor(&oledDevice, 0, 0); // Adjust cursor position as needed
    OLED_PutString(&oledDevice, gameOverMessage);
    OLED_Update(&oledDevice);
}

void checkAndRestartGame() {
    char key;
    if (getKeyPressed(&myKeypad, &key)) {
        if (key == '1' && isGameOver) {
            xil_printf("Restarting game...\n");
            isGameOver = false;
            player1Score = 0;
            player2Score = 0;
            initializePongGame(); // Reinitialize game settings
        }
    }
}


void updateGame(void) {
if (isGameOver) {
checkAndRestartGame();
return; // If the game is over, only check for restart
}
checkForPause();
if (isPaused) {
vTaskDelay(pdMS_TO_TICKS(100)); // Adjust this value as needed
return; // Skip game update logic if paused
}
    char key;
    // Check and process keypad input for paddle movement
    if (getKeyPressed(&myKeypad, &key)) {
        handlePaddleMovement(key, &leftPaddle, &rightPaddle);
    }

    // Update ball position
    ball.x += ball.vx;
    ball.y += ball.vy;

    // Check for collisions with walls and reverse ball direction if necessary
    if (ball.y <= 0 || ball.y >= OledRowMax - 1) {
        ball.vy = -ball.vy; // Reverse vertical direction
    }

    // Check for collisions with paddles
    checkPaddleCollisions();

    // Check if a player scored and update the game state accordingly
    if (ball.x <= 0) {
        player2Score++;
        if (player2Score >= 5) {
            xil_printf("Game Over. Player 2 Wins!\n");
            displayGameOver(2);
isGameOver = true; // Indicate game is over
return; // Stop further execution in this cycle
        }
        resetBall(true);
    } else if (ball.x >= OledColMax - 1) {
        player1Score++;
        if (player1Score >= 5) {
            xil_printf("Game Over. Player 1 Wins!\n");
            displayGameOver(1);
isGameOver = true; // Indicate game is over
return; // Stop further execution in this cycle
        }
        resetBall(false);
    }

    // Draw the updated game state
    drawGame();
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
    int status = XGpio_Initialize(&Gpio, XPAR_PS7_GPIO_0_DEVICE_ID);
    if (status != XST_SUCCESS) {
        xil_printf("GPIO Initialization Failed\n");
        return XST_FAILURE;
    }

    XGpio_SetDataDirection(&Gpio, 1, 0xFF); // Set the GPIO direction for input
    return XST_SUCCESS;
}


int main(void) {
// Look up the configuration based on the device ID
Config = XUartPs_LookupConfig(XPAR_PS7_UART_1_DEVICE_ID);
if (NULL == Config) {
   return XST_FAILURE;
}

// Initialize the UART driver
XUartPs_CfgInitialize(&Uart_PS, Config, Config->BaseAddress);

// Check for UART configuration, assuming 115200,8,N,1 settings
XUartPs_SetBaudRate(&Uart_PS, 115200);


    InitializeGpio();
    OLED_Begin(&oledDevice, XPAR_PMODOLED_0_AXI_LITE_GPIO_BASEADDR, XPAR_PMODOLED_0_AXI_LITE_SPI_BASEADDR, 0, 0);

    // Initialize and load the keypad key table
    KYPD_begin(&myKeypad, XPAR_KEYPAD_BASEADDR); // Make sure this matches your actual base address
    char keyTable[16] = {'1', '2', '3', 'A', '4', '5', '6', 'B', '7', '8', '9', 'C', '*', '0', '#', 'D'};
    KYPD_loadKeyTable(&myKeypad, keyTable);

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
