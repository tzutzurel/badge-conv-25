#include "view_cat.h"
#include "button.h"
#include "esp_timer.h"
#include "esp_random.h"
#include "esp_log.h"
#include <cmath>

static const char *TAG = "ViewCat";

ViewCat::ViewCat(AppState &state, LGFX &lcd)
    : m_state(state), m_lcd(lcd)
{
    // Initialiser les couleurs
    m_colBackground = lcd.color565(20, 20, 40);      // Bleu nuit doux
    m_colCatBody = lcd.color565(255, 200, 150);      // Orange clair pour le chat
    m_colCatEyes = lcd.color565(100, 200, 100);      // Vert pour les yeux
    m_colCatNose = lcd.color565(255, 150, 150);      // Rose pour le nez
    m_colWhite = lcd.color565(255, 255, 255);
    m_colBlack = lcd.color565(0, 0, 0);
    m_colYellow = lcd.color565(255, 255, 100);
    m_colOrange = lcd.color565(255, 140, 0);
    m_colRed = lcd.color565(255, 0, 0);
    m_colPink = lcd.color565(255, 180, 200);

    // Position du chat au centre
    m_cat_x = m_state.screenW / 2;
    m_cat_y = m_state.screenH / 2 + 10;
}

void ViewCat::init()
{
    if (m_initialized)
        return;

    ESP_LOGI(TAG, "Initializing Cat View");

    // Réinitialiser l'état
    m_cat_state = SLEEPING;
    m_pet_duration = 0.0f;
    m_calm_timer = 0.0f;
    m_animation_timer = 0.0f;
    m_z_spawn_timer = 0.0f;
    m_eye_blink_timer = 0.0f;
    m_eyes_closed = false;
    m_lion_scale = 1.0f;
    m_lion_roaring = false;
    m_roar_timer = 0.0f;
    m_is_touching = false;
    m_last_touch_x = 0;
    m_last_touch_y = 0;
    m_stroke_distance = 0.0f;

    // Initialiser les Z
    for (int i = 0; i < MAX_ZS; i++)
    {
        m_zs[i].active = false;
    }

    m_initialized = true;
    ESP_LOGI(TAG, "Cat View initialized");
}

void ViewCat::update(float dt)
{
    m_animation_timer += dt;
    
    // Gestion du clignement des yeux (quand réveillé)
    if (m_cat_state == AWAKE || m_cat_state == ANGRY)
    {
        m_eye_blink_timer += dt;
        if (m_eye_blink_timer > 3.0f)
        {
            m_eyes_closed = true;
            if (m_eye_blink_timer > 3.2f)
            {
                m_eyes_closed = false;
                m_eye_blink_timer = 0.0f;
            }
        }
    }

    // Calmer le chat progressivement si on ne le touche pas
    if (m_pet_duration > 0.0f && !m_is_touching)
    {
        m_calm_timer += dt;
        // Diminuer progressivement la durée de caresse
        // Le chat se calme plus lentement maintenant (environ 10-12 secondes)
        m_pet_duration -= dt * 0.4f; // Diminue de 0.4 seconde par seconde (était 0.8)
        if (m_pet_duration < 0.0f)
            m_pet_duration = 0.0f;
    }

    // Mise à jour des états selon la durée de caresse
    if (m_pet_duration < 1.0f) // Moins de 1 seconde de caresse
    {
        if (m_cat_state != SLEEPING)
        {
            m_cat_state = SLEEPING;
            ESP_LOGI(TAG, "Cat is now sleeping");
        }
    }
    else if (m_pet_duration < 2.5f) // 1-2.5 secondes
    {
        if (m_cat_state != AWAKE && m_cat_state != SLEEPING)
        {
            // Descente depuis ANGRY ou LION
            m_cat_state = AWAKE;
            ESP_LOGI(TAG, "Cat calming down to awake");
        }
        else if (m_cat_state == SLEEPING)
        {
            m_cat_state = AWAKE;
            ESP_LOGI(TAG, "Cat waking up");
        }
    }
    else if (m_pet_duration < 5.0f) // 2.5-5 secondes
    {
        if (m_cat_state != ANGRY)
        {
            m_cat_state = ANGRY;
            if (m_pet_duration >= 2.5f)
                ESP_LOGI(TAG, "Cat is getting angry");
        }
    }
    else // Plus de 5 secondes
    {
        if (m_cat_state != LION)
        {
            m_cat_state = LION;
            m_lion_roaring = true;
            m_roar_timer = 0.0f;
            ESP_LOGI(TAG, "Cat transformed into LION!");
        }
    }

    // Animation du lion qui rugit
    if (m_cat_state == LION)
    {
        m_roar_timer += dt;
        
        // Animation de rugissement: le lion grossit et rapetisse
        if (m_lion_roaring)
        {
            m_lion_scale = 1.0f + sin(m_roar_timer * 10.0f) * 0.3f;
            
            // Arrêter de rugir après 3 secondes
            if (m_roar_timer > 3.0f)
            {
                m_lion_roaring = false;
            }
        }
        else
        {
            // Respiration lente du lion
            m_lion_scale = 1.0f + sin(m_animation_timer * 2.0f) * 0.1f;
        }
    }

    // Spawner des Z quand le chat dort
    if (m_cat_state == SLEEPING)
    {
        m_z_spawn_timer += dt;
        if (m_z_spawn_timer > 0.8f)
        {
            spawnZ();
            m_z_spawn_timer = 0.0f;
        }
    }

    // Mettre à jour les Z
    for (int i = 0; i < MAX_ZS; i++)
    {
        if (!m_zs[i].active)
            continue;

        // Les Z montent et grandissent
        m_zs[i].y -= 20.0f * dt;
        m_zs[i].size += 0.3f * dt;
        m_zs[i].life -= dt;

        // Faire osciller les Z légèrement
        m_zs[i].x += sin(m_animation_timer * 3.0f + i) * 0.5f;

        if (m_zs[i].life <= 0)
        {
            m_zs[i].active = false;
        }
    }
}

