#include "view_plasma.h"
#include "user_info.h"
#include "esp_timer.h"
#include "../Orbitron_Bold24pt7b.h"

// Lookup table pour FastSin: 128 entrées, valeurs de 0 à 255
// Représente une demi-période de sinusoïde (0 à π)
static const uint8_t sin_lookup[128] = {
    128, 134, 140, 146, 152, 158, 165, 170,
    176, 182, 188, 193, 198, 203, 208, 213,
    218, 222, 226, 230, 234, 237, 240, 243,
    245, 248, 250, 251, 253, 254, 254, 255,
    255, 255, 254, 254, 253, 251, 250, 248,
    245, 243, 240, 237, 234, 230, 226, 222,
    218, 213, 208, 203, 198, 193, 188, 182,
    176, 170, 165, 158, 152, 146, 140, 134,
    128, 121, 115, 109, 103, 97, 90, 85,
    79, 73, 67, 62, 57, 52, 47, 42,
    37, 33, 29, 25, 21, 18, 15, 12,
    10, 7, 5, 4, 2, 1, 1, 0,
    0, 0, 1, 1, 2, 4, 5, 7,
    10, 12, 15, 18, 21, 25, 29, 33,
    37, 42, 47, 52, 57, 62, 67, 73,
    79, 85, 90, 97, 103, 109, 115, 121};

static const float sinf_lookup[128] = {
    0.0f, 0.049067674327418015f, 0.0980171403295606f, 0.14673047445536175f,
    0.19509032201612825f, 0.24298017990326387f, 0.29028467725446233f, 0.33688985339222005f,
    0.3826834323650898f, 0.4275550934302821f, 0.47139673682599764f, 0.5141027441932217f,
    0.5555702330196022f, 0.5956993044924334f, 0.6343932841636455f, 0.6715589548470183f,
    0.7071067811865475f, 0.7409511253549591f, 0.773010453362737f, 0.8032075314806448f,
    0.8314696123025452f, 0.8577286100002721f, 0.8819212643483549f, 0.9039892931234433f,
    0.9238795325112867f, 0.9415440651830208f, 0.9569403357322089f, 0.970031253194544f,
    0.9807852804032304f, 0.989176509964781f, 0.9951847266721968f, 0.9987954562051724f,
    1.0f, 0.9987954562051724f, 0.9951847266721969f, 0.989176509964781f,
    0.9807852804032304f, 0.970031253194544f, 0.9569403357322089f, 0.9415440651830208f,
    0.9238795325112867f, 0.9039892931234434f, 0.881921264348355f, 0.8577286100002721f,
    0.8314696123025455f, 0.8032075314806449f, 0.7730104533627371f, 0.740951125354959f,
    0.7071067811865476f, 0.6715589548470186f, 0.6343932841636455f, 0.5956993044924335f,
    0.5555702330196022f, 0.5141027441932218f, 0.47139673682599786f, 0.42755509343028203f,
    0.3826834323650899f, 0.33688985339222033f, 0.2902846772544624f, 0.24298017990326407f,
    0.1950903220161286f, 0.1467304744553618f, 0.09801714032956083f, 0.049067674327417966f,
    0.0f, -0.049067674327417724f, -0.09801714032956059f, -0.14673047445536158f,
    -0.19509032201612836f, -0.24298017990326382f, -0.2902846772544621f, -0.3368898533922201f,
    -0.38268343236508967f, -0.4275550934302818f, -0.47139673682599764f, -0.5141027441932216f,
    -0.555570233019602f, -0.5956993044924332f, -0.6343932841636453f, -0.6715589548470184f,
    -0.7071067811865475f, -0.7409511253549589f, -0.7730104533627367f, -0.803207531480645f,
    -0.8314696123025452f, -0.857728610000272f, -0.8819212643483549f, -0.9039892931234431f,
    -0.9238795325112865f, -0.9415440651830208f, -0.9569403357322088f, -0.970031253194544f,
    -0.9807852804032303f, -0.9891765099647809f, -0.9951847266721969f, -0.9987954562051724f,
    -1.0f, -0.9987954562051724f, -0.9951847266721969f, -0.9891765099647809f,
    -0.9807852804032304f, -0.970031253194544f, -0.9569403357322089f, -0.9415440651830209f,
    -0.9238795325112866f, -0.9039892931234433f, -0.881921264348355f, -0.8577286100002722f,
    -0.8314696123025455f, -0.8032075314806453f, -0.7730104533627369f, -0.7409511253549591f,
    -0.7071067811865477f, -0.6715589548470187f, -0.6343932841636459f, -0.5956993044924332f,
    -0.5555702330196022f, -0.5141027441932219f, -0.4713967368259979f, -0.42755509343028253f,
    -0.3826834323650904f, -0.33688985339222f, -0.2902846772544625f, -0.24298017990326418f,
    -0.19509032201612872f, -0.1467304744553624f, -0.0980171403295605f, -0.04906767432741809f};

