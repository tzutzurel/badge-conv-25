#ifndef CONFIG_H
#include <cstdint>
#define CONFIG_H

namespace Config
{
    extern float awakeTime;          // Durée avant veille (minutes)
    extern uint8_t sleepBrightness;  // Luminosité en veille (%)
    extern uint8_t activeBrightness; // Luminosité active (%)
    extern bool display_rotated;     // Rotation écran 180°

    void setActiveBrightness(uint8_t value);
    void setSleepBrightness(uint8_t value);
    void setDisplayRotated(bool value);
    void setAwakeTime(float value);

    void saveToNVS();
    void loadFromNVS();
    void initNVS();
}

#endif // CONFIG_H