void ViewCat::spawnZ()
{
    // Trouver un slot libre
    for (int i = 0; i < MAX_ZS; i++)
    {
        if (!m_zs[i].active)
        {
            // Spawner au-dessus de la tête du chat, légèrement décalé
            m_zs[i].x = m_cat_x + 25 + (esp_random() % 10) - 5;
            m_zs[i].y = m_cat_y - 35;
            m_zs[i].life = 2.0f + (esp_random() % 100) / 100.0f;
            m_zs[i].size = 1.0f;
            m_zs[i].active = true;
            break;
        }
    }
}

bool ViewCat::handleTouch(int x, int y)
{
    // Coordonnées négatives signalent la fin du touch
    if (x < 0 || y < 0)
    {
        if (m_is_touching)
        {
            ESP_LOGI(TAG, "Touch ended - Total duration: %.1fs", m_pet_duration);
        }
        m_is_touching = false;
        m_stroke_distance = 0.0f;
        return false;
    }

    // Vérifier si on touche le chat (zone centrale, réduite pour laisser place aux bords)
    float dx = x - m_cat_x;
    float dy = y - m_cat_y;
    float dist = sqrt(dx * dx + dy * dy);

    if (dist < 70) // Rayon de détection (réduit de 100 à 70 pour laisser les bords libres)
    {
        if (m_is_touching)
        {
            // Calculer la distance parcourue depuis le dernier touch
            float move_dx = x - m_last_touch_x;
            float move_dy = y - m_last_touch_y;
            float move_dist = sqrt(move_dx * move_dx + move_dy * move_dy);
            
            // Accumuler la distance de caresse
            m_stroke_distance += move_dist;
            
            // Caresse détectée dès 10 pixels de mouvement (était 30)
            if (m_stroke_distance >= 10.0f)
            {
                m_stroke_distance = 0.0f; // Reset pour continuer à détecter
                ESP_LOGI(TAG, "Stroke detected - Duration: %.1fs", m_pet_duration);
            }
        }
        else
        {
            // Premier touch - initialiser
            m_is_touching = true;
            m_stroke_distance = 0.0f;
            ESP_LOGI(TAG, "Started touching cat");
        }
        
        // Accumuler la durée tant qu'on touche le chat (même sans mouvement)
        m_pet_duration += m_state.dt;
        m_calm_timer = 0.0f;
        
        m_last_touch_x = x;
        m_last_touch_y = y;
        
        return true; // Consommer le touch
    }
    else
    {
        // Doigt hors de la zone du chat
        if (m_is_touching)
        {
            ESP_LOGI(TAG, "Stopped touching cat - Total duration: %.1fs", m_pet_duration);
        }
        m_is_touching = false;
        m_stroke_distance = 0.0f;
    }

    return false;
}

