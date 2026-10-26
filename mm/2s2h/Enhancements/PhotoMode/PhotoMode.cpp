// Modo Foto para 2 Ship 2 Harkinian.
// Congela el mundo, oculta el HUD y a Tatl, activa la camara libre y permite posar a Link:
// cualquier frame de cualquiera de sus animaciones, mas ajustes por hueso, expresion y giro.
// Tambien puede convertir a Link en un objeto (mascaras, armas, objetos clave) para tomas de detalle,
// moverlo libremente por el escenario y teletransportarse a cualquier escena.

#include "PhotoMode.h"
#include "PhotoModeAnims.h"

#include <libultraship/bridge/consolevariablebridge.h>
#include "2s2h/BenGui/UIWidgets.hpp"
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"
#include "2s2h/ShipUtils.h"

#include <cstring>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <set>
#include <nlohmann/json.hpp>
#include <ship/Context.h>
#include <ship/config/Config.h>
#include <imgui_internal.h>

extern "C" {
#include "z64.h"
#include "z64player.h"
#include "functions.h"
#include "variables.h"
#include "sys_matrix.h"
#include "gfx.h"
#include "objects/object_stk/object_stk.h"
AnimationHeaderCommon* ResourceMgr_LoadAnimByName(const char* path);
extern Gfx* gPlayerLeftHandOpenDLs[];
extern Gfx* gPlayerLeftHandClosedDLs[];
}

#include "PhotoModeScenes.h"

#define DEBUG_CAM_CVAR "gEnhancements.Camera.DebugCam.Enable"
#define DEBUG_CAM_SPEED_CVAR "gEnhancements.Camera.DebugCam.CameraSpeed"
#define DEBUG_CAM_VSPEED_CVAR "gEnhancements.Camera.DebugCam.VerticalSpeed"
#define PHOTO_FRAME_SIZE (sizeof(Vec3s) * PLAYER_LIMB_MAX + sizeof(s16))

