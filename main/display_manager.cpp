#include "display_manager.h"
#include "esp_log.h"
#include <cmath>
#include <cstdio>

DisplayManager::DisplayManager(LGFX &lcd, AppState &state)
    : m_lcd(lcd), m_state(state), m_sprite(&lcd) {}

void DisplayManager::init()
{
    m_state.screenW = m_lcd.width();
    m_state.screenH = m_lcd.height();
    m_sprite.setColorDepth(16);
    m_sprite.createSprite(m_state.screenW, m_state.screenH);
}

void DisplayManager::displayLoop()
{
    if (!shouldRenderFrame())
        return;

    int pixel_x = -1, pixel_y = -1;
    bool touchDetected = m_lcd.getTouch(&pixel_x, &pixel_y);
    if (touchDetected)
    {
        if (!m_wasTouched)
        {
            nextView();
            m_wasTouched = true;
        }
    }
    else
    {
        m_wasTouched = false;
    }

    if (!m_views.empty())
    {
        m_views[m_currentView]->render(m_lcd, m_sprite);
    }
    m_sprite.pushSprite(0, 0);
    vTaskDelay(pdMS_TO_TICKS(16));
}

void DisplayManager::addView(std::unique_ptr<View> view)
{
    m_views.push_back(std::move(view));
}

void DisplayManager::nextView()
{
    if (m_views.empty())
        return;
    m_currentView = (m_currentView + 1) % m_views.size();
}

bool DisplayManager::shouldRenderFrame()
{
    static unsigned long lastFrame = 0;
    unsigned long now = lgfx::v1::millis();
    if (now - lastFrame < 16)
    {
        vTaskDelay(1 / portTICK_PERIOD_MS);
        return false;
    }
    lastFrame = now;
    m_state.t = lgfx::v1::millis() * 0.001f;
    return true;
}
