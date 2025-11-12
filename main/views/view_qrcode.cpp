#include "view_qrcode.h"
#include <string>
#include "user_info.h"
#include "qrcodegen.h"

// Affiche le QR code généré dans le buffer global g_qrcode
void ViewQRCode::render(LGFX &display, LGFX_Sprite &spr)
{
    spr.fillSprite(TFT_BLACK);
    spr.setTextDatum(TC_DATUM);
    spr.setTextFont(2);
    spr.setTextSize(2);
    spr.setTextColor(TFT_WHITE);
    std::string titre = user_info.prenom + std::string(" ") + user_info.nom;
    spr.drawString(titre.c_str(), spr.width() / 2, 30);

    // Affichage du QR code
    int scale = 4; // Taille d'un module (pixel) du QR code
    int qr_size = g_qrcode_size;
    int qr_pix_size = qr_size * scale;
    int x0 = (spr.width() - qr_pix_size) / 2;
    int y0 = 70;
    for (int y = 0; y < qr_size; y++)
    {
        for (int x = 0; x < qr_size; x++)
        {
            if (qrcodegen_getModule(g_qrcode, x, y))
            {
                spr.fillRect(x0 + x * scale, y0 + y * scale, scale, scale, TFT_WHITE);
            }
            else
            {
                spr.fillRect(x0 + x * scale, y0 + y * scale, scale, scale, TFT_BLACK);
            }
        }
    }
    spr.drawRect(x0 - 2, y0 - 2, qr_pix_size + 4, qr_pix_size + 4, TFT_WHITE);
    spr.drawString("QR Code", spr.width() / 2, y0 + qr_pix_size + 20);
}