namespace {

// ---- Objetos en los que se puede convertir a Link ----
#define PHOTO_OBJ_MAJORA -2

struct PhotoModeObject {
    const char* name;
    s16 gid;         // GID_* o PHOTO_OBJ_MAJORA
    float baseScale; // Escala que lo deja de un tamano parecido al de Link
};

const PhotoModeObject sObjects[] = {
    // Mascaras
    { "Mascara de Majora", PHOTO_OBJ_MAJORA, 0.03f },
    { "Mascara Deku", GID_MASK_DEKU, 0.6f },
    { "Mascara Goron", GID_MASK_GORON, 0.6f },
    { "Mascara Zora", GID_MASK_ZORA, 0.6f },
    { "Mascara de la Fiera Deidad", GID_MASK_FIERCE_DEITY, 0.6f },
    { "Mascara del Gigante", GID_MASK_GIANT, 0.6f },
    { "Mascara de la Verdad", GID_MASK_TRUTH, 0.6f },
    { "Mascara de Kafei", GID_MASK_KAFEIS_MASK, 0.6f },
    { "Mascara del Sol", GID_MASK_SUN, 0.6f },
    { "Mascara de los Amantes", GID_MASK_COUPLE, 0.6f },
    { "Mascara de Toda la Noche", GID_MASK_ALL_NIGHT, 0.6f },
    { "Capucha de Conejo", GID_MASK_BUNNY, 0.6f },
    { "Mascara de Keaton", GID_MASK_KEATON, 0.6f },
    { "Mascara de Garo", GID_MASK_GARO, 0.6f },
    { "Mascara de Romani", GID_MASK_ROMANI, 0.6f },
    { "Mascara del Lider de la Compania", GID_MASK_CIRCUS_LEADER, 0.6f },
    { "Gorra del Cartero", GID_MASK_POSTMAN, 0.6f },
    { "Mascara de la Gran Hada", GID_MASK_GREAT_FAIRY, 0.6f },
    { "Mascara de Gibdo", GID_MASK_GIBDO, 0.6f },
    { "Mascara de Don Gero", GID_MASK_DON_GERO, 0.6f },
    { "Mascara de Kamaro", GID_MASK_KAMARO, 0.6f },
    { "Sombrero del Capitan", GID_MASK_CAPTAIN, 0.6f },
    { "Mascara de Piedra", GID_MASK_STONE, 0.6f },
    { "Mascara de Bremen", GID_MASK_BREMEN, 0.6f },
    { "Mascara Explosiva", GID_MASK_BLAST, 0.6f },
    { "Mascara de los Aromas", GID_MASK_SCENTS, 0.6f },
    // Restos de los jefes
    { "Restos de Odolwa", GID_REMAINS_ODOLWA, 0.6f },
    { "Restos de Goht", GID_REMAINS_GOHT, 0.6f },
    { "Restos de Gyorg", GID_REMAINS_GYORG, 0.6f },
    { "Restos de Twinmold", GID_REMAINS_TWINMOLD, 0.6f },
    // Objetos y equipo
    { "Ocarina del Tiempo", GID_OCARINA, 0.6f },
    { "Espada Kokiri", GID_SWORD_KOKIRI, 0.6f },
    { "Espada Afilada", GID_SWORD_RAZOR, 0.6f },
    { "Espada Dorada", GID_SWORD_GILDED, 0.6f },
    { "Espada de la Gran Hada", GID_SWORD_GREAT_FAIRY, 0.6f },
    { "Escudo del Heroe", GID_SHIELD_HERO, 0.6f },
    { "Escudo Espejo", GID_SHIELD_MIRROR, 0.6f },
    { "Arco del Heroe", GID_BOW, 0.6f },
    { "Gancho", GID_HOOKSHOT, 0.6f },
    { "Lente de la Verdad", GID_LENS, 0.6f },
    { "Pictografo", GID_PICTOGRAPH_BOX, 0.6f },
    { "Bomba", GID_BOMB, 0.6f },
    { "Bombchu", GID_BOMBCHU, 0.6f },
    { "Barril de polvora", GID_POWDER_KEG, 0.6f },
    { "Botella", GID_BOTTLE, 0.6f },
    { "Lagrima de Luna", GID_MOONS_TEAR, 0.6f },
    { "Cuaderno de los Bombers", GID_BOMBERS_NOTEBOOK, 0.6f },
    { "Colgante de los Recuerdos", GID_PENDANT_OF_MEMORIES, 0.6f },
    { "Huevo Zora", GID_ZORA_EGG, 0.6f },
    { "Hada", GID_FAIRY, 0.6f },
    { "Pieza de corazon", GID_HEART_PIECE, 0.6f },
    { "Contenedor de corazon", GID_HEART_CONTAINER, 0.6f },
    { "Rupia verde", GID_RUPEE_GREEN, 0.6f },
    { "Rupia roja", GID_RUPEE_RED, 0.6f },
    { "Rupia enorme", GID_RUPEE_HUGE, 0.6f },
    { "Ficha de Skulltula dorada", GID_SKULL_TOKEN, 0.6f },
};
const int sObjectCount = (int)(sizeof(sObjects) / sizeof(sObjects[0]));

// ---- Estado ----
bool sEnabled = false;      // Interruptor general
bool sFreezeWorld = true;   // Congela actores y la hora
bool sHideHud = true;       // Oculta la interfaz
bool sHideTatl = true;      // Oculta a Tatl
bool sPoseActive = false;   // Aplica la pose personalizada a Link
float sWindowAlpha = 0.35f; // Opacidad del fondo de la ventana

u16 sFrozenTime = 0;
bool sTimeCaptured = false;

// Pose
PlayerAnimationFrame sBaseFrame; // Frame base (de una animacion o capturado)
bool sHasBaseFrame = false;
int sAnimIndex = -1;
int sAnimFrame = 0;
int sAnimFrameCount = 1;
float sLimbOffsets[PLAYER_LIMB_MAX][3] = {}; // Grados por hueso (X, Y, Z)
int sEyes = -1;                              // -1 = la que traiga la animacion
int sMouth = -1;                             // -1 = la que traiga la animacion
float sYawOffset = 0.0f;
s16 sBaseYaw = 0;
bool sHasBaseYaw = false;
char sFilter[64] = "";
int sCategory = 0; // 0 = todas, 1.. = sPhotoModeAnimCategories[i - 1]

// Objeto
bool sObjectMode = false;
int sObjectIndex = 0;
float sObjectSize = 1.0f;
float sObjectHeight = 30.0f;
float sObjectRot[3] = { 0.0f, 0.0f, 0.0f };
bool sObjectSpin = false;
float sObjectSpinAngle = 0.0f;

// Mover a Link
bool sMoveLink = false;
float sMoveSpeed = 0.5f;
Vec3f sSavedPos;
s16 sSavedYaw = 0;
bool sHasSavedPos = false;

// Equipo visible
bool sHideSword = false;
bool sHideSheath = false;
bool sHideShield = false;
bool sOpenHand = true; // Mano izquierda abierta (true) o cerrada (false) al quitar la espada
Gfx* sEmptyDLs[2 * PLAYER_FORM_MAX] = {};

// Copia de lo que se cambia solo durante el dibujo de Link (se restaura justo despues).
struct EquipBackup {
    bool active = false;
    Player* player = nullptr;
    u16 equipment = 0;
    s8 shield = 0;
    Gfx** leftHandDLists = nullptr;
    Gfx** sheathDLists = nullptr;
} sEquipBackup;

// Luna
bool sMoonAdjust = false;
bool sMoonHide = false;
float sMoonOffset[3] = { 0.0f, 0.0f, 0.0f }; // derecha, arriba, mas lejos (segun la camara al activar)
float sMoonScale = 1.0f;
float sMoonRot[3] = { 0.0f, 0.0f, 0.0f }; // giro, inclinacion, ladeo
s16 sMoonBasisYaw = 0;

struct MoonBackup {
    Actor* actor = nullptr;
    Vec3f pos;
    Vec3f scale;
    Vec3s rot;
} sMoonBackup;

// Fondo de color solido y limpieza de la escena
bool sChroma = false;
float sChromaColor[3] = { 0.0f, 1.0f, 0.0f };
bool sChromaOnlyLink = true;
bool sHideNpcs = false;
bool sHideEnemies = false;
bool sHideParticles = false;

// Objeto en la mano (0 = izquierda, 1 = derecha)
struct HandObject {
    bool on = false;
    int index = 0;
    float size = 1.0f;
    float offset[3] = { 0.0f, 0.0f, 0.0f };
    float rot[3] = { 0.0f, 0.0f, 0.0f };
};
HandObject sHand[2];
MtxF sHandMtx[2];
bool sHandMtxValid[2] = { false, false };

// Tamano de Link
float sLinkScale = 1.0f;
struct ScaleBackup {
    Actor* actor = nullptr;
    Vec3f scale;
} sScaleBackup;

// Lente y guias de encuadre
bool sFovOverride = false;
float sFov = 60.0f;
bool sGuides = false;
bool sGuideThirds = true;
bool sGuideCenter = true;
bool sGuideBadge = true;

// Hora y dia
int sDayChoice = 0; // 0 = todavia no se ha leido el dia de la partida

// Favoritas y escenas guardadas (se guardan en el archivo de configuracion de 2Ship)
std::set<std::string> sFavorites;
bool sOnlyFavorites = false;
nlohmann::json sSavedScenes = nlohmann::json::array();
bool sConfigLoaded = false;
char sSceneName[64] = "";
int sSavedSceneIndex = -1;
int sPendingLoad = -1;
int sPendingDelay = 0;
std::string sSceneMessage;

// Teletransporte
char sSceneFilter[64] = "";
int sRegion = 0; // 0 = todas, 1.. = sPhotoModeRegions[i - 1]
int sSceneIndex = -1;

const char* sLimbNames[PLAYER_LIMB_MAX] = {
    "(posicion raiz)", "Raiz",          "Cintura",           "Raiz inferior",    "Muslo derecho",
    "Pierna derecha",  "Pie derecho",   "Muslo izquierdo",   "Pierna izquierda", "Pie izquierdo",
    "Raiz superior",   "Cabeza",        "Gorro",             "Cuello",           "Hombro izquierdo",
    "Antebrazo izquierdo", "Mano izquierda", "Hombro derecho", "Antebrazo derecho", "Mano derecha",
    "Funda",           "Torso",
};

const char* sEyeNames[] = { "De la animacion",   "Abiertos",       "Entrecerrados", "Cerrados", "Mirando derecha",
                            "Mirando izquierda", "Mirando arriba", "Mirando abajo", "Con dolor" };
const char* sMouthNames[] = { "De la animacion", "Cerrada", "Entreabierta", "Abierta", "Sonrisa" };

s16 DegToBinang(float deg) {
    return (s16)(deg * (32768.0f / 180.0f));
}

Player* GetPlayer() {
    if (gPlayState == nullptr) {
        return nullptr;
    }
    return GET_PLAYER(gPlayState);
}

// Carga un frame de una animacion de Link directamente desde los recursos del juego.
bool LoadAnimFrame(int animIndex, int frame) {
    if (animIndex < 0 || animIndex >= (int)PHOTO_MODE_ANIM_COUNT) {
        return false;
    }

    PlayerAnimationHeader* anim = (PlayerAnimationHeader*)ResourceMgr_LoadAnimByName(sPhotoModeAnims[animIndex].path);
    if (anim == nullptr || anim->segmentVoid == nullptr) {
        return false;
    }

    sAnimFrameCount = std::max<int>(1, anim->common.frameCount);
    frame = std::clamp(frame, 0, sAnimFrameCount - 1);
    sAnimFrame = frame;

    const u8* data = (const u8*)anim->segmentVoid;
    memcpy(&sBaseFrame, data + PHOTO_FRAME_SIZE * frame, PHOTO_FRAME_SIZE);
    sHasBaseFrame = true;
    return true;
}

// Copia la pose que Link tiene en este momento como base.
void CaptureCurrentPose() {
    Player* player = GetPlayer();
    if (player == nullptr || player->skelAnime.jointTable == nullptr) {
        return;
    }
    memcpy(&sBaseFrame, player->skelAnime.jointTable, PHOTO_FRAME_SIZE);
    sHasBaseFrame = true;
    sAnimIndex = -1;
}

void EnablePose() {
    if (sPoseActive) {
        return;
    }
    sPoseActive = true;
    if (!sHasBaseFrame) {
        CaptureCurrentPose();
    }
    Player* player = GetPlayer();
    if (player != nullptr) {
        sBaseYaw = player->actor.shape.rot.y;
        sHasBaseYaw = true;
    }
}

void ResetLimbs() {
    memset(sLimbOffsets, 0, sizeof(sLimbOffsets));
    sEyes = -1;
    sMouth = -1;
    sYawOffset = 0.0f;
}

void ApplyPose(Player* player) {
    Vec3s* jointTable = player->skelAnime.jointTable;
    if (jointTable == nullptr || !sHasBaseFrame) {
        return;
    }

    memcpy(jointTable, &sBaseFrame, PHOTO_FRAME_SIZE);

    for (int i = 1; i < PLAYER_LIMB_MAX; i++) {
        jointTable[i].x += DegToBinang(sLimbOffsets[i][0]);
        jointTable[i].y += DegToBinang(sLimbOffsets[i][1]);
        jointTable[i].z += DegToBinang(sLimbOffsets[i][2]);
    }

    s16* appearance = &((PlayerAnimationFrame*)jointTable)->appearanceInfo;
    if (sEyes >= 0) {
        *appearance = (s16)((*appearance & ~0x000F) | ((sEyes + 1) & 0xF));
    }
    if (sMouth >= 0) {
        *appearance = (s16)((*appearance & ~0x00F0) | (((sMouth + 1) & 0xF) << 4));
    }

    if (sHasBaseYaw) {
        player->actor.shape.rot.y = (s16)(sBaseYaw + DegToBinang(sYawOffset));
    }
}

// Dibuja el objeto elegido en la posicion de Link.
void DrawObject(Player* player) {
    const PhotoModeObject& obj = sObjects[std::clamp(sObjectIndex, 0, sObjectCount - 1)];

    if (sObjectSpin) {
        sObjectSpinAngle += 1.5f;
        if (sObjectSpinAngle >= 360.0f) {
            sObjectSpinAngle -= 360.0f;
        }
    }

    Vec3s rot;
    rot.x = DegToBinang(sObjectRot[0]);
    rot.y = (s16)(player->actor.shape.rot.y + DegToBinang(sObjectRot[1] + (sObjectSpin ? sObjectSpinAngle : 0.0f)));
    rot.z = DegToBinang(sObjectRot[2]);

    float scale = obj.baseScale * sObjectSize;

    Matrix_Push();
    Matrix_SetTranslateRotateYXZ(player->actor.world.pos.x, player->actor.world.pos.y + sObjectHeight,
                                 player->actor.world.pos.z, &rot);
    Matrix_Scale(scale, scale, scale, MTXMODE_APPLY);
    if (obj.gid == PHOTO_OBJ_MAJORA) {
        Gfx_DrawDListOpa(gPlayState, (Gfx*)gSkullKidMajorasMask1DL);
    } else {
        GetItem_Draw(gPlayState, obj.gid);
    }
    Matrix_Pop();
}

// Quita espada / funda / escudo solo mientras se dibuja a Link. La partida no cambia: la espada vuelve a
// "equiparse" en cuanto termina el dibujo, antes de que el juego actualice o guarde nada.
void RestoreEquip() {
    if (!sEquipBackup.active) {
        return;
    }
    gSaveContext.save.saveInfo.equips.equipment = sEquipBackup.equipment;
    if (sEquipBackup.player != nullptr) {
        sEquipBackup.player->currentShield = sEquipBackup.shield;
        sEquipBackup.player->leftHandDLists = sEquipBackup.leftHandDLists;
        sEquipBackup.player->sheathDLists = sEquipBackup.sheathDLists;
    }
    sEquipBackup.active = false;
}

void ApplyEquipHide(Player* player) {
    RestoreEquip();
    if (!sHideSword && !sHideShield) {
        return;
    }
    sEquipBackup.active = true;
    sEquipBackup.player = player;
    sEquipBackup.equipment = gSaveContext.save.saveInfo.equips.equipment;
    sEquipBackup.shield = player->currentShield;
    sEquipBackup.leftHandDLists = player->leftHandDLists;
    sEquipBackup.sheathDLists = player->sheathDLists;

    if (sHideSword) {
        SET_EQUIP_VALUE(EQUIP_TYPE_SWORD, EQUIP_VALUE_SWORD_NONE);
        if (player->leftHandType == PLAYER_MODELTYPE_LH_ONE_HAND_SWORD ||
            player->leftHandType == PLAYER_MODELTYPE_LH_TWO_HAND_SWORD) {
            Gfx** hands = sOpenHand ? gPlayerLeftHandOpenDLs : gPlayerLeftHandClosedDLs;
            player->leftHandDLists = &hands[player->transformation * 2];
        }
        if (sHideSheath) {
            player->sheathDLists = sEmptyDLs;
        }
    }
    if (sHideShield) {
        player->currentShield = PLAYER_SHIELD_NONE;
    }
}

// La luna es el actor En_Fall. Sus tipos 3, 4, 8 y 11 son efectos (bolas de fuego, escombros, lagrima, anillo).
bool IsMoonActor(Actor* actor) {
    if (actor->id != ACTOR_EN_FALL) {
        return false;
    }
    int type = (actor->params & 0xF80) >> 7;
    return type != 3 && type != 4 && type != 8 && type != 11;
}

Actor* FindMoon() {
    if (gPlayState == nullptr) {
        return nullptr;
    }
    for (Actor* a = gPlayState->actorCtx.actorLists[ACTORCAT_ITEMACTION].first; a != nullptr; a = a->next) {
        if (IsMoonActor(a)) {
            return a;
        }
    }
    return nullptr;
}

void CaptureMoonBasis() {
    Camera* cam = (gPlayState != nullptr) ? GET_ACTIVE_CAM(gPlayState) : nullptr;
    if (cam != nullptr) {
        sMoonBasisYaw = Math_Vec3f_Yaw(&cam->eye, &cam->at);
    }
}

// Mueve / escala / gira la luna solo durante su dibujo. Actor_Draw ya calculo la matriz, asi que se rehace.
void ApplyMoonAdjust(Actor* moon) {
    sMoonBackup.actor = moon;
    sMoonBackup.pos = moon->world.pos;
    sMoonBackup.scale = moon->scale;
    sMoonBackup.rot = moon->shape.rot;

    float fx = Math_SinS(sMoonBasisYaw);
    float fz = Math_CosS(sMoonBasisYaw);
    moon->world.pos.x += -fz * sMoonOffset[0] + fx * sMoonOffset[2];
    moon->world.pos.z += fx * sMoonOffset[0] + fz * sMoonOffset[2];
    moon->world.pos.y += sMoonOffset[1];
    moon->scale.x *= sMoonScale;
    moon->scale.y *= sMoonScale;
    moon->scale.z *= sMoonScale;
    moon->shape.rot.y += DegToBinang(sMoonRot[0]);
    moon->shape.rot.x += DegToBinang(sMoonRot[1]);
    moon->shape.rot.z += DegToBinang(sMoonRot[2]);

    Matrix_SetTranslateRotateYXZ(moon->world.pos.x, moon->world.pos.y + (moon->shape.yOffset * moon->scale.y),
                                 moon->world.pos.z, &moon->shape.rot);
    Matrix_Scale(moon->scale.x, moon->scale.y, moon->scale.z, MTXMODE_APPLY);
}

void RestoreMoon() {
    if (sMoonBackup.actor == nullptr) {
        return;
    }
    sMoonBackup.actor->world.pos = sMoonBackup.pos;
    sMoonBackup.actor->scale = sMoonBackup.scale;
    sMoonBackup.actor->shape.rot = sMoonBackup.rot;
    sMoonBackup.actor = nullptr;
}

// ---- Ocultar actores ----
bool IsHiddenByPhotoMode(Actor* actor) {
    if (!sEnabled || gPlayState == nullptr) {
        return false;
    }
    Player* player = GET_PLAYER(gPlayState);
    if (player != nullptr && actor == &player->actor) {
        return false;
    }
    bool onlyLink = sChroma && sChromaOnlyLink;
    if (IsMoonActor(actor)) {
        return sMoonHide || onlyLink;
    }
    if (player != nullptr && actor == player->tatlActor) {
        return sHideTatl || onlyLink;
    }
    if (onlyLink) {
        return true;
    }
    if (sHideNpcs && actor->category == ACTORCAT_NPC) {
        return true;
    }
    if (sHideEnemies && (actor->category == ACTORCAT_ENEMY || actor->category == ACTORCAT_BOSS)) {
        return true;
    }
    return false;
}

// ---- Tamano de Link (solo al dibujar; se rehace la matriz que ya calculo Actor_Draw) ----
void ApplyLinkScale(Player* player) {
    if (std::fabs(sLinkScale - 1.0f) < 0.001f) {
        return;
    }
    Actor* a = &player->actor;
    sScaleBackup.actor = a;
    sScaleBackup.scale = a->scale;
    a->scale.x *= sLinkScale;
    a->scale.y *= sLinkScale;
    a->scale.z *= sLinkScale;
    Matrix_SetTranslateRotateYXZ(a->world.pos.x, a->world.pos.y + (a->shape.yOffset * a->scale.y), a->world.pos.z,
                                 &a->shape.rot);
    Matrix_Scale(a->scale.x, a->scale.y, a->scale.z, MTXMODE_APPLY);
}

void RestoreLinkScale() {
    if (sScaleBackup.actor == nullptr) {
        return;
    }
    sScaleBackup.actor->scale = sScaleBackup.scale;
    sScaleBackup.actor = nullptr;
}

// ---- Objeto en la mano ----
// Durante el dibujo de Link solo se guarda la matriz de cada mano. El objeto se dibuja despues de Link,
// para no cambiarle el estado grafico al resto del cuerpo (colores de la tunica, etc.).
// Escala: en el juego un objeto sostenido se dibuja a 0.2 en el mundo; dentro de la mano (Link mide 0.01) es x20.
void DrawHandObject(int h) {
    const HandObject& ho = sHand[h];
    const PhotoModeObject& obj = sObjects[std::clamp(ho.index, 0, sObjectCount - 1)];
    float scale = obj.baseScale * (20.0f / 0.6f) * ho.size;

    Matrix_Push();
    Matrix_Put(&sHandMtx[h]);
    Matrix_Translate(ho.offset[0], ho.offset[1], ho.offset[2], MTXMODE_APPLY);
    Matrix_RotateZYX(DegToBinang(ho.rot[0]), DegToBinang(ho.rot[1]), DegToBinang(ho.rot[2]), MTXMODE_APPLY);
    Matrix_Scale(scale, scale, scale, MTXMODE_APPLY);
    if (obj.gid == PHOTO_OBJ_MAJORA) {
        Gfx_DrawDListOpa(gPlayState, (Gfx*)gSkullKidMajorasMask1DL);
    } else {
        GetItem_Draw(gPlayState, obj.gid);
    }
    Matrix_Pop();
}

// ---- Configuracion guardada (favoritas y escenas) ----
void LoadPhotoConfig() {
    if (sConfigLoaded) {
        return;
    }
    sConfigLoaded = true;
    auto all = Ship::Context::GetRawInstance()->GetConfig()->GetNestedJson();
    if (!all.contains("PhotoMode") || !all["PhotoMode"].is_object()) {
        return;
    }
    auto pm = all["PhotoMode"];
    if (pm.contains("favorites") && pm["favorites"].is_array()) {
        for (auto& f : pm["favorites"]) {
            if (f.is_string()) {
                sFavorites.insert(f.get<std::string>());
            }
        }
    }
    if (pm.contains("scenes") && pm["scenes"].is_array()) {
        sSavedScenes = pm["scenes"];
    }
}

void SavePhotoConfig() {
    nlohmann::json pm;
    pm["favorites"] = std::vector<std::string>(sFavorites.begin(), sFavorites.end());
    pm["scenes"] = sSavedScenes;
    auto config = Ship::Context::GetRawInstance()->GetConfig();
    config->SetBlock("PhotoMode", pm);
    config->Save();
}

void SetDebugCam(bool on) {
    CVarSetInteger(DEBUG_CAM_CVAR, on ? 1 : 0);
    ShipInit::Init(DEBUG_CAM_CVAR);
}

// Mueve a Link con el mando mientras "Mover a Link" esta activo. Link esta congelado, asi que solo se cambia
// su posicion: no hay colisiones ni gravedad, puede quedar flotando o dentro de una pared a proposito.
void MoveLinkUpdate() {
    Player* player = GetPlayer();
    if (player == nullptr || gPlayState->pauseCtx.state != PAUSE_STATE_OFF) {
        return;
    }
    Input* input = &gPlayState->state.input[0];
    float speed = sMoveSpeed * 20.0f;
    if (CHECK_BTN_ANY(input->cur.button, BTN_A | BTN_B | BTN_L)) {
        speed *= 3.0f;
    }

    // Direccion segun hacia donde mira la camara, para que "arriba" en el stick sea "alejarse de la camara".
    Camera* cam = GET_ACTIVE_CAM(gPlayState);
    s16 camYaw = (cam != nullptr) ? Math_Vec3f_Yaw(&cam->eye, &cam->at) : player->actor.shape.rot.y;
    float fx = Math_SinS(camYaw);
    float fz = Math_CosS(camYaw);
    float rx = -fz; // derecha = adelante girado -90 grados
    float rz = fx;

    float sx = input->cur.stick_x / 60.0f;
    float sy = input->cur.stick_y / 60.0f;
    if (fabsf(sx) < 0.1f) {
        sx = 0.0f;
    }
    if (fabsf(sy) < 0.1f) {
        sy = 0.0f;
    }

    Vec3f* pos = &player->actor.world.pos;
    pos->x += (fx * sy + rx * sx) * speed;
    pos->z += (fz * sy + rz * sx) * speed;
    pos->y += (CHECK_BTN_ANY(input->cur.button, BTN_Z) - CHECK_BTN_ANY(input->cur.button, BTN_R)) * speed * 0.5f;

    // Girar a Link con C izquierda / C derecha.
    s16 turn = (s16)((CHECK_BTN_ANY(input->cur.button, BTN_CLEFT) - CHECK_BTN_ANY(input->cur.button, BTN_CRIGHT)) *
                     0x200 * (CHECK_BTN_ANY(input->cur.button, BTN_A | BTN_B | BTN_L) ? 3 : 1));
    if (turn != 0) {
        if (sPoseActive && sHasBaseYaw) {
            sBaseYaw += turn;
        } else {
            player->actor.shape.rot.y += turn;
            player->actor.world.rot.y = player->actor.shape.rot.y;
        }
    }

    player->actor.prevPos = *pos;
    player->actor.home.pos = *pos;
}

void StartMoveLink() {
    Player* player = GetPlayer();
    if (player == nullptr) {
        return;
    }
    if (!sHasSavedPos) {
        sSavedPos = player->actor.world.pos;
        sSavedYaw = player->actor.shape.rot.y;
        sHasSavedPos = true;
    }
}

void RestoreLinkPosition() {
    Player* player = GetPlayer();
    if (player == nullptr || !sHasSavedPos) {
        return;
    }
    player->actor.world.pos = sSavedPos;
    player->actor.prevPos = sSavedPos;
    player->actor.shape.rot.y = sSavedYaw;
    player->actor.world.rot.y = sSavedYaw;
    if (sPoseActive) {
        sBaseYaw = sSavedYaw;
    }
}

void TeleportTo(s32 entrance) {
    if (gPlayState == nullptr) {
        return;
    }
    sMoveLink = false;
    sHasSavedPos = false;
    sPoseActive = false;
    sHasBaseYaw = false;
    gPlayState->nextEntrance = entrance;
    gPlayState->transitionTrigger = TRANS_TRIGGER_START;
    gPlayState->transitionType = TRANS_TYPE_FADE_BLACK_FAST;
    gSaveContext.nextTransitionType = TRANS_TYPE_FADE_BLACK_FAST;
}

// ---- Escenas guardadas ----
nlohmann::json Vec3ToJson(const Vec3f& v) {
    return nlohmann::json::array({ v.x, v.y, v.z });
}

Vec3f JsonToVec3(const nlohmann::json& j) {
    Vec3f v = { 0.0f, 0.0f, 0.0f };
    if (j.is_array() && j.size() == 3) {
        v.x = j[0].get<float>();
        v.y = j[1].get<float>();
        v.z = j[2].get<float>();
    }
    return v;
}

template <size_t N> nlohmann::json ArrToJson(const float (&a)[N]) {
    return std::vector<float>(a, a + N);
}

template <size_t N> void JsonToArr(const nlohmann::json& j, float (&a)[N]) {
    if (j.is_array() && j.size() == N) {
        for (size_t i = 0; i < N; i++) {
            a[i] = j[i].get<float>();
        }
    }
}

int FindObjectByName(const std::string& name) {
    for (int i = 0; i < sObjectCount; i++) {
        if (name == sObjects[i].name) {
            return i;
        }
    }
    return 0;
}

int FindAnimByName(const std::string& name) {
    for (int i = 0; i < (int)PHOTO_MODE_ANIM_COUNT; i++) {
        if (name == sPhotoModeAnims[i].name) {
            return i;
        }
    }
    return -1;
}

nlohmann::json BuildScene(const std::string& name) {
    nlohmann::json j;
    Player* player = GetPlayer();
    j["name"] = name;
    j["sceneId"] = gPlayState->sceneId;
    j["sceneName"] = Ship_GetSceneName(gPlayState->sceneId);
    j["entrance"] = gSaveContext.save.entrance;
    j["day"] = (int)CURRENT_DAY;
    j["time"] = (int)gSaveContext.save.time;

    if (player != nullptr) {
        j["pos"] = Vec3ToJson(player->actor.world.pos);
        j["rotY"] = player->actor.shape.rot.y;
    }

    // Pose
    j["pose"] = sPoseActive;
    j["animName"] = (sAnimIndex >= 0) ? sPhotoModeAnims[sAnimIndex].name : "";
    j["frame"] = sAnimFrame;
    if (sHasBaseFrame) {
        const s16* raw = (const s16*)&sBaseFrame;
        j["baseFrame"] = std::vector<int>(raw, raw + PHOTO_FRAME_SIZE / sizeof(s16));
    }
    std::vector<float> limbs;
    for (int i = 0; i < PLAYER_LIMB_MAX; i++) {
        limbs.insert(limbs.end(), sLimbOffsets[i], sLimbOffsets[i] + 3);
    }
    j["limbs"] = limbs;
    j["eyes"] = sEyes;
    j["mouth"] = sMouth;
    j["yawOffset"] = sYawOffset;
    j["baseYaw"] = sBaseYaw;

    // Equipo, tamano y objetos
    j["hideSword"] = sHideSword;
    j["hideSheath"] = sHideSheath;
    j["hideShield"] = sHideShield;
    j["openHand"] = sOpenHand;
    j["linkScale"] = sLinkScale;
    j["objectMode"] = sObjectMode;
    j["object"] = sObjects[sObjectIndex].name;
    j["objectSize"] = sObjectSize;
    j["objectHeight"] = sObjectHeight;
    j["objectRot"] = ArrToJson(sObjectRot);
    j["objectSpin"] = sObjectSpin;
    nlohmann::json hands = nlohmann::json::array();
    for (int h = 0; h < 2; h++) {
        hands.push_back({ { "on", sHand[h].on },
                          { "object", sObjects[sHand[h].index].name },
                          { "size", sHand[h].size },
                          { "offset", ArrToJson(sHand[h].offset) },
                          { "rot", ArrToJson(sHand[h].rot) } });
    }
    j["hands"] = hands;

    // Mundo, fondo, luna y lente
    j["freeze"] = sFreezeWorld;
    j["hideHud"] = sHideHud;
    j["hideTatl"] = sHideTatl;
    j["hideNpcs"] = sHideNpcs;
    j["hideEnemies"] = sHideEnemies;
    j["hideParticles"] = sHideParticles;
    j["chroma"] = sChroma;
    j["chromaColor"] = ArrToJson(sChromaColor);
    j["chromaOnlyLink"] = sChromaOnlyLink;
    j["moonHide"] = sMoonHide;
    j["moonAdjust"] = sMoonAdjust;
    j["moonOffset"] = ArrToJson(sMoonOffset);
    j["moonScale"] = sMoonScale;
    j["moonRot"] = ArrToJson(sMoonRot);
    j["moonBasisYaw"] = sMoonBasisYaw;
    j["fovOverride"] = sFovOverride;
    j["fov"] = sFov;

    // Camara
    bool debugCam = CVarGetInteger(DEBUG_CAM_CVAR, 0) != 0;
    j["debugCam"] = debugCam;
    Camera* cam = GET_ACTIVE_CAM(gPlayState);
    if (cam != nullptr) {
        j["camEye"] = Vec3ToJson(cam->eye);
        j["camAt"] = Vec3ToJson(cam->at);
        j["camRoll"] = cam->roll;
    }
    return j;
}

template <typename T> void Get(const nlohmann::json& j, const char* key, T& out) {
    if (j.contains(key) && !j[key].is_null()) {
        out = j[key].get<T>();
    }
}

void ApplyScene(const nlohmann::json& j) {
    Player* player = GetPlayer();
    if (player == nullptr) {
        return;
    }

    // Hora
    if (j.contains("time")) {
        u16 t = (u16)j["time"].get<int>();
        gSaveContext.save.time = t;
        sFrozenTime = t;
        sTimeCaptured = sFreezeWorld;
    }

    // Opciones generales
    Get(j, "freeze", sFreezeWorld);
    Get(j, "hideHud", sHideHud);
    Get(j, "hideTatl", sHideTatl);
    Get(j, "hideNpcs", sHideNpcs);
    Get(j, "hideEnemies", sHideEnemies);
    Get(j, "hideParticles", sHideParticles);
    Get(j, "chroma", sChroma);
    JsonToArr(j.value("chromaColor", nlohmann::json()), sChromaColor);
    Get(j, "chromaOnlyLink", sChromaOnlyLink);
    Get(j, "moonHide", sMoonHide);
    Get(j, "moonAdjust", sMoonAdjust);
    JsonToArr(j.value("moonOffset", nlohmann::json()), sMoonOffset);
    Get(j, "moonScale", sMoonScale);
    JsonToArr(j.value("moonRot", nlohmann::json()), sMoonRot);
    Get(j, "moonBasisYaw", sMoonBasisYaw);
    Get(j, "fovOverride", sFovOverride);
    Get(j, "fov", sFov);

    // Equipo, tamano y objetos
    Get(j, "hideSword", sHideSword);
    Get(j, "hideSheath", sHideSheath);
    Get(j, "hideShield", sHideShield);
    Get(j, "openHand", sOpenHand);
    Get(j, "linkScale", sLinkScale);
    Get(j, "objectMode", sObjectMode);
    sObjectIndex = FindObjectByName(j.value("object", std::string()));
    Get(j, "objectSize", sObjectSize);
    Get(j, "objectHeight", sObjectHeight);
    JsonToArr(j.value("objectRot", nlohmann::json()), sObjectRot);
    Get(j, "objectSpin", sObjectSpin);
    if (j.contains("hands") && j["hands"].is_array()) {
        for (int h = 0; h < 2 && h < (int)j["hands"].size(); h++) {
            const auto& hj = j["hands"][h];
            Get(hj, "on", sHand[h].on);
            sHand[h].index = FindObjectByName(hj.value("object", std::string()));
            Get(hj, "size", sHand[h].size);
            JsonToArr(hj.value("offset", nlohmann::json()), sHand[h].offset);
            JsonToArr(hj.value("rot", nlohmann::json()), sHand[h].rot);
        }
    }

    // Posicion de Link
    if (j.contains("pos")) {
        player->actor.world.pos = JsonToVec3(j["pos"]);
        player->actor.prevPos = player->actor.world.pos;
        player->actor.home.pos = player->actor.world.pos;
    }
    if (j.contains("rotY")) {
        player->actor.shape.rot.y = j["rotY"].get<s16>();
        player->actor.world.rot.y = player->actor.shape.rot.y;
    }

    // Pose
    std::string animName = j.value("animName", std::string());
    int animIndex = animName.empty() ? -1 : FindAnimByName(animName);
    if (animIndex >= 0) {
        sAnimIndex = animIndex;
        LoadAnimFrame(animIndex, j.value("frame", 0));
    } else if (j.contains("baseFrame") && j["baseFrame"].is_array() &&
               j["baseFrame"].size() == PHOTO_FRAME_SIZE / sizeof(s16)) {
        s16* raw = (s16*)&sBaseFrame;
        for (size_t i = 0; i < j["baseFrame"].size(); i++) {
            raw[i] = (s16)j["baseFrame"][i].get<int>();
        }
        sHasBaseFrame = true;
        sAnimIndex = -1;
    }
    if (j.contains("limbs") && j["limbs"].is_array() && j["limbs"].size() == PLAYER_LIMB_MAX * 3) {
        for (int i = 0; i < PLAYER_LIMB_MAX * 3; i++) {
            sLimbOffsets[i / 3][i % 3] = j["limbs"][i].get<float>();
        }
    }
    Get(j, "eyes", sEyes);
    Get(j, "mouth", sMouth);
    Get(j, "yawOffset", sYawOffset);
    bool pose = false;
    Get(j, "pose", pose);
    sPoseActive = pose && sHasBaseFrame;
    if (sPoseActive) {
        sBaseYaw = j.value("baseYaw", (int)player->actor.shape.rot.y);
        sHasBaseYaw = true;
    }

    // Camara
    bool debugCam = false;
    Get(j, "debugCam", debugCam);
    if (debugCam) {
        if (CVarGetInteger(DEBUG_CAM_CVAR, 0) == 0) {
            SetDebugCam(true);
        }
        Camera* cam = GET_ACTIVE_CAM(gPlayState);
        if (cam != nullptr && j.contains("camEye") && j.contains("camAt")) {
            cam->eye = JsonToVec3(j["camEye"]);
            cam->eyeNext = cam->eye;
            cam->at = JsonToVec3(j["camAt"]);
            cam->roll = j.value("camRoll", 0);
        }
    }
    sSceneMessage = "Escena cargada: " + j.value("name", std::string());
}

void LoadSavedScene(int index) {
    if (index < 0 || index >= (int)sSavedScenes.size() || gPlayState == nullptr) {
        return;
    }
    const auto& j = sSavedScenes[index];
    int sceneId = j.value("sceneId", -1);
    int day = j.value("day", (int)CURRENT_DAY);
    if (sceneId != gPlayState->sceneId || day != (int)CURRENT_DAY) {
        // Primero hay que ir a la escena (y al dia) donde se guardo; se termina de cargar al llegar.
        gSaveContext.save.day = day;
        TeleportTo(j.value("entrance", (int)gSaveContext.save.entrance));
        sPendingLoad = index;
        sPendingDelay = 20;
        sSceneMessage = "Viajando a la escena guardada...";
        return;
    }
    ApplyScene(j);
}

void RegisterPhotoMode() {
    // Congelar actores. Con una pose o un objeto activo, Link siempre se congela.
    COND_HOOK(ShouldActorUpdate, true, [](Actor* actor, bool* should) {
        if (!sEnabled) {
            return;
        }
        if (sFreezeWorld) {
            *should = false;
        } else if ((sPoseActive || sObjectMode || sMoveLink) && actor->id == ACTOR_PLAYER) {
            *should = false;
        }
    });

    // Aplicar la pose (o dibujar el objeto) justo antes de dibujar a Link, y ocultar a Tatl.
    COND_HOOK(ShouldActorDraw, true, [](Actor* actor, bool* should) {
        if (!sEnabled) {
            return;
        }
        if (IsHiddenByPhotoMode(actor)) {
            *should = false;
            return;
        }
        if (IsMoonActor(actor)) {
            if (sMoonAdjust) {
                ApplyMoonAdjust(actor);
            }
            return;
        }
        Player* player = GetPlayer();
        if (player == nullptr || actor != &player->actor) {
            return;
        }
        sHandMtxValid[0] = sHandMtxValid[1] = false;
        if (sObjectMode) {
            DrawObject(player);
            *should = false;
        } else {
            if (sPoseActive) {
                ApplyPose(player);
            }
            ApplyEquipHide(player);
            ApplyLinkScale(player);
        }
    });

    // Guardar la matriz de cada mano mientras se dibuja el cuerpo de Link.
    COND_HOOK(OnPlayerPostLimbDraw, true, [](Player* player, s32 limbIndex) {
        if (!sEnabled) {
            return;
        }
        int h = (limbIndex == PLAYER_LIMB_LEFT_HAND) ? 0 : (limbIndex == PLAYER_LIMB_RIGHT_HAND) ? 1 : -1;
        if (h >= 0 && sHand[h].on) {
            Matrix_Get(&sHandMtx[h]);
            sHandMtxValid[h] = true;
        }
    });

    // Justo despues de dibujar a Link se le devuelve el equipo.
    COND_HOOK(OnActorDraw, true, [](Actor* actor) {
        if (sEquipBackup.active && actor == &sEquipBackup.player->actor) {
            RestoreEquip();
        }
        if (actor == sScaleBackup.actor) {
            RestoreLinkScale();
        }
        if (sEnabled && gPlayState != nullptr && actor == &GET_PLAYER(gPlayState)->actor) {
            for (int h = 0; h < 2; h++) {
                if (sHand[h].on && sHandMtxValid[h]) {
                    DrawHandObject(h);
                }
            }
        }
        if (actor == sMoonBackup.actor) {
            RestoreMoon();
        }
    });

    // Detener la hora mientras el mundo esta congelado.
    COND_HOOK(OnGameStateUpdate, true, []() {
        RestoreEquip(); // seguridad: nunca dejar el equipo cambiado durante la logica del juego
        RestoreMoon();
        RestoreLinkScale();
        // Terminar de cargar una escena guardada cuando ya se llego a su escenario.
        if (sPendingLoad >= 0 && gPlayState != nullptr && gPlayState->transitionTrigger == TRANS_TRIGGER_OFF &&
            gPlayState->transitionMode == TRANS_MODE_OFF && GetPlayer() != nullptr) {
            if (--sPendingDelay <= 0) {
                int index = sPendingLoad;
                sPendingLoad = -1;
                if (index < (int)sSavedScenes.size() &&
                    sSavedScenes[index].value("sceneId", -1) == gPlayState->sceneId) {
                    ApplyScene(sSavedScenes[index]);
                } else {
                    sSceneMessage = "No se pudo llegar a la escena guardada.";
                }
            }
        }
        if (sEnabled && sFreezeWorld) {
            if (!sTimeCaptured) {
                sFrozenTime = gSaveContext.save.time;
                sTimeCaptured = true;
            }
            gSaveContext.save.time = sFrozenTime;
        } else {
            sTimeCaptured = false;
        }
        if (sEnabled && sMoveLink && gPlayState != nullptr) {
            MoveLinkUpdate();
        }
    });

    // Al cambiar de escena se apaga la pose para no aplicarla a un Link recien cargado.
    COND_HOOK(OnSceneInit, true, [](s8 sceneId, s8 spawnNum) {
        sPoseActive = false;
        sHasBaseYaw = false;
        sMoveLink = false;
        sHasSavedPos = false;
        sMoonBackup.actor = nullptr;
        sScaleBackup.actor = nullptr;
        sHandMtxValid[0] = sHandMtxValid[1] = false;
        // El Link anterior ya no existe: solo se devuelve la espada a la partida.
        if (sEquipBackup.active) {
            gSaveContext.save.saveInfo.equips.equipment = sEquipBackup.equipment;
            sEquipBackup.active = false;
        }
    });
}

static RegisterShipInitFunc initFunc(RegisterPhotoMode, {});

std::string ToLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}

