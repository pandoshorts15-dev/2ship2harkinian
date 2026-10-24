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

#ifdef __cplusplus
}
#endif
