
#include "view_badge.h"
#include "user_info.h"
#include <string>
#include "state.h"
#include <cmath>

// Couleurs badge (similaire à l'ancien display_manager)
static uint16_t colBackground = 0;
static uint16_t colNeonViolet = 0;
static uint16_t colCyan = 0;
static uint16_t colYellow = 0;
static uint16_t colHeart = 0;

// Utilitaire pour initialiser les couleurs une seule fois
static void initColors(LGFX &display)
{
    if (colBackground == 0)
    {
        colBackground = display.color565(8, 6, 20);
        colNeonViolet = display.color565(199, 120, 255);
        colCyan = display.color565(0, 224, 255);
        colYellow = display.color565(255, 195, 0);
        colHeart = display.color565(255, 40, 180);
    }
}

ViewBadge::ViewBadge(AppState &state, LGFX &lcd)
    : m_state(state), m_lcd(lcd)
{
}

void ViewBadge::updateNeonFlicker()
{
    float flicker_fast = 0.75f + 0.35f * ((float)rand() / RAND_MAX) * ((float)rand() / RAND_MAX);
    if (m_state.neon_dim_frames <= 0 && ((float)rand() / RAND_MAX) < 0.016f)
    {
        m_state.neon_dim_frames = 6 + rand() % 10;
        float base_dim = 0.65f + 0.25f * ((float)rand() / RAND_MAX) * ((float)rand() / RAND_MAX);
        m_state.neon_dim_target = base_dim;
    }
    if (m_state.neon_dim_frames > 0)
    {
        float frame_dim = m_state.neon_dim_target + 0.08f * (((float)rand() / RAND_MAX) - 0.5f);
        flicker_fast *= frame_dim;
        m_state.neon_dim_frames--;
    }
    float smoothing = 0.72f;
    m_state.neon_flicker_smooth = smoothing * m_state.neon_flicker_smooth + (1.0f - smoothing) * flicker_fast;
    m_state.neon_flicker = m_state.neon_flicker_smooth;
}

void ViewBadge::updateScanlineOffset()
{
    m_state.scanline_offset = (m_state.scanline_offset + 1) % 10;
}

void ViewBadge::updateBrightness()
{
    int brightness = (int)(m_state.neon_flicker * 255.0f);
    brightness = brightness < 32 ? 32 : (brightness > 255 ? 255 : brightness);
    m_lcd.setBrightness(brightness);
}

void ViewBadge::renderBackground(LGFX_Sprite &spr)
{
    spr.fillSprite(colBackground);
    renderScanlines(spr);
}

void ViewBadge::renderHeader(LGFX_Sprite &spr)
{
    spr.setTextDatum(TC_DATUM);
    spr.setTextFont(1);
    spr.setTextSize(2);
    spr.setTextColor(colCyan);
    spr.drawString("TAP TO LIKE", m_state.screenW / 2, 20);
}

void ViewBadge::renderHeart(LGFX_Sprite &spr)
{
    spr.fillCircle(m_state.screenW / 2, 54, 16, colHeart);
}

void ViewBadge::renderName(LGFX_Sprite &spr)
{
    renderNeonFullName(spr, user_info.prenom.c_str(), user_info.nom.c_str());
}

void ViewBadge::renderSeparator(LGFX_Sprite &spr)
{
    spr.fillRect(36, 180, m_state.screenW - 80, 3, colCyan);
}

void ViewBadge::renderTeam(LGFX_Sprite &spr)
{
    spr.setTextDatum(TC_DATUM);
    spr.setTextFont(1);
    spr.setTextSize(2);
    spr.setTextColor(colCyan);
    spr.drawString(user_info.equipe.c_str(), m_state.screenW / 2, 200);
}

void ViewBadge::renderLocationAndRole(LGFX_Sprite &spr)
{
    spr.setTextDatum(TC_DATUM);
    spr.setTextFont(1);
    spr.setTextSize(2);
    spr.setTextColor(colYellow);
    spr.drawString(user_info.ville.c_str(), m_state.screenW / 2, 230);
    spr.drawString(user_info.poste.c_str(), m_state.screenW / 2, 260);
}

void ViewBadge::renderScanlines(LGFX_Sprite &spr)
{
    for (int y = 0; y < m_state.screenH; y += 10)
    {
        int animY = y + m_state.scanline_offset;
        if (animY < m_state.screenH)
        {
            uint16_t col = m_lcd.color565(12, 8, 30);
            spr.drawFastHLine(0, animY, m_state.screenW, col);
        }
    }
}

void ViewBadge::renderNeonText(LGFX_Sprite &spr, const char *txt1, const char *txt2, int x, int y1, int y2)
{
    uint8_t baseR = 255, baseG = 60, baseB = 200;
    float flicker = m_state.neon_flicker;
    float organic = fmaxf(flicker, 0.32f);
    spr.setTextDatum(MC_DATUM);
    spr.setTextFont(4);
    int layers = (int)(m_state.neon_flicker * 12.0f);
    for (int i = layers; i > 0; i--)
    {
        float fade = powf(0.8f, i) * organic;
        float alpha = fade * 0.5f * flicker;
        uint8_t r = (uint8_t)(baseR * alpha + 8 * (1.0f - alpha));
        uint8_t g = (uint8_t)(baseG * alpha + 6 * (1.0f - alpha));
        uint8_t b = (uint8_t)(baseB * alpha + 20 * (1.0f - alpha));
        uint16_t color = m_lcd.color565(r, g, b);
        spr.setTextColor(color);
        spr.setTextSize(1.5f + 0.02f * i);
        spr.drawString(txt1, x + i, y1);
        spr.drawString(txt1, x - i, y1);
        spr.drawString(txt1, x, y1 + i);
        spr.drawString(txt1, x, y1 - i);
        spr.drawString(txt2, x + i, y2);
        spr.drawString(txt2, x - i, y2);
        spr.drawString(txt2, x, y2 + i);
        spr.drawString(txt2, x, y2 - i);
    }
    spr.setTextSize(1.5f);
    uint16_t brightColor = m_lcd.color565(
        baseR * (0.3f + 0.6f * organic),
        baseG * (0.3f + 0.6f * organic),
        baseB * (0.3f + 0.6f * organic));
    spr.setTextColor(brightColor);
    spr.drawString(txt1, x, y1);
    spr.drawString(txt2, x, y2);
}

void ViewBadge::renderNeonFullName(LGFX_Sprite &spr, const char *name1, const char *name2)
{
    int x = m_state.screenW / 2;
    int y1 = 110;
    int y2 = 145;
    renderNeonText(spr, name1, name2, x, y1, y2);
}

void ViewBadge::renderLikeCounter(LGFX_Sprite &spr)
{
    spr.setTextDatum(TR_DATUM);
    spr.setTextFont(1);
    spr.setTextSize(2);
    spr.setTextColor(colHeart);
    char buf[16];
    snprintf(buf, sizeof(buf), "\xe2\x99\xa5 %d", m_state.likes);
    spr.drawString(buf, m_state.screenW - 8, 8);
}

// Affichage principal du badge
void ViewBadge::render(LGFX &display, LGFX_Sprite &spr)
{
    initColors(display);
    // Update AppState screen size
    m_state.screenW = spr.width();
    m_state.screenH = spr.height();

    // Mise à jour des variables d'animation
    updateNeonFlicker();
    updateScanlineOffset();
    updateBrightness();

    // Rendu
    renderBackground(spr);
    renderHeader(spr);
    renderHeart(spr);
    renderName(spr);
    renderSeparator(spr);
    renderTeam(spr);
    renderLocationAndRole(spr);
    renderLikeCounter(spr);
}
