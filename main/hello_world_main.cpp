/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "LovyanGFX.hpp"

#include "esp_log.h"
#define TAG "BADGE"



// Configuration pour ILI9341 (écran CYD ESP32 2432S028)
#include <LovyanGFX.hpp>
class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ILI9341 _panel_instance;
  lgfx::Bus_SPI _bus_instance;
  lgfx::Light_PWM _light_instance;
public:
  LGFX(void) {
    {
      auto cfg = _bus_instance.config();
      cfg.spi_host = HSPI_HOST;     // HSPI utilisé pour le display
      cfg.spi_mode = 0;
      cfg.freq_write = 40000000;    // Fréquence d'écriture optimale pour CYD
      cfg.freq_read  = 16000000;
      cfg.spi_3wire = false;
      cfg.use_lock = true;
      cfg.dma_channel = SPI_DMA_CH_AUTO;
      cfg.pin_sclk = 14;   // SCLK sur GPIO14 (CYD standard)
      cfg.pin_mosi = 13;   // MOSI sur GPIO13 (CYD standard)
      cfg.pin_miso = 12;   // MISO sur GPIO12 (CYD standard) - non utilisé pour display
      cfg.pin_dc   = 2;    // DC (Data/Command) sur GPIO2 (CYD standard)
      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }
    {
      auto cfg = _panel_instance.config();
      cfg.pin_cs = 15;     // CS (Chip Select) sur GPIO15 (CYD standard)
      cfg.pin_rst = -1;    // RST non utilisé sur CYD
      cfg.pin_busy = -1;   // Non utilisé
      cfg.memory_width = 320;
      cfg.memory_height = 240;
      cfg.panel_width = 320;
      cfg.panel_height = 240;
      cfg.offset_x = 0;
      cfg.offset_y = 0;
      cfg.offset_rotation = 0;
      cfg.dummy_read_pixel = 8;
      cfg.dummy_read_bits = 1;
      cfg.readable = false;
      cfg.invert = false;
      cfg.rgb_order = true;
      cfg.dlen_16bit = false;
      cfg.bus_shared = false;
      _panel_instance.config(cfg);
    }
    {
      auto cfg = _light_instance.config();
      cfg.pin_bl = 21;              // Backlight sur GPIO21 (CYD standard)
      cfg.invert = false;           // false = HIGH pour allumer
      cfg.freq   = 44100;           // Fréquence PWM
      cfg.pwm_channel = 7;          // Canal PWM
      _light_instance.config(cfg);
      _panel_instance.setLight(&_light_instance);
    }
    setPanel(&_panel_instance);
  }
};

LGFX lcd;

extern "C" void app_main(void)
{
  ESP_LOGI(TAG, "Initialisation de l'écran...");
  lcd.init();
  ESP_LOGI(TAG, "lcd.init() OK");
  lcd.setBrightness(255); // Luminosité max (0-255)
  ESP_LOGI(TAG, "setBrightness OK");
  lcd.setRotation(5); // mode portrait
  ESP_LOGI(TAG, "setRotation OK");
  lcd.fillScreen(TFT_BLACK);
  ESP_LOGI(TAG, "fillScreen OK");
  lcd.setTextColor(TFT_RED);
  ESP_LOGI(TAG, "setTextColor OK");
  // Augmente la taille de la police
  lcd.setTextSize(2);
  // Calcul des coordonnées pour centrer le texte
  const char* text = "Hello World !";
  int text_width = lcd.textWidth(text);
  int text_height = lcd.fontHeight();
  int x = (lcd.width() - text_width) / 2;
  int y = (lcd.height() - text_height) / 2;
  lcd.drawRect(0, 0, lcd.width(), lcd.height(), TFT_WHITE); // Rectangle pour visualiser la zone
  lcd.drawString(text, x, y);
  ESP_LOGI(TAG, "drawString OK");
  while (true) {
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}
