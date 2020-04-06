#include "ImGuiController.h"

#include "imgui.h"

#include "examples/imgui_impl_glfw.h"
#include "examples/imgui_impl_opengl3.h"
#include <stdio.h>

#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
#include <GL/gl3w.h> // Initialize with gl3wInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
#include <GL/glew.h> // Initialize with glewInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
#include <glad/glad.h> // Initialize with gladLoadGL()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING2)
#define GLFW_INCLUDE_NONE // GLFW including OpenGL headers causes ambiguity or multiple definition
                          // errors.
#include <glbinding/Binding.h> // Initialize with glbinding::Binding::initialize()
#include <glbinding/gl/gl.h>
using namespace gl;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING3)
#define GLFW_INCLUDE_NONE // GLFW including OpenGL headers causes ambiguity or multiple definition
                          // errors.
#include <glbinding/gl/gl.h>
#include <glbinding/glbinding.h> // Initialize with glbinding::initialize()
using namespace gl;
#else
#include IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#endif

#include <GLFW/glfw3.h>

ImGuiController::ImGuiController(const std::string& windowTitle, int rcFlags)
    : GLFWController(windowTitle, rcFlags) {
    // Create GLFWwindow for imgui rendering
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    imguiWindow = glfwCreateWindow(1280, 720, "Dear ImGui GLFW+OpenGL3 example", NULL, NULL);

    glfwMakeContextCurrent(imguiWindow);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsClassic();

    // Setup Platform/Renderer bindings
    const char* glsl_version = "#version 450";
    ImGui_ImplGlfw_InitForOpenGL(imguiWindow, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
}

ImGuiController::~ImGuiController() { // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(imguiWindow);
}

void ImGuiController::run() {
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

void ImGuiController::step() {
    glfwMakeContextCurrent(imguiWindow);
    glfwPollEvents();

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Call menu stuff

    // Rendering
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(imguiWindow, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
}

void ImGuiController::handleDisplay() {
    prepareWindow();
    // clear the frame buffer
    glClear(clearBits);

    renderAllModels();

    glfwSwapBuffers(theWindow);

    checkForErrors(std::cout, "GLFWController::handleDisplay");
}