void ViewCat::render(LGFX &display, LGFX_Sprite &spr)
{
    if (!m_initialized)
    {
        init();
    }

    update(m_state.dt);

    renderBackground(spr);
    
    if (m_cat_state == LION)
    {
        renderLion(spr);
    }
    else
    {
        renderCat(spr);
        if (m_cat_state == SLEEPING)
        {
            renderSleepingZs(spr);
        }
    }

    // Afficher un indicateur de l'état en bas
    spr.setTextDatum(TC_DATUM);
    spr.setTextSize(1);
    
    if (m_cat_state == SLEEPING)
    {
        spr.setTextColor(m_colYellow);
        spr.drawString("Chat endormi... Zzz", m_state.screenW / 2, m_state.screenH - 20);
    }
    else if (m_cat_state == AWAKE)
    {
        spr.setTextColor(m_colCatEyes);
        spr.drawString("Miaou~", m_state.screenW / 2, m_state.screenH - 20);
    }
    else if (m_cat_state == ANGRY)
    {
        spr.setTextColor(m_colOrange);
        spr.drawString("Grr... Arrete!", m_state.screenW / 2, m_state.screenH - 20);
    }
    else if (m_cat_state == LION)
    {
        spr.setTextColor(m_colRed);
        spr.drawString("ROAAARRR!!!", m_state.screenW / 2, m_state.screenH - 20);
    }
    
    spr.setTextDatum(TL_DATUM);
}

void ViewCat::renderBackground(LGFX_Sprite &spr)
{
    spr.fillScreen(m_colBackground);
    
    // Ajouter quelques étoiles pour l'ambiance nuit
    for (int i = 0; i < 20; i++)
    {
        int star_x = (i * 37 + 15) % m_state.screenW;
        int star_y = (i * 23 + 10) % m_state.screenH;
        
        // Scintillement
        float twinkle = sin(m_animation_timer * 3.0f + i) * 0.5f + 0.5f;
        uint8_t brightness = 150 + (uint8_t)(twinkle * 105);
        
        spr.fillCircle(star_x, star_y, 1, m_lcd.color565(brightness, brightness, brightness));
    }
}

void ViewCat::renderSleepingZs(LGFX_Sprite &spr)
{
    for (int i = 0; i < MAX_ZS; i++)
    {
        if (!m_zs[i].active)
            continue;

        int z_x = (int)m_zs[i].x;
        int z_y = (int)m_zs[i].y;

        uint16_t color = m_lcd.color565(200, 200, 255);

        // Dessiner le Z
        spr.setTextSize((int)m_zs[i].size + 1);
        spr.setTextColor(color);
        spr.setCursor(z_x, z_y);
        spr.print("Z");
    }
    
    spr.setTextSize(1); // Reset
}

