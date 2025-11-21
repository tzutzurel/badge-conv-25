#pragma once

struct Button
{
    int x, y, w, h;
    const char *text;
};

bool isButtonPressed(const Button &btn, int touch_x, int touch_y, int hysteresis = 8);
bool isRectanglePressed(int rect_x, int rect_y, int rect_w, int rect_h, int touch_x, int touch_y, int hysteresis = 8);