bool MatchesFilter(const PhotoModeAnim& anim) {
    if (sCategory > 0 && anim.category != sCategory - 1) {
        return false;
    }
    if (sOnlyFavorites && sFavorites.count(anim.name) == 0) {
        return false;
    }
    if (sFilter[0] == '\0') {
        return true;
    }
    std::string needle = ToLower(sFilter);
    return ToLower(anim.label).find(needle) != std::string::npos ||
           ToLower(anim.name).find(needle) != std::string::npos;
}

void DrawCameraSection() {
    if (!ImGui::CollapsingHeader("Mundo y camara", ImGuiTreeNodeFlags_DefaultOpen)) {
        return;
    }
    ImGui::Checkbox("Congelar el mundo y la hora", &sFreezeWorld);
    ImGui::Checkbox("Ocultar HUD", &sHideHud);
    ImGui::Checkbox("Ocultar a Tatl", &sHideTatl);

    // Hora del dia
    int minutes = (int)((u32)gSaveContext.save.time * 1440 / 0x10000);
    char timeLabel[16];
    snprintf(timeLabel, sizeof(timeLabel), "%02d:%02d", minutes / 60, minutes % 60);
    if (ImGui::SliderInt("Hora", &minutes, 0, 1439, timeLabel)) {
        u16 t = (u16)((u32)minutes * 0x10000 / 1440);
        gSaveContext.save.time = t;
        sFrozenTime = t;
    }
    if (sDayChoice == 0) {
        sDayChoice = std::clamp((int)CURRENT_DAY, 1, 3);
    }
    ImGui::SliderInt("Dia", &sDayChoice, 1, 3);
    ImGui::SameLine();
    if (ImGui::Button("Aplicar dia")) {
        gSaveContext.save.day = sDayChoice;
        TeleportTo(gSaveContext.save.entrance);
    }
    ImGui::TextWrapped("Dia actual: %d. Cambiar el dia recarga la escena. No guardes la partida despues.",
                       (int)CURRENT_DAY);

    bool debugCam = CVarGetInteger(DEBUG_CAM_CVAR, 0) != 0;
    if (ImGui::Checkbox("Camara libre", &debugCam)) {
        SetDebugCam(debugCam);
    }
    if (debugCam) {
        float speed = CVarGetFloat(DEBUG_CAM_SPEED_CVAR, 0.5f);
        if (ImGui::SliderFloat("Velocidad de la camara", &speed, 0.05f, 2.0f, "%.2f")) {
            CVarSetFloat(DEBUG_CAM_SPEED_CVAR, speed);
        }
        float vspeed = CVarGetFloat(DEBUG_CAM_VSPEED_CVAR, 0.25f);
        if (ImGui::SliderFloat("Velocidad al subir / bajar", &vspeed, 0.02f, 1.0f, "%.2f")) {
            CVarSetFloat(DEBUG_CAM_VSPEED_CVAR, vspeed);
        }
        ImGui::TextWrapped("Stick izquierdo: mover. Stick derecho o botones C: girar. Z / R: subir / bajar. "
                           "Cruceta arriba / abajo: acercar / alejar. A, B o L: mas rapido.");
    }
}

