#pragma once

#include "lgfx_custom.h"
#include "state.h"

#include <vector>
#include <memory>
#include "views/view.h"

class DisplayManager
{
public:
    DisplayManager(LGFX &lcd, AppState &state);
    void init();
    void displayLoop();
    void addView(std::unique_ptr<View> view);

private:
    LGFX &m_lcd;
    AppState &m_state;
    std::vector<std::unique_ptr<View>> m_views;
    size_t m_currentView = 0;
    bool m_wasTouched = false;
    LGFX_Sprite m_sprite;
    void nextView();
    bool shouldRenderFrame();
};
