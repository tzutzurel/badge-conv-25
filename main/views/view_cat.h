#ifndef VIEW_CAT_H
#define VIEW_CAT_H

#include "view.h"
#include "button.h"
#include "../state.h"
#include "../lgfx_custom.h"

// Structure pour les Z du chat qui dort
struct SleepZ
{
    float x;
    float y;
    float life;
    float size;
    bool active;
};

class ViewCat : public View
{
public:
    ViewCat(AppState &state, LGFX &lcd);
    void render(LGFX &display, LGFX_Sprite &spr) override;
    bool handleTouch(int x, int y) override;
    bool isInteractiveView() const override { return true; }
    bool isTouchInInteractiveZone(int x, int y) const override
    {
        // Vérifier si le touch est dans la zone du chat (rayon de 70 pixels)
        float dx = x - m_cat_x;
        float dy = y - m_cat_y;
        float dist = sqrt(dx * dx + dy * dy);
        return dist < 70;
    }

    void init();
    void update(float dt);

    void renderBackground(LGFX_Sprite &spr);
    void renderCat(LGFX_Sprite &spr);
    void renderSleepingZs(LGFX_Sprite &spr);
    void renderLion(LGFX_Sprite &spr);

    void spawnZ();

private:
    AppState &m_state;
    LGFX &m_lcd;

    // États du chat
    enum CatState
    {
        SLEEPING,    // Chat dort
        AWAKE,       // Chat réveillé
        ANGRY,       // Chat commence à être agacé
        LION         // Transformé en lion qui rugit
    };

    CatState m_cat_state = SLEEPING;
    
    // Compteurs
    float m_pet_duration = 0.0f;        // Durée totale de caresse (en secondes)
    float m_calm_timer = 0.0f;          // Timer pour calmer le chat
    float m_animation_timer = 0.0f;     // Timer pour animations
    float m_z_spawn_timer = 0.0f;       // Timer pour spawner les Z
    float m_eye_blink_timer = 0.0f;     // Timer pour clignement des yeux
    bool m_eyes_closed = false;         // Yeux fermés ou ouverts
    
    // Position du chat
    int m_cat_x;
    int m_cat_y;
    
    // Particules Z pour le sommeil
    static const int MAX_ZS = 5;
    SleepZ m_zs[MAX_ZS];
    
    // Couleurs
    uint16_t m_colBackground;
    uint16_t m_colCatBody;
    uint16_t m_colCatEyes;
    uint16_t m_colCatNose;
    uint16_t m_colWhite;
    uint16_t m_colBlack;
    uint16_t m_colYellow;
    uint16_t m_colOrange;
    uint16_t m_colRed;
    uint16_t m_colPink;
    
    // Animation du lion
    float m_lion_scale = 1.0f;          // Échelle du lion (grossit quand il rugit)
    bool m_lion_roaring = false;        // Est-ce que le lion rugit actuellement
    float m_roar_timer = 0.0f;          // Timer pour l'animation de rugissement
    
    // Détection de mouvement pour caresses
    bool m_is_touching = false;          // Doigt actuellement sur l'écran
    int m_last_touch_x = 0;              // Dernière position X du touch
    int m_last_touch_y = 0;              // Dernière position Y du touch
    float m_stroke_distance = 0.0f;      // Distance parcourue pendant la caresse
    
    bool m_initialized = false;
};

#endif // VIEW_CAT_H