void DrawPoseSection() {
    if (!ImGui::CollapsingHeader("Pose de Link", ImGuiTreeNodeFlags_DefaultOpen)) {
        return;
    }

    bool pose = sPoseActive;
    if (ImGui::Checkbox("Usar pose personalizada", &pose)) {
        if (pose) {
            EnablePose();
        } else {
            sPoseActive = false;
            sHasBaseYaw = false;
        }
    }

    if (ImGui::Button("Tomar la pose actual de Link")) {
        CaptureCurrentPose();
    }
    ImGui::SameLine();
    if (ImGui::Button("Reiniciar ajustes")) {
        ResetLimbs();
    }

    ImGui::Separator();
    ImGui::Text("Animacion");

    // Categoria
    std::vector<const char*> categories;
    categories.push_back("Todas");
    for (size_t i = 0; i < PHOTO_MODE_ANIM_CATEGORY_COUNT; i++) {
        categories.push_back(sPhotoModeAnimCategories[i]);
    }
    ImGui::Combo("Categoria", &sCategory, categories.data(), (int)categories.size());
    ImGui::InputText("Buscar", sFilter, sizeof(sFilter));
    ImGui::Checkbox("Solo favoritas", &sOnlyFavorites);

    if (ImGui::BeginListBox("##anims", ImVec2(-FLT_MIN, 200))) {
        for (int i = 0; i < (int)PHOTO_MODE_ANIM_COUNT; i++) {
            const PhotoModeAnim& anim = sPhotoModeAnims[i];
            if (!MatchesFilter(anim)) {
                continue;
            }
            ImGui::PushID(i);
            bool selected = (i == sAnimIndex);
            std::string label = (sFavorites.count(anim.name) ? "* " : "") + std::string(anim.label);
            if (ImGui::Selectable(label.c_str(), selected)) {
                sAnimIndex = i;
                LoadAnimFrame(sAnimIndex, 0);
                EnablePose(); // Elegir una animacion activa la pose automaticamente
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("%s\n(%s)", sPhotoModeAnimCategories[anim.category], anim.name);
            }
            if (selected) {
                ImGui::SetItemDefaultFocus();
            }
            ImGui::PopID();
        }
        ImGui::EndListBox();
    }

    if (sAnimIndex >= 0) {
        ImGui::Text("%s  (%s)", sPhotoModeAnims[sAnimIndex].label, sPhotoModeAnims[sAnimIndex].name);
        bool fav = sFavorites.count(sPhotoModeAnims[sAnimIndex].name) != 0;
        if (ImGui::Button(fav ? "Quitar de favoritas" : "Agregar a favoritas")) {
            if (fav) {
                sFavorites.erase(sPhotoModeAnims[sAnimIndex].name);
            } else {
                sFavorites.insert(sPhotoModeAnims[sAnimIndex].name);
            }
            SavePhotoConfig();
        }
        int frame = sAnimFrame;
        if (ImGui::SliderInt("Frame", &frame, 0, std::max(0, sAnimFrameCount - 1))) {
            LoadAnimFrame(sAnimIndex, frame);
        }
    } else {
        ImGui::TextWrapped("Base: pose capturada del juego.");
    }
    ImGui::TextWrapped("Goron, Deku y Zora: esas animaciones se ven bien solo con esa forma puesta.");

    ImGui::Separator();
    ImGui::SliderFloat("Girar a Link", &sYawOffset, -180.0f, 180.0f, "%.0f grados");

    int eyes = sEyes + 1;
    if (ImGui::Combo("Ojos", &eyes, sEyeNames, IM_ARRAYSIZE(sEyeNames))) {
        sEyes = eyes - 1;
    }
    int mouth = sMouth + 1;
    if (ImGui::Combo("Boca", &mouth, sMouthNames, IM_ARRAYSIZE(sMouthNames))) {
        sMouth = mouth - 1;
    }

    if (ImGui::TreeNode("Ajustar huesos")) {
        for (int i = 1; i < PLAYER_LIMB_MAX; i++) {
            ImGui::PushID(i);
            ImGui::SliderFloat3(sLimbNames[i], sLimbOffsets[i], -180.0f, 180.0f, "%.0f");
            ImGui::PopID();
        }
        ImGui::TreePop();
    }
}

