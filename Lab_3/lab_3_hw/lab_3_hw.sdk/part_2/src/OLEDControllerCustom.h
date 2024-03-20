#ifndef OLEDGRAPHICS_H
#define OLEDGRAPHICS_H

#include "PmodOLED.h"

// Function prototypes
void OLED_DrawLineTo(PmodOLED *InstancePtr, int xco, int yco);
void OLED_RectangleTo(PmodOLED *InstancePtr, int xco, int yco);
void OLED_PutPixel(PmodOLED *InstancePtr, int x, int y);

#endif // OLEDGRAPHICS_H
