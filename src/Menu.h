#ifndef MENU_H
#define MENU_H
#define NANOGUI_USE_OPENGL

#if defined(NANOGUI_GLAD)
#if defined(NANOGUI_SHARED) && !defined(GLAD_GLAPI_EXPORT)
#define GLAD_GLAPI_EXPORT
#endif

#include <glad/glad.h>
#else
#if defined(__APPLE__)
#ifndef GLFW_INCLUDE_GLCOREARB
#define GLFW_INCLUDE_GLCOREARB
#endif
#else
#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#endif
#endif
#endif

#include <GLFW/glfw3.h>

#include "NanoGUIController.h"

#include <iostream>
#include <nanogui/nanogui.h>

using namespace nanogui;

enum test_enum {
    Item1 = 0,
    Item2,
    Item3,
};

class Menu {
  public:
    Menu(Screen*);
    ~Menu();

  protected:
    ref<Window> nanoguiWindow;

  private:
    bool bvar = true;
    int ivar = 12345678;
    double dvar = 3.1415926;
    float fvar = (float)dvar;
    std::string strval = "A string";
    test_enum enumval = Item2;
    Color colval = Color(0.5f, 0.5f, 0.7f, 1.f);

    Screen* screen = nullptr;
};

#endif