void DrawObjectSection() {
    if (!ImGui::CollapsingHeader("Convertir a Link en un objeto")) {
        return;
    }
    ImGui::Checkbox("Mostrar un objeto en lugar de Link", &sObjectMode);
    ImGui::TextWrapped("El objeto aparece donde esta Link. Con la camara libre puedes rodearlo.");

    if (ImGui::BeginListBox("##objects", ImVec2(-FLT_MIN, 160))) {
        for (int i = 0; i < sObjectCount; i++) {
            ImGui::PushID(i);
            if (ImGui::Selectable(sObjects[i].name, i == sObjectIndex)) {
                sObjectIndex = i;
                sObjectMode = true;
            }
            ImGui::PopID();
        }
        ImGui::EndListBox();
    }

    ImGui::SliderFloat("Tamano", &sObjectSize, 0.1f, 10.0f, "%.2f", ImGuiSliderFlags_Logarithmic);
    ImGui::SliderFloat("Altura", &sObjectHeight, -50.0f, 200.0f, "%.0f");
    ImGui::SliderFloat("Inclinar", &sObjectRot[0], -180.0f, 180.0f, "%.0f grados");
    ImGui::SliderFloat("Girar", &sObjectRot[1], -180.0f, 180.0f, "%.0f grados");
    ImGui::SliderFloat("Rotar", &sObjectRot[2], -180.0f, 180.0f, "%.0f grados");
    ImGui::Checkbox("Girar solo", &sObjectSpin);
    if (ImGui::Button("Reiniciar objeto")) {
        sObjectSize = 1.0f;
        sObjectHeight = 30.0f;
        sObjectRot[0] = sObjectRot[1] = sObjectRot[2] = 0.0f;
        sObjectSpin = false;
        sObjectSpinAngle = 0.0f;
    }
}