void ViewCat::renderCat(LGFX_Sprite &spr)
{
    int x = m_cat_x;
    int y = m_cat_y;
    
    // Animation de respiration
    float breath = sin(m_animation_timer * 2.0f) * 0.05f + 1.0f;
    int body_h = (int)(30 * breath);
    
    // Corps du chat (ellipse)
    spr.fillEllipse(x, y, 35, body_h, m_colCatBody);
    spr.drawEllipse(x, y, 35, body_h, m_colOrange);
    
    // Tête du chat (cercle)
    int head_y = y - 25;
    spr.fillCircle(x, head_y, 20, m_colCatBody);
    spr.drawCircle(x, head_y, 20, m_colOrange);
    
    // Oreilles (triangles)
    // Oreille gauche
    spr.fillTriangle(x - 18, head_y - 10, x - 10, head_y - 20, x - 5, head_y - 8, m_colCatBody);
    spr.drawTriangle(x - 18, head_y - 10, x - 10, head_y - 20, x - 5, head_y - 8, m_colOrange);
    // Oreille droite
    spr.fillTriangle(x + 18, head_y - 10, x + 10, head_y - 20, x + 5, head_y - 8, m_colCatBody);
    spr.drawTriangle(x + 18, head_y - 10, x + 10, head_y - 20, x + 5, head_y - 8, m_colOrange);
    
    if (m_cat_state == SLEEPING)
    {
        // Yeux fermés (petites lignes)
        spr.drawLine(x - 10, head_y, x - 5, head_y, m_colBlack);
        spr.drawLine(x + 5, head_y, x + 10, head_y, m_colBlack);
    }
    else
    {
        // Yeux ouverts ou qui clignent
        if (m_eyes_closed)
        {
            spr.drawLine(x - 10, head_y, x - 5, head_y, m_colBlack);
            spr.drawLine(x + 5, head_y, x + 10, head_y, m_colBlack);
        }
        else
        {
            // Yeux ouverts
            spr.fillCircle(x - 7, head_y, 4, m_colWhite);
            spr.fillCircle(x + 7, head_y, 4, m_colWhite);
            spr.fillCircle(x - 7, head_y, 2, m_colCatEyes);
            spr.fillCircle(x + 7, head_y, 2, m_colCatEyes);
            
            // Pupilles (plus petites si en colère)
            if (m_cat_state == ANGRY)
            {
                spr.fillCircle(x - 7, head_y, 1, m_colBlack);
                spr.fillCircle(x + 7, head_y, 1, m_colBlack);
                
                // Sourcils froncés
                spr.drawLine(x - 10, head_y - 5, x - 4, head_y - 3, m_colRed);
                spr.drawLine(x + 10, head_y - 5, x + 4, head_y - 3, m_colRed);
            }
            else
            {
                spr.fillCircle(x - 7, head_y + 1, 1, m_colBlack);
                spr.fillCircle(x + 7, head_y + 1, 1, m_colBlack);
            }
        }
    }
    
    // Nez
    spr.fillCircle(x, head_y + 5, 2, m_colCatNose);
    
    // Bouche (sourire ou neutre)
    if (m_cat_state == ANGRY)
    {
        // Bouche mécontente
        spr.drawLine(x - 5, head_y + 10, x, head_y + 8, m_colBlack);
        spr.drawLine(x, head_y + 8, x + 5, head_y + 10, m_colBlack);
    }
    else if (m_cat_state == AWAKE)
    {
        // Petit sourire
        spr.drawLine(x - 5, head_y + 8, x, head_y + 10, m_colBlack);
        spr.drawLine(x, head_y + 10, x + 5, head_y + 8, m_colBlack);
    }
    
    // Moustaches
    spr.drawLine(x - 20, head_y + 2, x - 8, head_y + 3, m_colBlack);
    spr.drawLine(x - 20, head_y + 6, x - 8, head_y + 5, m_colBlack);
    spr.drawLine(x + 20, head_y + 2, x + 8, head_y + 3, m_colBlack);
    spr.drawLine(x + 20, head_y + 6, x + 8, head_y + 5, m_colBlack);
    
    // Queue (ondule légèrement)
    int tail_x = x + 35;
    int tail_y = y + 10;
    
    for (int i = 0; i < 5; i++)
    {
        int tx = tail_x + i * 5;
        int ty = tail_y + (int)(sin((float)i * 0.5f + m_animation_timer * 3.0f) * 8.0f);
        spr.fillCircle(tx, ty, 4 - i/2, m_colCatBody);
    }
    
    // Pattes (4 pattes)
    spr.fillEllipse(x - 15, y + 25, 5, 10, m_colCatBody);
    spr.fillEllipse(x + 15, y + 25, 5, 10, m_colCatBody);
    spr.fillEllipse(x - 10, y + 28, 4, 8, m_colCatBody);
    spr.fillEllipse(x + 10, y + 28, 4, 8, m_colCatBody);
}

