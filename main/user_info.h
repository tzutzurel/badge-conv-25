
#ifndef USER_INFO_H
#define USER_INFO_H

#include "qrcodegen.h"
#include <string>

struct UserInfo
{
    std::string nom;
    std::string prenom;
    std::string equipe;
    std::string ville;
    std::string poste;
    std::string accessBadgeToken;
};

// À personnaliser
constexpr UserInfo user_info = {
    "OPRESCU",      // nom
    "Ion",         // prénom
    "open|core",    // équipe
    "Lyon",         // ville
    "IOP Master", // poste
    "KGgpjiaGcVBU"  // accessBadgeToken
};

// Version QR code suffisante pour un token court (12 caractères)
#define QRCODE_VERSION_FOR_TOKEN 5
// Buffer pour le QR code (beaucoup plus petit que le max)
extern uint8_t g_qrcode[qrcodegen_BUFFER_LEN_FOR_VERSION(QRCODE_VERSION_FOR_TOKEN)];
extern int g_qrcode_size;

// Fonction pour générer le QR code à partir d'un texte
void user_info_generate_qrcode(const char *text);

#endif // USER_INFO_H
