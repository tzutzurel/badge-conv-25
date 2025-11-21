#ifndef VIEW_PLASMA_H
#define VIEW_PLASMA_H

#include "view.h"
#include "../state.h"
#include "../lgfx_custom.h"

class ViewPlasma : public View
{
public:
    ViewPlasma(AppState &state, LGFX &lcd);
    void render(LGFX &display, LGFX_Sprite &spr) override;
    bool handleTouch(int x, int y) override;

    void updateAnimation(float dt);
    void renderPlasma(LGFX_Sprite &spr);
    void renderName(LGFX_Sprite &spr);

private:
    AppState &m_state;
    LGFX &m_lcd;
    float tempo;
};

#endif // VIEW_PLASMA_H