// Repompe du code de Quake III Arena pour une racine carrée rapide
static float fastSqrt(long x)
{
    if (x < 2l)
        return 1.0f;

    // Conversion vers float pour utiliser l'algorithme
    float xf = (float)x; // static_cast<float>(x);
    float xhalf = 0.5f * xf;

    // Magic constant pour l'inverse de racine carrée rapide
    union
    {
        float f;
        uint32_t i;
    } conv;

    conv.f = xf;
    conv.i = 0x5f3759df - (conv.i >> 1);                // Constante magique
    conv.f = conv.f * (1.5f - xhalf * conv.f * conv.f); // Une itération de Newton

    // Calculer sqrt(x) = x * (1/sqrt(x))
    float result = xf * conv.f;

    return result + 0.5f; // Arrondi au plus proche
};

// Fonction FastSin: prend un angle en radians, retourne une valeur entre 0 et 255
static uint8_t FastSin(float radians)
{
    int index = (int)(radians * 128.0f / 6.28318530718f);
    index &= 0x7F; // Assurer que l'index est dans [0, 127]
    return sin_lookup[index];
}

static float FastSinf(float radians)
{
    int index = (int)(radians * 128.0f / 6.28318530718f);
    index &= 0x7F; // Assurer que l'index est dans [0, 127]
    return sinf_lookup[index];
}

ViewPlasma::ViewPlasma(AppState &state, LGFX &lcd)
    : m_state(state), m_lcd(lcd), tempo(0.0f)
{
}

void ViewPlasma::updateAnimation(float dt)
{
    // Incrémenter le tempo pour l'animation
    tempo += dt;
    if (tempo > 200.0f)
    { // Éviter les débordements
        tempo -= 200.0f;
    }
}

void ViewPlasma::renderPlasma(LGFX_Sprite &spr)
{
    // Accéder aux dimensions depuis l'état
    const int width = m_state.screenW;
    const int height = m_state.screenH;
    const int cx = width >> 1;
    const int cy = height >> 1;

    spr.startWrite(); // Commencer l'écriture pour de meilleures performances

    for (int y = 0; y < height; y += 4)
    {
        for (int x = 0; x < width; x += 4)
        {
            // Calcul des distances au centre
            long dx = x - cx;
            long dy = y - cy;
            float dist = fastSqrt(dx * dx + dy * dy);

            // Multiples ondes sinusoïdales
            float x1 = x * 0.04f;
            float y1 = y * 0.03f;

            float val1 = FastSinf(x1 + tempo);
            float val2 = FastSinf(y1 - tempo * 0.7f);
            float val3 = FastSinf(x1 + y1 + tempo * 1.2f);
            float val4 = FastSinf(dist * 0.05f - tempo * 0.5f);

            // Combinaison des ondes
            float plasma = val1 + val2 + val3 + val4;

            // Mapping en couleurs RGB avec palette type demo-scene
            uint8_t r = FastSin(plasma + tempo * 0.3f);
            uint8_t g = FastSin(plasma + tempo * 0.5f + 1.33f);
            uint8_t b = FastSin(plasma + tempo * 0.7f + 2.66f);

            // Créer la couleur 16-bit
            // uint16_t color = m_lcd.color565(r, g, b);
            // uint16_t color = r >> 3 << 11 | g >> 2 << 5 | b >> 3; // Equivalent à color565
            uint16_t color = r >> 4 << 12 | g >> 3 << 6 | b >> 4 << 1;

            // Dessiner le pixel
            spr.fillRect(x, y, 4, 4, color);
        }
    }

    spr.endWrite(); // Terminer l'écriture
}

void ViewPlasma::renderName(LGFX_Sprite &spr)
{
    // Afficher le prénom en haut de l'écran avec effet néon
    spr.setTextDatum(TC_DATUM);
    spr.setFont(&Orbitron_Bold24pt7b);
    spr.setTextSize(0.8);

    uint16_t blackColor = m_lcd.color565(255, 0, 150);
    uint16_t shadowColor = m_lcd.color565(0, 0, 0);

    int centerX = m_state.screenW / 2;
    int y = 20;

    // Effet néon simple : ombre décalée
    spr.setTextColor(shadowColor);
    spr.drawString(user_info.prenom.c_str(), centerX + 2, y + 2);

    // Texte principal
    spr.setTextColor(blackColor);
    spr.drawString(user_info.prenom.c_str(), centerX, y);

    spr.setFont(nullptr);
}

void ViewPlasma::render(LGFX &display, LGFX_Sprite &spr)
{
    // Utiliser le delta time calculé globalement
    float dt = m_state.dt;

    // Mise à jour de l'animation
    updateAnimation(dt);

    // Rendu du plasma
    renderPlasma(spr);

    // Afficher le prénom au-dessus du plasma
    renderName(spr);
}

bool ViewPlasma::handleTouch(int x, int y)
{
    // Pas d'interaction tactile pour cette vue
    return false;
}
