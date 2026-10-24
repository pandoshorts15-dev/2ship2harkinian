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

#ifdef __cplusplus
}
#endif
