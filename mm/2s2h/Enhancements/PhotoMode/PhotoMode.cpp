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

#include <cstring>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>

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
        Player* player = GetPlayer();
        if (player == nullptr) {
            return;
        }
        if (actor == &player->actor) {
            if (sObjectMode) {
                DrawObject(player);
                *should = false;
            } else {
                if (sPoseActive) {
                    ApplyPose(player);
                }
                ApplyEquipHide(player);
            }
        } else if (sHideTatl && actor == player->tatlActor) {
            *should = false;
        }
    });

    // Justo despues de dibujar a Link se le devuelve el equipo.
    COND_HOOK(OnActorDraw, true, [](Actor* actor) {
        if (sEquipBackup.active && actor == &sEquipBackup.player->actor) {
            RestoreEquip();
        }
    });

    // Detener la hora mientras el mundo esta congelado.
    COND_HOOK(OnGameStateUpdate, true, []() {
        RestoreEquip(); // seguridad: nunca dejar el equipo cambiado durante la logica del juego
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

    if (ImGui::BeginListBox("##anims", ImVec2(-FLT_MIN, 200))) {
        for (int i = 0; i < (int)PHOTO_MODE_ANIM_COUNT; i++) {
            const PhotoModeAnim& anim = sPhotoModeAnims[i];
            if (!MatchesFilter(anim)) {
                continue;
            }
            ImGui::PushID(i);
            bool selected = (i == sAnimIndex);
            if (ImGui::Selectable(anim.label, selected)) {
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

    DrawCameraSection();
    DrawMoveSection();
    DrawEquipSection();
    DrawPoseSection();
    DrawObjectSection();
    DrawTeleportSection();
}
