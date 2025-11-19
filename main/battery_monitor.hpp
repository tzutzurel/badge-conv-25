#pragma once
#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"
#include <vector>
#include <algorithm>
#include <cmath>

class BatteryMonitor
{
public:
    BatteryMonitor(
        adc_unit_t unit,
        adc_channel_t channel,
        float dividerGain,
        int numCells,
        float minCellV,
        float maxCellV) : unit(unit),
                          channel(channel),
                          dividerGain(dividerGain),
                          numCells(numCells),
                          minCellV(minCellV),
                          maxCellV(maxCellV)
    {
        adc_oneshot_unit_init_cfg_t init_cfg = {
            .unit_id = unit,
            .ulp_mode = ADC_ULP_MODE_DISABLE};
        ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_cfg, &adcHandle));

        adc_oneshot_chan_cfg_t cfg = {
            .atten = ADC_ATTEN_DB_11,
            .bitwidth = ADC_BITWIDTH_DEFAULT};
        ESP_ERROR_CHECK(adc_oneshot_config_channel(adcHandle, channel, &cfg));
    }

    float readRawADC()
    {
        int raw = 0;
        adc_oneshot_read(adcHandle, channel, &raw);
        return raw;
    }

    float adcToVoltage(int raw) const
    {
        constexpr float ADC_MAX_VOLT = 3.9f; // ESP32 approx
        return (raw / 4095.0f) * ADC_MAX_VOLT;
    }

    float readBatteryVoltage()
    {
        int raw = readRawADC();
        float v_adc = adcToVoltage(raw);
        return v_adc * dividerGain;
    }

    // NiMH non-linéaire → lookup + interpolation
    float getPercentage(float vbat)
    {
        float vcell = vbat / numCells;

        const std::vector<std::pair<float, float>> curve = {
            {1.40f, 100.0f},
            {1.30f, 80.0f},
            {1.25f, 60.0f},
            {1.20f, 40.0f},
            {1.15f, 20.0f},
            {1.10f, 10.0f},
            {1.00f, 0.0f}};

        if (vcell >= curve.front().first)
            return 100.0f;
        if (vcell <= curve.back().first)
            return 0.0f;

        for (size_t i = 0; i < curve.size() - 1; i++)
        {
            auto [v1, p1] = curve[i];
            auto [v2, p2] = curve[i + 1];

            if (vcell <= v1 && vcell >= v2)
            {
                float t = (vcell - v2) / (v1 - v2);
                return p2 + t * (p1 - p2);
            }
        }
        return 0.0f;
    }

    // Estimation autonomie (rough)
    float estimateHoursLeft(float percent, float load_mA)
    {
        if (load_mA <= 0)
            return 0;
        constexpr float capacity_mAh = 800.0f; // AAA NiMH typique
        return (capacity_mAh * (percent / 100.0f)) / load_mA;
    }

    // Évaluation santé batterie (SOH simplifié)
    float estimateSOH(float vbat)
    {
        float full = maxCellV * numCells;
        return std::min(100.0f, (vbat / full) * 100.0f);
    }

private:
    adc_unit_t unit;
    adc_channel_t channel;
    float dividerGain;
    int numCells;
    float minCellV;
    float maxCellV;

    adc_oneshot_unit_handle_t adcHandle;
};
