#include "view_badge.h" // Pour initColors et couleurs badge
#include "view_settings.h"
#include "retro_colors.h"
#include "button.h"
#include <esp_log.h>

ViewSettings::ViewSettings(LGFX &lcd, DisplayManager &displayManager)
    : View(true), m_lcd(lcd), m_displayManager(displayManager)
{
    // Init badge colors if needed
    initColors(lcd);

    // Slider brightness
    m_sliderX = 20;
    m_sliderY = 80;
    m_sliderW = 180;
    m_sliderH = 24;

    // Stepper awakeTime
    this->m_stepperAwakeX = 20;
    this->m_stepperAwakeY = 130;
    this->m_stepperAwakeW = 180;
    this->m_stepperAwakeH = 32;
    this->m_stepperBtnW = 32;
    this->m_stepperBtnH = 32;

    // Cross button (top right)
    int crossX = (int)(lcd.width()) - 32;
    m_crossButton = {crossX, 8, 24, 24, "X"};
}

// --- Slider générique ---
void ViewSettings::drawSlider(LGFX_Sprite &spr, int x, int y, int w, int h, int colorFill, int colorGlow, int colorLabel, int colorValue, float value, float min, float max, const char *valueFormat, const char *label, int valueInt)
{
    // Background
    spr.fillRoundRect(x, y, w, h, 6, colBackground);
    // Glow
    spr.drawRoundRect(x - 1, y - 1, w + 2, h + 2, 8, colorGlow);
    // Fill
    int fillW = (value - min) * (w - 8) / (max - min);
    spr.fillRoundRect(x + 4, y + 4, fillW, h - 8, 4, colorFill);
    // Knob
    int knobX = x + 4 + fillW;
    spr.fillCircle(knobX, y + h / 2, h / 2 - 2, colYellow);
    // Value (bigger font)
    spr.setTextColor(colorValue);
    spr.setFont(&fonts::Font0);
    spr.setTextDatum(textdatum_t::middle_left);
    char buf[16];
    if (valueInt >= 0)
        snprintf(buf, sizeof(buf), valueFormat, valueInt);
    else
        snprintf(buf, sizeof(buf), valueFormat, value);
    spr.drawString(buf, x + w + 10, y + h / 2 - 2);
    // Label (bigger font)
    spr.setTextColor(colorLabel);
    spr.setFont(&fonts::Font0);
    spr.setTextDatum(textdatum_t::bottom_left);
    spr.drawString(label, x, y - 6);
}

