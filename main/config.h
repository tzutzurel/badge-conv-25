#ifndef CONFIG_H
#define CONFIG_H

// Paramètres de veille généraux
namespace Config
{
    inline float awakeTime = 5.0f;        // Durée avant veille (minutes)
    inline uint8_t sleepBrightness = 10;  // Luminosité en veille (%)
    inline uint8_t activeBrightness = 80; // Luminosité active (%)

    inline void setActiveBrightness(uint8_t value) { activeBrightness = value; }
    inline void setSleepBrightness(uint8_t value) { sleepBrightness = value; }
}

#endif // CONFIG_H
