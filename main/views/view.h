#ifndef VIEW_H
#define VIEW_H

#include "../lgfx_custom.h"

class View
{
public:
    virtual ~View() = default;
    virtual void render(LGFX &display, LGFX_Sprite &spr) = 0;
};

#endif // VIEW_H
