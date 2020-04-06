#pragma once

#include "GLFWController.h"

class ImGuiController : public GLFWController {
  public:
    ImGuiController(const std::string& windowTitle, int rcFlags);
    virtual ~ImGuiController();
    void run();
    void step();
    void handleDisplay();

  private:
    GLFWwindow* imguiWindow;
};