void DrawMoveSection() {
    if (!ImGui::CollapsingHeader("Mover a Link", ImGuiTreeNodeFlags_DefaultOpen)) {
        return;
    }
    if (ImGui::Checkbox("Mover a Link con el mando", &sMoveLink)) {
        if (sMoveLink) {
            StartMoveLink();
        }
    }
    if (sMoveLink) {
        ImGui::SliderFloat("Velocidad de Link", &sMoveSpeed, 0.05f, 3.0f, "%.2f");
        ImGui::TextWrapped("Stick izquierdo: mover (segun la camara). Z / R: subir / bajar. "
                           "C izquierda / C derecha: girar. A, B o L: mas rapido. "
                           "Mientras esta activo la camara libre se queda quieta. "
                           "No hay colisiones: Link puede flotar o atravesar paredes.");
    }
    ImGui::BeginDisabled(!sHasSavedPos);
    if (ImGui::Button("Volver a la posicion original")) {
        RestoreLinkPosition();
    }
    ImGui::EndDisabled();

    ImGui::SliderFloat("Tamano de Link", &sLinkScale, 0.1f, 20.0f, "x%.2f", ImGuiSliderFlags_Logarithmic);
    ImGui::SameLine();
    if (ImGui::Button("x1")) {
        sLinkScale = 1.0f;
    }
}

void DrawFramingSection() {
    if (!ImGui::CollapsingHeader("Encuadre y lente")) {
        return;
    }
    ImGui::Checkbox("Mostrar guias de miniatura", &sGuides);
    if (sGuides) {
        ImGui::Indent();
        ImGui::Checkbox("Tercios", &sGuideThirds);
        ImGui::Checkbox("Linea del centro (para 0% / 100%)", &sGuideCenter);
        ImGui::Checkbox("Zona de la duracion de YouTube (aprox.)", &sGuideBadge);
        ImGui::Unindent();
        ImGui::TextWrapped("Las guias solo se ven con esta ventana abierta: cierrala (o pulsa F1) antes de capturar.");
    }

    if (ImGui::Checkbox("Cambiar el lente (zoom)", &sFovOverride)) {
        if (sFovOverride && gPlayState != nullptr) {
            sFov = gPlayState->view.fovy;
        }
    }
    if (sFovOverride) {
        ImGui::SliderFloat("Campo de vision", &sFov, 5.0f, 120.0f, "%.0f grados");
        ImGui::TextWrapped("Menos grados = mas zoom y fondo mas \"pegado\" al personaje. El juego usa unos 60.");
    }
}

