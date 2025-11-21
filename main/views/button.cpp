#include "button.h"

bool isButtonPressed(const Button &btn, int touch_x, int touch_y, int hysteresis)
{
    return (touch_x >= btn.x - hysteresis && touch_x <= btn.x + btn.w + hysteresis && touch_y >= btn.y - hysteresis && touch_y <= btn.y + btn.h + hysteresis);
}

bool isRectanglePressed(int rect_x, int rect_y, int rect_w, int rect_h, int touch_x, int touch_y, int hysteresis)
{
    return (touch_x >= rect_x - hysteresis && touch_x <= rect_x + rect_w + hysteresis && touch_y >= rect_y - hysteresis && touch_y <= rect_y + rect_h + hysteresis);
}
