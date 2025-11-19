#ifndef VIEW_battery_H
#define VIEW_battery_H

#include "view.h"
#include "../battery_monitor.hpp"

class ViewBattery : public View
{
public:
    ViewBattery(BatteryMonitor *monitor) : View(true), m_monitor(monitor) {}
    void render(LGFX &display, LGFX_Sprite &spr) override;

private:
    BatteryMonitor *m_monitor;
};

#endif // VIEW_battery_H
