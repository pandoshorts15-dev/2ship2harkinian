#pragma once

#ifdef __cplusplus
#include <ship/window/gui/GuiWindow.h>

class PhotoModeWindow : public Ship::GuiWindow {
  public:
    using GuiWindow::GuiWindow;

    void Draw() override;
    void InitElement() override;
    void DrawElement() override;
    void UpdateElement() override{};
};

extern "C" {
#endif

// Lo llama z_play.c antes de dibujar la interfaz: devuelve 1 si el Modo Foto pide ocultar el HUD.
int PhotoMode_ShouldHideHud(void);
// Lo usa la camara libre: devuelve 1 mientras el stick se usa para mover a Link (la camara se queda quieta).
int PhotoMode_IsMovingLink(void);

struct Actor;
// Fondo de color solido: z_play.c no dibuja el escenario y limpia la pantalla con este color.
int PhotoMode_ShouldHideWorld(void);
void PhotoMode_GetBackgroundColor(unsigned char* rgb);
// z_actor.c: no dibujar la sombra de actores ocultos (o ninguna, con el fondo de color).
int PhotoMode_ShouldHideShadow(struct Actor* actor);
// z_actor.c: no dibujar particulas.
int PhotoMode_ShouldHideParticles(void);
// z_play.c: campo de vision (lente) elegido en el Modo Foto.
float PhotoMode_AdjustFov(float fov);

#ifdef __cplusplus
}
#endif
