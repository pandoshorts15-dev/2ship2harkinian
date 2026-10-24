// Modo Foto para 2 Ship 2 Harkinian.
// Congela el mundo, oculta el HUD y a Tatl, activa la camara libre y permite posar a Link:
// cualquier frame de cualquiera de sus animaciones, mas ajustes por hueso, expresion y giro.

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
AnimationHeaderCommon* ResourceMgr_LoadAnimByName(const char* path);
}

#define DEBUG_CAM_CVAR "gEnhancements.Camera.DebugCam.Enable"
#define PHOTO_FRAME_SIZE (sizeof(Vec3s) * PLAYER_LIMB_MAX + sizeof(s16))

namespace {

// ---- Estado ----
bool sEnabled = false;      // Interruptor general
bool sFreezeWorld = true;   // Congela actores y la hora
bool sHideHud = true;       // Oculta la interfaz
bool sHideTatl = true;      // Oculta a Tatl
bool sPoseActive = false;   // Aplica la pose personalizada a Link

u16 sFrozenTime = 0;
bool sTimeCaptured = false;

// Pose
PlayerAnimationFrame sBaseFrame;   // Frame base (de una animacion o capturado)
bool sHasBaseFrame = false;
int sAnimIndex = -1;
int sAnimFrame = 0;
int sAnimFrameCount = 1;
float sLimbOffsets[PLAYER_LIMB_MAX][3] = {};  // Grados por hueso (X, Y, Z)
int sEyes = -1;    // -1 = la que traiga la animacion
int sMouth = -1;   // -1 = la que traiga la animacion
float sYawOffset = 0.0f;
s16 sBaseYaw = 0;
bool sHasBaseYaw = false;
char sFilter[64] = "";

const char* sLimbNames[PLAYER_LIMB_MAX] = {
    "(posicion raiz)", "Raiz",          "Cintura",         "Raiz inferior",   "Muslo derecho",  "Pierna derecha",
    "Pie derecho",     "Muslo izquierdo", "Pierna izquierda", "Pie izquierdo", "Raiz superior",  "Cabeza",
    "Gorro",           "Cuello",        "Hombro izquierdo", "Antebrazo izquierdo", "Mano izquierda",
    "Hombro derecho",  "Antebrazo derecho", "Mano derecha", "Funda",          "Torso",
};

const char* sEyeNames[] = { "De la animacion", "Abiertos", "Entrecerrados", "Cerrados", "Mirando derecha",
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

    PlayerAnimationHeader* anim =
        (PlayerAnimationHeader*)ResourceMgr_LoadAnimByName(sPhotoModeAnims[animIndex].path);
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

void SetDebugCam(bool on) {
    CVarSetInteger(DEBUG_CAM_CVAR, on ? 1 : 0);
    ShipInit::Init(DEBUG_CAM_CVAR);
}

void RegisterPhotoMode() {
    // Congelar actores. Con una pose activa, Link siempre se congela para que no la sobrescriba.
    COND_HOOK(ShouldActorUpdate, true, [](Actor* actor, bool* should) {
        if (!sEnabled) {
            return;
        }
        if (sFreezeWorld) {
            *should = false;
        } else if (sPoseActive && actor->id == ACTOR_PLAYER) {
            *should = false;
        }
    });

    // Aplicar la pose justo antes de dibujar a Link y ocultar a Tatl.
    COND_HOOK(ShouldActorDraw, true, [](Actor* actor, bool* should) {
        if (!sEnabled) {
            return;
        }
        Player* player = GetPlayer();
        if (player == nullptr) {
            return;
        }
        if (actor == &player->actor && sPoseActive) {
            ApplyPose(player);
        } else if (sHideTatl && actor == player->tatlActor) {
            *should = false;
        }
    });

    // Detener la hora mientras el mundo esta congelado.
    COND_HOOK(OnGameStateUpdate, true, []() {
        if (sEnabled && sFreezeWorld) {
            if (!sTimeCaptured) {
                sFrozenTime = gSaveContext.save.time;
                sTimeCaptured = true;
            }
            gSaveContext.save.time = sFrozenTime;
        } else {
            sTimeCaptured = false;
        }
    });

    // Al cambiar de escena se apaga la pose para no aplicarla a un Link recien cargado.
    COND_HOOK(OnSceneInit, true, [](s8 sceneId, s8 spawnNum) {
        sPoseActive = false;
        sHasBaseYaw = false;
    });
}

static RegisterShipInitFunc initFunc(RegisterPhotoMode, {});

bool MatchesFilter(const char* name) {
    if (sFilter[0] == '\0') {
        return true;
    }
    std::string haystack(name);
    std::string needle(sFilter);
    std::transform(haystack.begin(), haystack.end(), haystack.begin(), ::tolower);
    std::transform(needle.begin(), needle.end(), needle.begin(), ::tolower);
    return haystack.find(needle) != std::string::npos;
}

} // namespace

extern "C" int PhotoMode_ShouldHideHud(void) {
    return (sEnabled && sHideHud) ? 1 : 0;
}

void PhotoModeWindow::InitElement() {
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
        }
    }
    ImGui::TextWrapped("Consejo: con F1 ocultas el menu y puedes cerrar esta ventana; el Modo Foto sigue activo.");

    if (!sEnabled) {
        return;
    }

    // ---- Mundo y camara ----
    if (ImGui::CollapsingHeader("Mundo y camara", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Checkbox("Congelar el mundo y la hora", &sFreezeWorld);
        ImGui::Checkbox("Ocultar HUD", &sHideHud);
        ImGui::Checkbox("Ocultar a Tatl", &sHideTatl);

        bool debugCam = CVarGetInteger(DEBUG_CAM_CVAR, 0) != 0;
        if (ImGui::Checkbox("Camara libre", &debugCam)) {
            SetDebugCam(debugCam);
        }
        if (debugCam) {
            ImGui::TextWrapped("Stick izquierdo: mover. Stick derecho: girar. Z / R: subir / bajar. "
                               "Cruceta arriba / abajo: acercar / alejar. A, B o L: mas rapido.");
        }
    }

    // ---- Pose de Link ----
    if (ImGui::CollapsingHeader("Pose de Link", ImGuiTreeNodeFlags_DefaultOpen)) {
        Player* player = GetPlayer();

        if (ImGui::Checkbox("Usar pose personalizada", &sPoseActive)) {
            if (sPoseActive) {
                if (!sHasBaseFrame) {
                    CaptureCurrentPose();
                }
                if (player != nullptr) {
                    sBaseYaw = player->actor.shape.rot.y;
                    sHasBaseYaw = true;
                }
            } else {
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
        ImGui::InputText("Buscar", sFilter, sizeof(sFilter));

        if (ImGui::BeginListBox("##anims", ImVec2(-FLT_MIN, 180))) {
            for (int i = 0; i < (int)PHOTO_MODE_ANIM_COUNT; i++) {
                if (!MatchesFilter(sPhotoModeAnims[i].name)) {
                    continue;
                }
                bool selected = (i == sAnimIndex);
                if (ImGui::Selectable(sPhotoModeAnims[i].name, selected)) {
                    sAnimIndex = i;
                    LoadAnimFrame(sAnimIndex, 0);
                    // Elegir una animacion activa la pose automaticamente
                    if (!sPoseActive) {
                        sPoseActive = true;
                        if (player != nullptr) {
                            sBaseYaw = player->actor.shape.rot.y;
                            sHasBaseYaw = true;
                        }
                    }
                }
                if (selected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndListBox();
        }

        if (sAnimIndex >= 0) {
            ImGui::Text("Animacion: %s", sPhotoModeAnims[sAnimIndex].name);
            int frame = sAnimFrame;
            if (ImGui::SliderInt("Frame", &frame, 0, std::max(0, sAnimFrameCount - 1))) {
                LoadAnimFrame(sAnimIndex, frame);
            }
        } else {
            ImGui::TextWrapped("Base: pose capturada del juego.");
        }

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
}
