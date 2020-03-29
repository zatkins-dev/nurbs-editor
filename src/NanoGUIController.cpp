#include "NanoGUIController.h"

int NanoGUIController::numNanoGUIControllers = 0;

NanoGUIController::NanoGUIController(const std::string& windowTitle, int rcFlags)
    : GLFWController(windowTitle, rcFlags), returnFromRun(false), runWaitsForAnEvent(true),
      lastPixelPosX(-999), lastPixelPosY(-999) {
    screen = new nanogui::Screen();
    screen->initialize(theWindow, false);
    initializeCallbacksForRC();
}

NanoGUIController::~NanoGUIController() {}

void NanoGUIController::run() {
    if (theWindow == nullptr)
        return;
    while (!glfwWindowShouldClose(theWindow) && !returnFromRun) {
        if (runWaitsForAnEvent)
            glfwWaitEvents();
        else
            glfwPollEvents();
        handleDisplay();
    }
    glfwDestroyWindow(theWindow);
    theWindow = nullptr;
}

void NanoGUIController::handleDisplay() {
    prepareWindow();
    // clear the frame buffer
    glClear(clearBits);

    renderAllModels();

    screen->draw_widgets();

    glfwSwapBuffers(theWindow);

    checkForErrors(std::cout, "GLFWController::handleDisplay");
}

void NanoGUIController::initializeCallbacksForRC() {
    glfwSetWindowSizeCallback(theWindow, reshapeCB);
    glfwSetCharCallback(theWindow, charCB);
    glfwSetKeyCallback(theWindow, keyboardCB);
    glfwSetMouseButtonCallback(theWindow, mouseFuncCB);
    glfwSetScrollCallback(theWindow, scrollCB);
    glfwSetCursorPosCallback(theWindow, mouseMotionCB);
}

void NanoGUIController::charCB(GLFWwindow* window, unsigned int theChar) {
    if (theChar < 128) {
        NanoGUIController* c = dynamic_cast<NanoGUIController*>(curController);
        c->handleAsciiChar(static_cast<unsigned char>(theChar), c->lastPixelPosX, c->lastPixelPosY);
        c->getNanoGUIScreen()->keyboard_character_event(theChar);
    }
}

void NanoGUIController::displayCB(GLFWwindow* window) {
    if (curController != nullptr) {
        dynamic_cast<NanoGUIController*>(curController)->handleDisplay();
    }
}

void NanoGUIController::keyboardCB(GLFWwindow* window, int key, int scanCode, int action,
                                   int mods) {
    if (curController != nullptr) {
        NanoGUIController* c = dynamic_cast<NanoGUIController*>(curController);
        if ((key == GLFW_KEY_ESCAPE) && (action != GLFW_PRESS)) {
            c->getNanoGUIScreen()->keyboard_event(key, scanCode, action, mods);
            c->handleAsciiChar(27, c->lastPixelPosX, c->lastPixelPosY);
        }
    }
}

int NanoGUIController::mapMods(int glfwMods) {
    int controllerMods = 0;
    if ((glfwMods & GLFW_MOD_SHIFT) != 0)
        controllerMods = MVC_SHIFT_BIT;
    if ((glfwMods & GLFW_MOD_CONTROL) != 0)
        controllerMods |= MVC_CTRL_BIT;
    if ((glfwMods & GLFW_MOD_ALT) != 0)
        controllerMods |= MVC_ALT_BIT;
    return controllerMods;
}

void NanoGUIController::mouseFuncCB(GLFWwindow* window, int button, int action, int mods) {
    if (curController != nullptr) {
        Controller::MouseButton mButton;
        if (button == GLFW_MOUSE_BUTTON_LEFT)
            mButton = Controller::LEFT_BUTTON;
        else if (button == GLFW_MOUSE_BUTTON_RIGHT)
            mButton = Controller::RIGHT_BUTTON;
        else
            mButton = Controller::MIDDLE_BUTTON;
        bool pressed = (action == GLFW_PRESS);
        NanoGUIController* c = dynamic_cast<NanoGUIController*>(curController);
        c->handleMouseButton(mButton, pressed, c->lastPixelPosX, c->lastPixelPosY, mapMods(mods));
        c->getNanoGUIScreen()->mouse_button_callback_event(button, action, mods);
    }
}

void NanoGUIController::mouseMotionCB(GLFWwindow* window, double x, double y) {
    if (curController != nullptr) {
        NanoGUIController* c = dynamic_cast<NanoGUIController*>(curController);
        c->lastPixelPosX = static_cast<int>(x + 0.5);
        c->lastPixelPosY = static_cast<int>(y + 0.5);
        c->handleMouseMotion(c->lastPixelPosX, c->lastPixelPosY);
        c->getNanoGUIScreen()->cursor_pos_callback_event(x, y);
    }
}

void NanoGUIController::reshapeCB(GLFWwindow* window, int width, int height) {
    if (curController != nullptr) {
        auto c = dynamic_cast<NanoGUIController*>(curController);
        c->getNanoGUIScreen()->resize_callback_event(width, height);
        c->handleReshape(width, height);
    }
}

void NanoGUIController::scrollCB(GLFWwindow* window, double xOffset, double yOffset) {
    if (curController != nullptr) {
        auto c = dynamic_cast<NanoGUIController*>(curController);
        c->getNanoGUIScreen()->scroll_callback_event(xOffset, yOffset);
        c->handleDisplay();
    }
}