void ViewCat::renderLion(LGFX_Sprite &spr)
{
    int x = m_cat_x;
    int y = m_cat_y;
    
    // Le lion est plus gros et plus proche (effet zoom)
    float scale = m_lion_scale * 1.8f;
    
    // Animation de rugissement (la tête bouge)
    float shake = 0;
    if (m_lion_roaring)
    {
        shake = sin(m_roar_timer * 30.0f) * 3.0f;
    }
    
    // Position de la tête - bien centrée et visible
    int head_y = (int)(y - 10 * scale);
    int head_x = (int)(x + shake);
    
    // Corps du lion EN DESSOUS de la tête (ellipse)
    spr.fillEllipse(x, y + (int)(15 * scale), (int)(35 * scale), (int)(30 * scale), m_colOrange);
    spr.drawEllipse(x, y + (int)(15 * scale), (int)(35 * scale), (int)(30 * scale), m_lcd.color565(200, 100, 0));
    
    // Pattes AVANT (visibles de face)
    spr.fillEllipse(x - (int)(18 * scale), y + (int)(38 * scale), (int)(7 * scale), (int)(12 * scale), m_colOrange);
    spr.fillEllipse(x + (int)(18 * scale), y + (int)(38 * scale), (int)(7 * scale), (int)(12 * scale), m_colOrange);
    // Pattes avec griffes
    spr.fillCircle(x - (int)(18 * scale), y + (int)(48 * scale), (int)(4 * scale), m_lcd.color565(200, 100, 0));
    spr.fillCircle(x + (int)(18 * scale), y + (int)(48 * scale), (int)(4 * scale), m_lcd.color565(200, 100, 0));
    
    // Crinière réaliste - autour de la tête avec emphase sur le haut et les côtés
    // Couches de crinière pour effet de profondeur
    int criniere_layers = 3;
    
    for (int layer = criniere_layers - 1; layer >= 0; layer--)
    {
        int base_radius = (int)(24 * scale);
        int layer_offset = layer * (int)(7 * scale);
        int num_tufts = 24 + layer * 4; // Plus de touffes pour un aspect dense
        
        for (int i = 0; i < num_tufts; i++)
        {
            // Répartition angulaire: emphase sur le haut (de -150° à +150°)
            // On évite le bas (menton) qui serait entre 150° et 210° (-150°)
            // Rotation de -90° (PI/2) pour corriger le décalage
            // Couverture angulaire augmentée pour combler le côté gauche
            float angle_progress = (float)i / (float)num_tufts;
            float angle = -2.8f + (angle_progress * 5.6f) - 1.5708f; // De -160° à +160° puis rotation -90°
            
            // Normaliser l'angle entre -PI et +PI
            while (angle > 3.14159f) angle -= 6.28318f;
            while (angle < -3.14159f) angle += 6.28318f;
            
            // Exclure la partie basse (menton) - après rotation, c'est maintenant sur la droite
            // On garde tout sauf entre ~60° et ~120° (partie basse du menton après rotation)
            bool is_chin_area = (angle > 1.0f && angle < 2.1f);
            
            if (!is_chin_area)
            {
                // Variation de longueur selon la position
                // Les touffes sur le dessus de la tête sont plus longues
                float height_factor = 1.0f;
                if (angle > -1.57f && angle < 1.57f) // Partie supérieure (-90° à +90°)
                {
                    // Plus long en haut
                    height_factor = 1.3f + cos(angle) * 0.3f; // Max au sommet
                }
                else
                {
                    // Côtés et arrière
                    height_factor = 1.0f + cos(angle) * 0.2f;
                }
                
                // Variation naturelle aléatoire
                float random_variation = 0.85f + (sin((float)i * 2.3f) * 0.15f);
                float length_variation = height_factor * random_variation;
                
                int tip_radius = base_radius + layer_offset + (int)(layer_offset * length_variation);
                
                int tip_x = head_x + (int)(cos(angle) * tip_radius);
                int tip_y = head_y + (int)(sin(angle) * tip_radius);
                
                // Base du triangle
                float base_width = 0.13f; // Triangles plus fins
                float base1_angle = angle - base_width;
                float base2_angle = angle + base_width;
                int base_tri_radius = base_radius + (int)(layer_offset * 0.4f);
                int base_x1 = head_x + (int)(cos(base1_angle) * base_tri_radius);
                int base_y1 = head_y + (int)(sin(base1_angle) * base_tri_radius);
                int base_x2 = head_x + (int)(cos(base2_angle) * base_tri_radius);
                int base_y2 = head_y + (int)(sin(base2_angle) * base_tri_radius);
                
                // Dégradé de couleurs selon la couche et la position
                uint16_t mane_color;
                
                // Variation selon l'angle (plus clair sur le dessus, plus foncé sur les côtés)
                // Après rotation de -90°, le haut de la tête est maintenant entre -2.57 et -0.57 (environ)
                bool is_top = (angle > -2.5f && angle < -0.6f); // Haut de la tête après rotation
                
                if (layer == 0)
                {
                    // Couche intérieure: marron foncé
                    mane_color = is_top ? m_lcd.color565(170, 85, 15) : m_lcd.color565(150, 75, 10);
                }
                else if (layer == 1)
                {
                    // Couche moyenne: orange avec variation
                    if (is_top)
                    {
                        mane_color = (i % 3 == 0) ? m_lcd.color565(240, 130, 30) : m_lcd.color565(220, 115, 25);
                    }
                    else
                    {
                        mane_color = (i % 3 == 0) ? m_lcd.color565(210, 105, 20) : m_lcd.color565(195, 98, 18);
                    }
                }
                else
                {
                    // Couche externe: doré/orange clair (plus de volume sur le haut)
                    if (is_top)
                    {
                        // Haut: couleurs très claires et dorées
                        if (i % 4 == 0)
                            mane_color = m_lcd.color565(255, 180, 50);
                        else if (i % 4 == 1)
                            mane_color = m_lcd.color565(255, 170, 45);
                        else if (i % 4 == 2)
                            mane_color = m_lcd.color565(245, 160, 40);
                        else
                            mane_color = m_lcd.color565(255, 190, 60);
                    }
                    else
                    {
                        // Côtés: oranges plus standards
                        if (i % 4 == 0)
                            mane_color = m_lcd.color565(240, 140, 35);
                        else if (i % 4 == 1)
                            mane_color = m_lcd.color565(230, 130, 30);
                        else if (i % 4 == 2)
                            mane_color = m_lcd.color565(220, 120, 25);
                        else
                            mane_color = m_lcd.color565(235, 135, 32);
                    }
                }
                
                spr.fillTriangle(tip_x, tip_y, base_x1, base_y1, base_x2, base_y2, mane_color);
                
                // Contours sombres pour créer de la profondeur (surtout sur le haut)
                if (layer > 0 && (i % 5 == 0 || (is_top && i % 3 == 0)))
                {
                    spr.drawTriangle(tip_x, tip_y, base_x1, base_y1, base_x2, base_y2, m_lcd.color565(120, 60, 8));
                }
            }
        }
    }
    
    // Tête du lion PAR DESSUS la crinière (plus grosse, de face)
    spr.fillCircle(head_x, head_y, (int)(25 * scale), m_colOrange);
    spr.drawCircle(head_x, head_y, (int)(25 * scale), m_lcd.color565(200, 100, 0));
    
    // Museau (partie inférieure de la face)
    spr.fillEllipse(head_x, head_y + (int)(8 * scale), (int)(15 * scale), (int)(12 * scale), m_lcd.color565(255, 220, 180));
    
    // Yeux de lion (grands et intenses) - BIEN VISIBLES DE FACE
    int eye_y = head_y - (int)(5 * scale);
    int eye_spacing = (int)(12 * scale);
    
    // Blanc des yeux (légèrement rougi pour effet menaçant)
    spr.fillCircle(head_x - eye_spacing, eye_y, (int)(6 * scale), m_lcd.color565(255, 240, 235));
    spr.fillCircle(head_x + eye_spacing, eye_y, (int)(6 * scale), m_lcd.color565(255, 240, 235));
    
    // Iris orange/rouge intense
    spr.fillCircle(head_x - eye_spacing, eye_y, (int)(5 * scale), m_lcd.color565(255, 100, 0));
    spr.fillCircle(head_x + eye_spacing, eye_y, (int)(5 * scale), m_lcd.color565(255, 100, 0));
    
    // Pupilles verticales dilatées (comme un félin en colère) - plus larges et plus longues
    int pupil_height = (int)(8 * scale);
    int pupil_width = 3;
    spr.fillRect(head_x - eye_spacing - pupil_width/2, eye_y - pupil_height/2, pupil_width, pupil_height, m_colBlack);
    spr.fillRect(head_x + eye_spacing - pupil_width/2, eye_y - pupil_height/2, pupil_width, pupil_height, m_colBlack);
    
    // Sourcils menaçants - plus épais et anguleux
    int brow_thickness = 2;
    for (int i = 0; i < brow_thickness; i++)
    {
        // Sourcil gauche - angle descendant vers le nez
        spr.drawLine(head_x - eye_spacing - (int)(10 * scale), eye_y - (int)(9 * scale) + i, 
                     head_x - eye_spacing + (int)(4 * scale), eye_y - (int)(3 * scale) + i, m_colBlack);
        // Sourcil droit - angle descendant vers le nez
        spr.drawLine(head_x + eye_spacing + (int)(10 * scale), eye_y - (int)(9 * scale) + i, 
                     head_x + eye_spacing - (int)(4 * scale), eye_y - (int)(3 * scale) + i, m_colBlack);
    }
    
    // Nez du lion (triangle inversé)
    int nose_y = head_y + (int)(5 * scale);
    spr.fillTriangle(head_x, nose_y + (int)(4 * scale), 
                     head_x - (int)(3 * scale), nose_y, 
                     head_x + (int)(3 * scale), nose_y, 
                     m_colBlack);
    
    // Bouche ouverte qui rugit
    if (m_lion_roaring)
    {
        int mouth_y = head_y + (int)(15 * scale);
        int mouth_w = (int)(12 * scale);
        int mouth_h = (int)(8 * scale + sin(m_roar_timer * 10.0f) * 2.0f * scale);
        
        // Bouche ouverte (ellipse rouge foncé pour intérieur menaçant)
        spr.fillEllipse(head_x, mouth_y, mouth_w, mouth_h, m_lcd.color565(140, 20, 30));
        spr.drawEllipse(head_x, mouth_y, mouth_w, mouth_h, m_colBlack);
        
        // Langue rouge intense
        spr.fillEllipse(head_x, mouth_y + (int)(2 * scale), (int)(4 * scale), (int)(6 * scale), m_colRed);
        
        // Gencive supérieure
        int gum_top_y = mouth_y - mouth_h;
        spr.fillRect(head_x - mouth_w, gum_top_y - (int)(2 * scale), 
                     mouth_w * 2, (int)(2 * scale), m_lcd.color565(255, 180, 180));
        
        // Dents du haut - Canines (grandes dents pointues sur les côtés)
        int tooth_height = (int)(7 * scale);
        
        // Canine gauche (pointe vers le bas)
        spr.fillTriangle(
            head_x - (int)(9 * scale), gum_top_y + tooth_height,
            head_x - (int)(9 * scale) - (int)(3 * scale), gum_top_y,
            head_x - (int)(9 * scale) + (int)(3 * scale), gum_top_y,
            m_colWhite
        );
        spr.drawTriangle(
            head_x - (int)(9 * scale), gum_top_y + tooth_height,
            head_x - (int)(9 * scale) - (int)(3 * scale), gum_top_y,
            head_x - (int)(9 * scale) + (int)(3 * scale), gum_top_y,
            m_lcd.color565(220, 220, 220)
        );
        
        // Canine droite (pointe vers le bas)
        spr.fillTriangle(
            head_x + (int)(9 * scale), gum_top_y + tooth_height,
            head_x + (int)(9 * scale) - (int)(3 * scale), gum_top_y,
            head_x + (int)(9 * scale) + (int)(3 * scale), gum_top_y,
            m_colWhite
        );
        spr.drawTriangle(
            head_x + (int)(9 * scale), gum_top_y + tooth_height,
            head_x + (int)(9 * scale) - (int)(3 * scale), gum_top_y,
            head_x + (int)(9 * scale) + (int)(3 * scale), gum_top_y,
            m_lcd.color565(220, 220, 220)
        );
        
        // Incisives (petites dents au centre, pointes vers le bas)
        int incisor_height = (int)(4 * scale);
        for (int i = -1; i <= 1; i += 2)
        {
            spr.fillTriangle(
                head_x + i * (int)(3 * scale), gum_top_y + incisor_height,
                head_x + i * (int)(3 * scale) - (int)(2 * scale), gum_top_y,
                head_x + i * (int)(3 * scale) + (int)(2 * scale), gum_top_y,
                m_colWhite
            );
        }
        
        // Gencive inférieure
        int gum_bottom_y = mouth_y + mouth_h;
        spr.fillRect(head_x - mouth_w, gum_bottom_y, 
                     mouth_w * 2, (int)(2 * scale), m_lcd.color565(255, 180, 180));
        
        // Canines du bas (pointes vers le haut) - remontées pour être plus visibles
        int bottom_tooth_height = (int)(6 * scale); // Augmenté de 5 à 6
        int bottom_tooth_offset = (int)(4 * scale); // Remonter les dents
        
        // Canine gauche bas
        spr.fillTriangle(
            head_x - (int)(9 * scale), gum_bottom_y - bottom_tooth_offset,
            head_x - (int)(9 * scale) - (int)(3 * scale), gum_bottom_y + (int)(2 * scale) + bottom_tooth_height - bottom_tooth_offset,
            head_x - (int)(9 * scale) + (int)(3 * scale), gum_bottom_y + (int)(2 * scale) + bottom_tooth_height - bottom_tooth_offset,
            m_colWhite
        );
        spr.drawTriangle(
            head_x - (int)(9 * scale), gum_bottom_y - bottom_tooth_offset,
            head_x - (int)(9 * scale) - (int)(3 * scale), gum_bottom_y + (int)(2 * scale) + bottom_tooth_height - bottom_tooth_offset,
            head_x - (int)(9 * scale) + (int)(3 * scale), gum_bottom_y + (int)(2 * scale) + bottom_tooth_height - bottom_tooth_offset,
            m_lcd.color565(220, 220, 220)
        );
        
        // Canine droite bas
        spr.fillTriangle(
            head_x + (int)(9 * scale), gum_bottom_y - bottom_tooth_offset,
            head_x + (int)(9 * scale) - (int)(3 * scale), gum_bottom_y + (int)(2 * scale) + bottom_tooth_height - bottom_tooth_offset,
            head_x + (int)(9 * scale) + (int)(3 * scale), gum_bottom_y + (int)(2 * scale) + bottom_tooth_height - bottom_tooth_offset,
            m_colWhite
        );
        spr.drawTriangle(
            head_x + (int)(9 * scale), gum_bottom_y - bottom_tooth_offset,
            head_x + (int)(9 * scale) - (int)(3 * scale), gum_bottom_y + (int)(2 * scale) + bottom_tooth_height - bottom_tooth_offset,
            head_x + (int)(9 * scale) + (int)(3 * scale), gum_bottom_y + (int)(2 * scale) + bottom_tooth_height - bottom_tooth_offset,
            m_lcd.color565(220, 220, 220)
        );
    }
    else
    {
        // Bouche fermée (ligne légèrement courbée vers le bas pour un air menaçant)
        int mouth_y = head_y + (int)(14 * scale);
        int mouth_left = head_x - (int)(6 * scale);
        int mouth_right = head_x + (int)(6 * scale);
        
        // Bouche courbée vers le bas (V inversé léger)
        spr.drawLine(mouth_left, mouth_y, head_x, mouth_y + (int)(2 * scale), m_colBlack);
        spr.drawLine(head_x, mouth_y + (int)(2 * scale), mouth_right, mouth_y, m_colBlack);
        
        // Moustaches
        spr.drawLine(head_x - (int)(15 * scale), mouth_y - (int)(2 * scale), 
                     head_x - (int)(8 * scale), mouth_y, m_colBlack);
        spr.drawLine(head_x + (int)(15 * scale), mouth_y - (int)(2 * scale), 
                     head_x + (int)(8 * scale), mouth_y, m_colBlack);
    }
    
    // Effet de rugissement: lignes qui rayonnent depuis la bouche
    if (m_lion_roaring)
    {
        for (int i = 0; i < 12; i++)
        {
            // Rotation de -90° pour aligner avec la crinière
            float angle = (i / 12.0f) * 6.28f + m_roar_timer * 2.0f - 1.5708f;
            int line_len = (int)(15 + sin(m_roar_timer * 5.0f + i) * 8);
            int start_radius = (int)(30 * scale);
            int x1 = head_x + (int)(cos(angle) * start_radius);
            int y1 = head_y + (int)(sin(angle) * start_radius);
            int x2 = head_x + (int)(cos(angle) * (start_radius + line_len));
            int y2 = head_y + (int)(sin(angle) * (start_radius + line_len));
            
            spr.drawLine(x1, y1, x2, y2, m_colRed);
            spr.drawLine(x1, y1, x2, y2, m_colOrange);
        }
    }
}