void ViewSettings::render(LGFX &display, LGFX_Sprite &spr)
{
    spr.fillScreen(colBackground);
    // Title (no accent)
    spr.setTextColor(colYellow);
    spr.setFont(&fonts::Font2);
    spr.setTextDatum(textdatum_t::top_center);
    spr.drawString("Reglages", display.width() / 2, 18);

    // Slider luminosité
    drawSlider(spr, m_sliderX, m_sliderY, m_sliderW, m_sliderH, colCyan, colCyan, colPink, colCyan, (float)Config::activeBrightness, 0.0f, 100.0f, "%d%%", "Luminosite", (int)Config::activeBrightness);

    // Stepper veille auto
    int stepperX = this->m_stepperAwakeX;
    int stepperY = this->m_stepperAwakeY;
    int btnW = this->m_stepperBtnW;
    int btnH = this->m_stepperBtnH;
    int valueW = 60;
    int valueH = btnH;
    // Label
    spr.setTextColor(colPink);
    spr.setFont(&fonts::Font0);
    spr.setTextDatum(textdatum_t::bottom_left);
    spr.drawString("Veille auto.", stepperX, stepperY - 6);
    // Bouton -
    spr.fillRoundRect(stepperX, stepperY, btnW, btnH, 6, colMagenta);
    spr.setTextColor(colYellow);
    spr.setTextDatum(textdatum_t::middle_center);
    spr.drawString("-", stepperX + btnW / 2, stepperY + btnH / 2);
    // Valeur
    spr.fillRoundRect(stepperX + btnW + 8, stepperY, valueW, valueH, 6, colBackground);
    spr.setTextColor(colMagenta);
    spr.setFont(&fonts::Font0);
    char buf[16];
    snprintf(buf, sizeof(buf), "%d min", (int)Config::awakeTime);
    spr.drawString(buf, stepperX + btnW + 8 + valueW / 2, stepperY + valueH / 2);
    // Bouton +
    spr.fillRoundRect(stepperX + btnW + 8 + valueW + 8, stepperY, btnW, btnH, 6, colMagenta);
    spr.setTextColor(colYellow);
    spr.drawString("+", stepperX + btnW + 8 + valueW + 8 + btnW / 2, stepperY + btnH / 2);

    // Checkbox rotation écran
    int cbx = m_checkboxRotX;
    int cby = m_checkboxRotY;
    int cbsize = m_checkboxRotSize;
    spr.drawRect(cbx, cby, cbsize, cbsize, colCyan);
    if (Config::display_rotated)
    {
        // Draw checkmark
        spr.drawLine(cbx + 4, cby + cbsize / 2, cbx + cbsize / 2, cby + cbsize - 4, colCyan);
        spr.drawLine(cbx + cbsize / 2, cby + cbsize - 4, cbx + cbsize - 4, cby + 4, colCyan);
    }
    spr.setTextColor(colCyan);
    spr.setFont(&fonts::Font0);
    spr.setTextDatum(textdatum_t::middle_left);
    spr.drawString("Tourner l'ecran 180°", cbx + cbsize + 10, cby + cbsize / 2);

    // Draw cross button (rectangle + X)
    spr.drawRect(m_crossButton.x, m_crossButton.y, m_crossButton.w, m_crossButton.h, colWhite);
    int cx = m_crossButton.x + m_crossButton.w / 2;
    int cy = m_crossButton.y + m_crossButton.h / 2;
    int cross_size = 12;
    spr.drawLine(cx - cross_size / 2, cy - cross_size / 2, cx + cross_size / 2, cy + cross_size / 2, colWhite);
    spr.drawLine(cx + cross_size / 2, cy - cross_size / 2, cx - cross_size / 2, cy + cross_size / 2, colWhite);
}

void ViewSettings::updateBrightnessFromTouch(int x)
{
    int relX = x - (m_sliderX + 4);
    int range = m_sliderW - 8;
    int value = relX * 100 / range;
    if (value < 1)
        value = 1;
    if (value > 100)
        value = 100;
    m_displayManager.updateBrightness(value);
}

void ViewSettings::updateAwakeTimeStepper(bool increment)
{
    int value = (int)Config::awakeTime;
    if (increment)
        value += 5;
    else
        value -= 5;
    if (value < 5)
        value = 5;
    if (value > 60)
        value = 60;
    this->m_displayManager.updateAwakeTime(value);
}

bool ViewSettings::handleTouch(int x, int y)
{
    // Cross button
    if (isButtonPressed(m_crossButton, x, y))
        return false;

    // Checkbox rotation
    int cbx = m_checkboxRotX;
    int cby = m_checkboxRotY;
    int cbsize = m_checkboxRotSize;
    if (isRectanglePressed(cbx, cby, cbsize, cbsize, x, y))
    {
        toggleRotation();
        return true;
    }

    // Slider luminosité
    if (isRectanglePressed(m_sliderX, m_sliderY, m_sliderW, m_sliderH, x, y))
    {
        updateBrightnessFromTouch(x);
        return true;
    }
    // Stepper veille auto
    int stepperX = this->m_stepperAwakeX;
    int stepperY = this->m_stepperAwakeY;
    int btnW = this->m_stepperBtnW;
    int btnH = this->m_stepperBtnH;
    int valueW = 60;
    // Bouton -
    if (isRectanglePressed(stepperX, stepperY, btnW, btnH, x, y))
    {
        this->updateAwakeTimeStepper(false);
        return true;
    }
    // Bouton +
    int plusX = stepperX + btnW + 8 + valueW + 8;
    if (isRectanglePressed(plusX, stepperY, btnW, btnH, x, y))
    {
        this->updateAwakeTimeStepper(true);
        return true;
    }
    return true;
}

void ViewSettings::toggleRotation()
{
    // Inverser la rotation dans Config
    Config::setDisplayRotated(!Config::display_rotated);
    ESP_LOGI("ViewSettings", "Display rotation set to %d", Config::display_rotated);
    // Appliquer la rotation via DisplayManager
    m_displayManager.applyRotationFromConfig();
}
