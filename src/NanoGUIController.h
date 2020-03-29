#ifndef NANO_GUI_CONTROLLER_H
#define NANO_GUI_CONTROLLER_H

#define NANOGUI_USE_OPENGL

#include <GLFWController.h>

#include <nanogui/nanogui.h>

class NanoGUIController : public GLFWController {
  public:
    NanoGUIController(const std::string& windowTitle, int rcFlags = 0);
    virtual ~NanoGUIController();

    nanogui::Screen* getNanoGUIScreen() { return screen; }
    void setRunWaitsForAnEvent(bool b) { runWaitsForAnEvent = b; }
    void run();

  protected:
    virtual void handleDisplay();
    void initializeCallbacksForRC();

    GLFWwindow* theWindow;

  private:
    bool returnFromRun, runWaitsForAnEvent;
    int lastPixelPosX, lastPixelPosY;
    nanogui::Screen* screen;

    static int numNanoGUIControllers;

    static void charCB(GLFWwindow* window, unsigned int theChar);
    static void displayCB(GLFWwindow* window);
    static void keyboardCB(GLFWwindow* window, int key, int scanCode, int action, int mods);
    static int mapMods(int glfwMods);
    static void mouseFuncCB(GLFWwindow* window, int button, int action, int mods);
    static void mouseMotionCB(GLFWwindow* window, double x, double y);
    static void reshapeCB(GLFWwindow* window, int width, int height);
    static void scrollCB(GLFWwindow* window, double xOffset, double yOffset);
};

#endif