void DrawCleanSection() {
    if (!ImGui::CollapsingHeader("Fondo y limpieza")) {
        return;
    }
    ImGui::Checkbox("Fondo de color solido (pantalla verde)", &sChroma);
    if (sChroma) {
        ImGui::Indent();
        ImGui::ColorEdit3("Color del fondo", sChromaColor, ImGuiColorEditFlags_NoInputs);
        ImGui::SameLine();
        if (ImGui::Button("Verde")) {
            sChromaColor[0] = 0.0f;
            sChromaColor[1] = 1.0f;
            sChromaColor[2] = 0.0f;
        }
        ImGui::SameLine();
        if (ImGui::Button("Magenta")) {
            sChromaColor[0] = 1.0f;
            sChromaColor[1] = 0.0f;
            sChromaColor[2] = 1.0f;
        }
        ImGui::Checkbox("Solo Link (ocultar todos los demas personajes)", &sChromaOnlyLink);
        ImGui::Unindent();
        ImGui::TextWrapped("Usa magenta si el personaje tiene verde (Link con tunica, mascara Deku).");
    }
    ImGui::Checkbox("Ocultar personajes (NPC)", &sHideNpcs);
    ImGui::Checkbox("Ocultar enemigos y jefes", &sHideEnemies);
    ImGui::Checkbox("Ocultar particulas", &sHideParticles);
}

void DrawHandSection() {
    if (!ImGui::CollapsingHeader("Objeto en la mano")) {
        return;
    }
    const char* handNames[2] = { "Mano izquierda", "Mano derecha" };
    for (int h = 0; h < 2; h++) {
        ImGui::PushID(h);
        HandObject& ho = sHand[h];
        ImGui::Checkbox(handNames[h], &ho.on);
        if (ho.on) {
            ImGui::Indent();
            if (ImGui::BeginCombo("Objeto", sObjects[ho.index].name)) {
                for (int i = 0; i < sObjectCount; i++) {
                    ImGui::PushID(i);
                    if (ImGui::Selectable(sObjects[i].name, i == ho.index)) {
                        ho.index = i;
                    }
                    ImGui::PopID();
                }
                ImGui::EndCombo();
            }
            ImGui::SliderFloat("Tamano", &ho.size, 0.1f, 10.0f, "x%.2f", ImGuiSliderFlags_Logarithmic);
            ImGui::SliderFloat3("Mover (X Y Z)", ho.offset, -1500.0f, 1500.0f, "%.0f");
            ImGui::SliderFloat3("Girar (X Y Z)", ho.rot, -180.0f, 180.0f, "%.0f");
            if (ImGui::Button("Reiniciar")) {
                HandObject fresh;
                fresh.on = true;
                fresh.index = ho.index;
                ho = fresh;
            }
            ImGui::Unindent();
        }
        ImGui::PopID();
    }
    ImGui::TextWrapped("Se mueve y gira junto con la mano. Para dos espadas: oculta la espada de Link en \"Equipo "
                       "visible\" o deja la suya y pon otra en la mano libre. Ctrl + clic en un slider para "
                       "escribir el valor exacto.");
}

void DrawScenesSection() {
    if (!ImGui::CollapsingHeader("Escenas guardadas")) {
        return;
    }
    ImGui::InputText("Nombre", sSceneName, sizeof(sSceneName));
    ImGui::SameLine();
    if (ImGui::Button("Guardar") && GetPlayer() != nullptr) {
        std::string name = sSceneName[0] != '\0' ? sSceneName : ("Escena " + std::to_string(sSavedScenes.size() + 1));
        sSavedScenes.push_back(BuildScene(name));
        SavePhotoConfig();
        sSavedSceneIndex = (int)sSavedScenes.size() - 1;
        sSceneName[0] = '\0';
        sSceneMessage = "Guardada: " + name;
    }

    if (ImGui::BeginListBox("##saved", ImVec2(-FLT_MIN, 5 * ImGui::GetTextLineHeightWithSpacing()))) {
        for (int i = 0; i < (int)sSavedScenes.size(); i++) {
            ImGui::PushID(i);
            std::string label = sSavedScenes[i].value("name", std::string("?")) + "  -  " +
                                sSavedScenes[i].value("sceneName", std::string(""));
            if (ImGui::Selectable(label.c_str(), i == sSavedSceneIndex, ImGuiSelectableFlags_AllowDoubleClick)) {
                sSavedSceneIndex = i;
                if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                    LoadSavedScene(i);
                }
            }
            ImGui::PopID();
        }
        ImGui::EndListBox();
    }

    bool valid = sSavedSceneIndex >= 0 && sSavedSceneIndex < (int)sSavedScenes.size();
    ImGui::BeginDisabled(!valid);
    if (ImGui::Button("Cargar")) {
        LoadSavedScene(sSavedSceneIndex);
    }
    ImGui::SameLine();
    if (ImGui::Button("Sobrescribir") && GetPlayer() != nullptr) {
        std::string name = sSavedScenes[sSavedSceneIndex].value("name", std::string("Escena"));
        sSavedScenes[sSavedSceneIndex] = BuildScene(name);
        SavePhotoConfig();
        sSceneMessage = "Sobrescrita: " + name;
    }
    ImGui::SameLine();
    if (ImGui::Button("Borrar")) {
        sSavedScenes.erase(sSavedScenes.begin() + sSavedSceneIndex);
        SavePhotoConfig();
        sSavedSceneIndex = -1;
        sSceneMessage = "Escena borrada.";
    }
    ImGui::EndDisabled();
    if (!sSceneMessage.empty()) {
        ImGui::TextWrapped("%s", sSceneMessage.c_str());
    }
    ImGui::TextWrapped("Guarda pose, equipo, objetos, posicion de Link, camara, hora, luna y fondo. Si la escena es "
                       "de otro lugar o dia, primero te lleva alli. Doble clic tambien carga.");
}

// Guias encima de la imagen del juego (solo mientras la ventana del Modo Foto esta abierta).
void DrawGuides() {
    ImGuiWindow* game = ImGui::FindWindowByName("Main Game");
    if (game == nullptr) {
        return;
    }
    ImVec2 pos = game->Pos;
    ImVec2 size = game->Size;
    if (size.x < 10.0f || size.y < 10.0f) {
        return;
    }
    // Zona 16:9 centrada (lo que ocuparia la miniatura).
    float w = size.x, h = size.y;
    if (w / h > 16.0f / 9.0f) {
        w = h * 16.0f / 9.0f;
    } else {
        h = w * 9.0f / 16.0f;
    }
    ImVec2 a = ImVec2(pos.x + (size.x - w) * 0.5f, pos.y + (size.y - h) * 0.5f);
    ImVec2 b = ImVec2(a.x + w, a.y + h);
    ImDrawList* dl = ImGui::GetForegroundDrawList(game->Viewport);

    const ImU32 shade = IM_COL32(0, 0, 0, 140);
    if (a.x > pos.x + 1.0f) {
        dl->AddRectFilled(pos, ImVec2(a.x, pos.y + size.y), shade);
        dl->AddRectFilled(ImVec2(b.x, pos.y), ImVec2(pos.x + size.x, pos.y + size.y), shade);
    }
    if (a.y > pos.y + 1.0f) {
        dl->AddRectFilled(pos, ImVec2(pos.x + size.x, a.y), shade);
        dl->AddRectFilled(ImVec2(pos.x, b.y), ImVec2(pos.x + size.x, pos.y + size.y), shade);
    }
    dl->AddRect(a, b, IM_COL32(255, 255, 255, 160));

    if (sGuideThirds) {
        const ImU32 c = IM_COL32(255, 255, 255, 110);
        for (int i = 1; i <= 2; i++) {
            float x = a.x + w * i / 3.0f;
            float y = a.y + h * i / 3.0f;
            dl->AddLine(ImVec2(x, a.y), ImVec2(x, b.y), c);
            dl->AddLine(ImVec2(a.x, y), ImVec2(b.x, y), c);
        }
    }
    if (sGuideCenter) {
        float x = a.x + w * 0.5f;
        dl->AddLine(ImVec2(x, a.y), ImVec2(x, b.y), IM_COL32(255, 220, 0, 200), 2.0f);
    }
    if (sGuideBadge) {
        // Aproximado: la etiqueta de duracion ocupa la esquina inferior derecha.
        ImVec2 ba = ImVec2(b.x - w * 0.14f, b.y - h * 0.12f);
        ImVec2 bb = ImVec2(b.x - w * 0.01f, b.y - h * 0.02f);
        dl->AddRectFilled(ba, bb, IM_COL32(255, 0, 0, 70));
        dl->AddRect(ba, bb, IM_COL32(255, 60, 60, 200));
        dl->AddText(ImVec2(ba.x + 4.0f, ba.y + 2.0f), IM_COL32(255, 255, 255, 220), "duracion");
    }
}

void DrawEquipSection() {
    if (!ImGui::CollapsingHeader("Equipo visible", ImGuiTreeNodeFlags_DefaultOpen)) {
        return;
    }
    ImGui::Checkbox("Ocultar espada", &sHideSword);
    if (sHideSword) {
        ImGui::Indent();
        ImGui::Checkbox("Ocultar tambien la funda", &sHideSheath);
        if (sHideSheath) {
            ImGui::TextWrapped("Ojo: el escudo en la espalda va colgado de la funda, asi que tambien desaparece.");
        }
        ImGui::Checkbox("Mano abierta (si no, puno cerrado)", &sOpenHand);
        ImGui::Unindent();
    }
    ImGui::Checkbox("Ocultar escudo", &sHideShield);
    ImGui::TextWrapped("Solo cambia lo que se ve: tu partida conserva la espada y el escudo.");
}

