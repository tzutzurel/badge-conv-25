#include "config.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include <cstring>

static const char *TAG = "CONFIG";

namespace Config
{
    static const char *NAMESPACE = "appcfg";
    static const char *KEY_ROTATED = "rotated";
    static const char *KEY_ACTIVE_BRIGHT = "active_bright";
    static const char *KEY_SLEEP_BRIGHT = "sleep_bright";
    static const char *KEY_AWAKE_TIME = "awake_time";
    float awakeTime = 5.0f;
    uint8_t sleepBrightness = 10;
    uint8_t activeBrightness = 80;
    bool display_rotated = false;

    void setActiveBrightness(uint8_t value)
    {
        activeBrightness = value;
        saveToNVS();
    }
    void setSleepBrightness(uint8_t value)
    {
        sleepBrightness = value;
        saveToNVS();
    }
    void setDisplayRotated(bool value)
    {
        display_rotated = value;
        saveToNVS();
    }
    void setAwakeTime(float value)
    {
        awakeTime = value;
        saveToNVS();
    }

    void saveToNVS()
    {
        nvs_handle_t handle;
        esp_err_t err = nvs_open(NAMESPACE, NVS_READWRITE, &handle);
        if (err == ESP_OK)
        {
            nvs_set_u8(handle, KEY_ACTIVE_BRIGHT, activeBrightness);
            nvs_set_u8(handle, KEY_SLEEP_BRIGHT, sleepBrightness);
            nvs_set_u8(handle, KEY_ROTATED, display_rotated ? 1 : 0);
            nvs_set_blob(handle, KEY_AWAKE_TIME, &awakeTime, sizeof(awakeTime));
            err = nvs_commit(handle);
            if (err == ESP_OK)
            {
                ESP_LOGI(TAG, "Configuration saved to NVS");
            }
            else
            {
                ESP_LOGE(TAG, "Failed to commit to NVS: %s", esp_err_to_name(err));
            }
            nvs_close(handle);
        }
        else
        {
            ESP_LOGE(TAG, "Failed to open NVS for writing: %s", esp_err_to_name(err));
        }
    }

    void loadFromNVS()
    {
        nvs_handle_t handle;
        esp_err_t err = nvs_open(NAMESPACE, NVS_READONLY, &handle);
        if (err == ESP_OK)
        {
            uint8_t val8;
            if (nvs_get_u8(handle, KEY_ACTIVE_BRIGHT, &val8) == ESP_OK)
            {
                activeBrightness = val8;
                ESP_LOGI(TAG, "Loaded activeBrightness: %d", activeBrightness);
            }
            if (nvs_get_u8(handle, KEY_SLEEP_BRIGHT, &val8) == ESP_OK)
            {
                sleepBrightness = val8;
                ESP_LOGI(TAG, "Loaded sleepBrightness: %d", sleepBrightness);
            }
            if (nvs_get_u8(handle, KEY_ROTATED, &val8) == ESP_OK)
            {
                display_rotated = (val8 != 0);
                ESP_LOGI(TAG, "Loaded display_rotated: %d", display_rotated);
            }
            size_t size = sizeof(awakeTime);
            if (nvs_get_blob(handle, KEY_AWAKE_TIME, &awakeTime, &size) == ESP_OK)
            {
                ESP_LOGI(TAG, "Loaded awakeTime: %.2f", awakeTime);
            }
            nvs_close(handle);
        }
        else
        {
            ESP_LOGW(TAG, "Failed to open NVS for reading (using defaults): %s", esp_err_to_name(err));
        }
    }

    void initNVS()
    {
        static bool initialized = false;
        if (!initialized)
        {
            esp_err_t err = nvs_flash_init();
            if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
            {
                ESP_LOGW(TAG, "NVS partition needs to be erased (error: %s)", esp_err_to_name(err));
                ESP_ERROR_CHECK(nvs_flash_erase());
                err = nvs_flash_init();
            }
            ESP_ERROR_CHECK(err);
            ESP_LOGI(TAG, "NVS initialized successfully");
            initialized = true;
        }
    }
}