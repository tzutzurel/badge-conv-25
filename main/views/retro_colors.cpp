#include "retro_colors.h"

uint16_t colBackground = 0;
uint16_t colCyan = 0;
uint16_t colYellow = 0;
uint16_t colPink = 0;
uint16_t colMagenta = 0;
uint16_t colWhite = 0;

void initColors(LGFX &display)
{
    if (colBackground == 0)
    {
        colBackground = display.color565(10, 0, 30);
        colCyan = display.color565(0, 255, 255);
        colYellow = display.color565(255, 255, 0);
        colPink = display.color565(255, 20, 220);
        colMagenta = display.color565(255, 64, 255);
        colWhite = display.color565(255, 255, 255);
    }
}