void DrawMoonSection() {
    if (!ImGui::CollapsingHeader("Luna")) {
        return;
    }
    if (FindMoon() == nullptr) {
        ImGui::TextWrapped("No hay luna en esta escena (o todavia no se ve a esta hora).");
        return;
    }
    ImGui::Checkbox("Ocultar la luna", &sMoonHide);
    if (sMoonHide) {
        return;
    }
    if (ImGui::Checkbox("Ajustar la luna", &sMoonAdjust)) {
        if (sMoonAdjust) {
            CaptureMoonBasis();
        }
    }
    if (!sMoonAdjust) {
        return;
    }
    ImGui::TextWrapped("Las direcciones se toman de la camara al activar esta opcion. Si mueves mucho la camara, "
                       "pulsa \"Recalcular direcciones\".");
    ImGui::SliderFloat("Izquierda / derecha", &sMoonOffset[0], -15000.0f, 15000.0f, "%.0f");
    ImGui::SliderFloat("Abajo / arriba", &sMoonOffset[1], -15000.0f, 15000.0f, "%.0f");
    ImGui::SliderFloat("Mas cerca / mas lejos", &sMoonOffset[2], -15000.0f, 15000.0f, "%.0f");
    ImGui::SliderFloat("Tamano", &sMoonScale, 0.1f, 5.0f, "x%.2f");
    ImGui::SliderFloat("Girar", &sMoonRot[0], -180.0f, 180.0f, "%.0f");
    ImGui::SliderFloat("Inclinar", &sMoonRot[1], -90.0f, 90.0f, "%.0f");
    ImGui::SliderFloat("Ladear", &sMoonRot[2], -180.0f, 180.0f, "%.0f");
    if (ImGui::Button("Recalcular direcciones")) {
        CaptureMoonBasis();
    }
    ImGui::SameLine();
    if (ImGui::Button("Restablecer luna")) {
        memset(sMoonOffset, 0, sizeof(sMoonOffset));
        memset(sMoonRot, 0, sizeof(sMoonRot));
        sMoonScale = 1.0f;
    }
    ImGui::TextWrapped("Tip: Ctrl + clic en un slider para escribir el valor exacto.");
}

bool SceneMatchesFilter(const PhotoModeScene& scene) {
    if (sRegion > 0 && strcmp(scene.region, sPhotoModeRegions[sRegion - 1]) != 0) {
        return false;
    }
    if (sSceneFilter[0] == '\0') {
        return true;
    }
    std::string needle = ToLower(sSceneFilter);
    return ToLower(scene.label).find(needle) != std::string::npos ||
           ToLower(scene.alias).find(needle) != std::string::npos;
}

void DrawTeleportSection() {
    if (!ImGui::CollapsingHeader("Teletransporte")) {
        return;
    }
    const char* preview = sRegion == 0 ? "Todas las zonas" : sPhotoModeRegions[sRegion - 1];
    if (ImGui::BeginCombo("Zona", preview)) {
        if (ImGui::Selectable("Todas las zonas", sRegion == 0)) {
            sRegion = 0;
        }
        for (int i = 0; i < (int)PHOTO_MODE_REGION_COUNT; i++) {
            if (ImGui::Selectable(sPhotoModeRegions[i], sRegion == i + 1)) {
                sRegion = i + 1;
            }
        }
        ImGui::EndCombo();
    }
    ImGui::InputText("Buscar escenario", sSceneFilter, sizeof(sSceneFilter));

    if (ImGui::BeginListBox("##escenarios", ImVec2(-FLT_MIN, 8 * ImGui::GetTextLineHeightWithSpacing()))) {
        for (int i = 0; i < (int)PHOTO_MODE_SCENE_COUNT; i++) {
            const PhotoModeScene& scene = sPhotoModeScenes[i];
            if (!SceneMatchesFilter(scene)) {
                continue;
            }
            ImGui::PushID(i);
            if (ImGui::Selectable(scene.label, sSceneIndex == i, ImGuiSelectableFlags_AllowDoubleClick)) {
                sSceneIndex = i;
                if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                    TeleportTo(scene.entrance);
                }
            }
            ImGui::PopID();
        }
        ImGui::EndListBox();
    }

    ImGui::BeginDisabled(sSceneIndex < 0);
    if (ImGui::Button("Teletransportarme")) {
        TeleportTo(sPhotoModeScenes[sSceneIndex].entrance);
    }
    ImGui::EndDisabled();
    ImGui::TextWrapped("Doble clic en un escenario tambien te lleva. Apareces en la entrada principal; "
                       "despues usa \"Mover a Link\" para colocarlo donde quieras. "
                       "Algunas escenas pueden lanzar su cinematica o verse vacias segun el dia y tu progreso. "
                       "Por seguridad, no guardes la partida despues de teletransportarte.");
}

} // namespace

extern "C" int PhotoMode_ShouldHideHud(void) {
    return (sEnabled && sHideHud) ? 1 : 0;
}

extern "C" int PhotoMode_IsMovingLink(void) {
    return (sEnabled && sMoveLink) ? 1 : 0;
}

extern "C" int PhotoMode_ShouldHideWorld(void) {
    return (sEnabled && sChroma) ? 1 : 0;
}

extern "C" void PhotoMode_GetBackgroundColor(unsigned char* rgb) {
    for (int i = 0; i < 3; i++) {
        rgb[i] = (unsigned char)std::clamp((int)(sChromaColor[i] * 255.0f + 0.5f), 0, 255);
    }
}

extern "C" int PhotoMode_ShouldHideShadow(Actor* actor) {
    if (!sEnabled) {
        return 0;
    }
    return (sChroma || IsHiddenByPhotoMode(actor)) ? 1 : 0;
}

extern "C" int PhotoMode_ShouldHideParticles(void) {
    return (sEnabled && (sHideParticles || (sChroma && sChromaOnlyLink))) ? 1 : 0;
}

extern "C" float PhotoMode_AdjustFov(float fov) {
    return (sEnabled && sFovOverride) ? sFov : fov;
}

void PhotoModeWindow::InitElement() {
}

// Draw propio: el de GuiWindow no deja cambiar los colores ni los flags de la ventana.
// - Los colores de fondo se fuerzan con PushStyleColor (SetNextWindowBgAlpha no tenia efecto con el tema de 2Ship).
// - NoDocking: si la ventana esta acoplada a un lado, el juego se encoge y detras solo hay negro, asi que la
//   transparencia no se notaba. Con este flag se desacopla y flota encima del juego.
void PhotoModeWindow::Draw() {
    if (!IsVisible()) {
        return;
    }
    ImGui::SetNextWindowSize(ImVec2(420, 700), ImGuiCond_FirstUseEver);

    const float a = std::clamp(sWindowAlpha, 0.0f, 1.0f);
    ImVec4 windowBg = ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);
    ImVec4 childBg = ImGui::GetStyleColorVec4(ImGuiCol_ChildBg);
    ImVec4 frameBg = ImGui::GetStyleColorVec4(ImGuiCol_FrameBg);
    ImVec4 titleBg = ImGui::GetStyleColorVec4(ImGuiCol_TitleBg);
    ImVec4 titleBgActive = ImGui::GetStyleColorVec4(ImGuiCol_TitleBgActive);
    windowBg.w = a;
    childBg.w *= a;
    frameBg.w *= 0.35f + 0.65f * a; // los controles siguen visibles aunque el fondo sea invisible
    titleBg.w *= 0.5f + 0.5f * a;
    titleBgActive.w *= 0.5f + 0.5f * a;
    ImGui::PushStyleColor(ImGuiCol_WindowBg, windowBg);
    ImGui::PushStyleColor(ImGuiCol_ChildBg, childBg);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, frameBg);
    ImGui::PushStyleColor(ImGuiCol_TitleBg, titleBg);
    ImGui::PushStyleColor(ImGuiCol_TitleBgActive, titleBgActive);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDocking;
    if (a <= 0.001f) {
        flags |= ImGuiWindowFlags_NoBackground;
    }
    if (ImGui::Begin(GetName().c_str(), &mIsVisible, flags)) {
        DrawElement();
    }
    ImGui::End();
    ImGui::PopStyleColor(5);

    if (sEnabled && sGuides && mIsVisible) {
        DrawGuides();
    }

    SyncVisibilityConsoleVariable();
}

void PhotoModeWindow::DrawElement() {
    if (gPlayState == nullptr) {
        ImGui::TextWrapped("Carga una partida para usar el Modo Foto.");
        return;
    }

    if (ImGui::Checkbox("Activar Modo Foto", &sEnabled)) {
        if (!sEnabled) {
            sPoseActive = false;
            sHasBaseYaw = false;
            sObjectMode = false;
            sMoveLink = false;
            sHasSavedPos = false;
            RestoreEquip();
        }
    }
    ImGui::SliderFloat("Opacidad del menu", &sWindowAlpha, 0.0f, 1.0f, "%.2f");
    if (ImGui::GetWindowViewport() != ImGui::GetMainViewport()) {
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f),
                           "La ventana esta fuera del juego: arrastrala encima del juego para que se vea "
                           "la transparencia.");
    }
    ImGui::TextWrapped("Consejo: con F1 ocultas el menu y puedes cerrar esta ventana; el Modo Foto sigue activo.");

    if (!sEnabled) {
        return;
    }

    LoadPhotoConfig();
    DrawScenesSection();
    DrawCameraSection();
    DrawFramingSection();
    DrawCleanSection();
    DrawMoveSection();
    DrawEquipSection();
    DrawHandSection();
    DrawPoseSection();
    DrawObjectSection();
    DrawMoonSection();
    DrawTeleportSection();
}
