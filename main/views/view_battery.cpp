#include "view_battery.h"
#include "retro_colors.h"
#include <string>

void ViewBattery::render(LGFX &display, LGFX_Sprite &spr)
{
    initColors(display);
    spr.fillRect(0, 0, spr.width(), spr.height(), colBackground);

    // Titre rétro-futuriste
    spr.setTextDatum(TC_DATUM);
    spr.setTextFont(2);
    spr.setTextSize(2);
    spr.setTextColor(colMagenta);
    spr.drawString("BATTERIE", spr.width() / 2 + 2, 10);
    spr.setTextColor(colCyan);
    spr.drawString("BATTERIE", spr.width() / 2, 8);

    // Lecture des données batterie
    float vbat = m_monitor->readBatteryVoltage();
    float percent = m_monitor->getPercentage(vbat);
    float soh = m_monitor->estimateSOH(vbat);
    float hours = m_monitor->estimateHoursLeft(percent, 80.0f); // 80mA typique

    // Affichage jauge
    int bar_x = 30, bar_y = 60, bar_w = spr.width() - 60, bar_h = 28;
    spr.drawRect(bar_x, bar_y, bar_w, bar_h, colCyan);
    int fill_w = (int)(bar_w * (percent / 100.0f));
    spr.fillRect(bar_x + 2, bar_y + 2, fill_w - 4, bar_h - 4, colYellow);

    // Texte pourcentage
    spr.setTextDatum(CC_DATUM);
    spr.setTextFont(4);
    spr.setTextSize(1);
    spr.setTextColor(colWhite);
    char buf[16];
    snprintf(buf, sizeof(buf), "%2.0f%%", percent);
    spr.drawString(buf, spr.width() / 2, bar_y + bar_h / 2);

    // Infos voltage et SOH
    spr.setTextFont(2);
    spr.setTextSize(1);
    spr.setTextColor(colPink);
    snprintf(buf, sizeof(buf), "%.2fV", vbat);
    spr.drawString(buf, spr.width() / 2, bar_y + bar_h + 18);
    snprintf(buf, sizeof(buf), "SOH: %.0f%%", soh);
    spr.drawString(buf, spr.width() / 2, bar_y + bar_h + 38);

    // Estimation autonomie
    spr.setTextColor(colMagenta);
    snprintf(buf, sizeof(buf), "~%.1fh restantes", hours);
    spr.drawString(buf, spr.width() / 2, bar_y + bar_h + 58);
}
