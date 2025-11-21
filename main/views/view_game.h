#ifndef VIEW_GAME_H
#define VIEW_GAME_H

#include "view.h"
#include "button.h"
#include "../state.h"
#include "../lgfx_custom.h"

// Structure pour les cultures à protéger
struct Crop
{
    int x;
    int y;
    int health;              // 0-100
    bool infected;           // Infecté par un nuisible
    float pulse;             // Animation
    float health_loss_accum; // Accumulateur pour perte de santé fractionnelle
};

// Structure pour les menaces
struct Threat
{
    float x;
    float y;
    float vx;
    float vy;
    bool active;
    uint8_t type; // 0=insecte, 1=nuage orage, 2=grêle
    float size;
    unsigned long spawn_time;
    uint8_t movement_pattern; // 0=direct, 1=sinusoidal, 2=circular
    float phase;              // Phase pour les mouvements sinusoïdaux
    float target_x;           // Cible X pour trajectoire intelligente
    float target_y;           // Cible Y pour trajectoire intelligente
};

// Particules pour effets visuels
struct GameParticle
{
    float x;
    float y;
    float vx;
    float vy;
    float life;
    uint16_t color;
    bool active;
};

class ViewGame : public View
{
public:
    ViewGame(AppState &state, LGFX &lcd);
    void render(LGFX &display, LGFX_Sprite &spr) override;
    bool handleTouch(int x, int y) override;

    void init();
    void update(float dt);
    void handleTouchInternal(int touch_x, int touch_y);

    void renderBackground(LGFX_Sprite &spr);
    void renderCrops(LGFX_Sprite &spr);
    void renderThreats(LGFX_Sprite &spr);
    void renderParticles(LGFX_Sprite &spr);
    void renderHUD(LGFX_Sprite &spr);
    void renderGameOver(LGFX_Sprite &spr);
    void renderIntro(LGFX_Sprite &spr);

    void spawnThreat();
    void checkCollisions();
    void createExplosion(float x, float y, uint16_t color);

private:
    AppState &m_state;
    LGFX &m_lcd;

    // État du jeu
    bool m_initialized = false;
    bool m_show_intro = true;    // Afficher l'écran d'intro
    bool m_game_started = false; // Le jeu a démarré
    bool m_game_over = false;
    bool m_victory = false;
    int m_score = 0;
    int32_t m_best_score = 0;
    unsigned long m_last_spawn = 0;
    unsigned long m_spawn_interval = 2000; // ms entre spawn
    float m_game_time = 0.0f;
    unsigned long m_last_update = 0;
    unsigned long m_game_over_time = 0; // Temps quand game over est déclenché

    // Entités du jeu
    static const int MAX_CROPS = 6;
    static const int MAX_THREATS = 10;
    static const int MAX_PARTICLES = 40;

    Crop m_crops[MAX_CROPS];
    Threat m_threats[MAX_THREATS];
    GameParticle m_particles[MAX_PARTICLES];

    // Couleurs
    uint16_t m_colBackground;
    uint16_t m_colCyan;
    uint16_t m_colGreen;
    uint16_t m_colYellow;
    uint16_t m_colRed;
    uint16_t m_colOrange;
    uint16_t m_colMagenta;
    uint16_t m_colWhite;

    // Boutons
    Button m_backButton;
    Button m_playButton;
    Button m_replayButton;
};
#endif // VIEW_GAME_H
