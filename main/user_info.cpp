
#include "user_info.h"
#include "qrcodegen.h"
#include <string.h>
#include <stdlib.h>
#include "esp_log.h"

// Buffer global pour le QR code (taille optimisée)
uint8_t g_qrcode[qrcodegen_BUFFER_LEN_FOR_VERSION(QRCODE_VERSION_FOR_TOKEN)];
int g_qrcode_size = 0;

void user_info_generate_qrcode(const char *text)
{
    // log token
    ESP_LOGI("USER_INFO", "Generating QR code for token: %s", text);

    // Allouer le buffer temporaire sur le heap (taille optimisée)
    uint8_t *tempBuffer = (uint8_t *)malloc(qrcodegen_BUFFER_LEN_FOR_VERSION(QRCODE_VERSION_FOR_TOKEN));
    if (tempBuffer == NULL)
    {
        ESP_LOGE("USER_INFO", "Failed to allocate temp buffer for QR code");
        return;
    }

    // Génère le QR code à partir du texte fourni avec une version adaptée au token
    bool ok = qrcodegen_encodeText(text, tempBuffer, g_qrcode, qrcodegen_Ecc_LOW,
                                   qrcodegen_VERSION_MIN, QRCODE_VERSION_FOR_TOKEN,
                                   qrcodegen_Mask_AUTO, true);

    if (ok)
    {
        g_qrcode_size = qrcodegen_getSize(g_qrcode);
        ESP_LOGI("USER_INFO", "QR code generated successfully, size: %d", g_qrcode_size);
    }
    else
    {
        ESP_LOGE("USER_INFO", "Failed to generate QR code");
    }

    // Libérer le buffer temporaire
    free(tempBuffer);
}
