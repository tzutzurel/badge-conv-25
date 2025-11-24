#ifndef RETRO_COLORS_H
#define RETRO_COLORS_H

#include "../lgfx_custom.h"
#include <cstdint>

extern uint16_t colBackground;
extern uint16_t colCyan;
extern uint16_t colYellow;
extern uint16_t colPink;
extern uint16_t colMagenta;
extern uint16_t colWhite;
extern uint16_t colRed;

void initColors(LGFX &display);

#endif // RETRO_COLORS_H
