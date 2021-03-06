#pragma once

#include "imgui.h"

#include "examples/imgui_impl_glfw.h"
#include "examples/imgui_impl_opengl3.h"
#include "misc/cpp/imgui_stdlib.h"
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

#include "SceneElement.h"

#include "InteractiveController.h"
#include "ShaderIFManager.h"
#include "interactive/Interactive.h"
#include "interactive/InteractiveCurve.h"
#include "interactive/InteractivePoint.h"
#include "interactive/InteractiveSurface.h"

#include <functional>
#include <map>
#include <queue>
#include <vector>

using std::vector, std::map, std::queue, std::function;

#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

class ImGUIMenu : public SceneElement {
  public:
    ImGUIMenu();
    ~ImGUIMenu();
    void render();
    void getMCBoundingBox(double* xyzLimits) const {
        for (int i = 0; i < 6; i++)
            xyzLimits[i] = 0;
    }

  protected:
    void initRestrictions();

  private:
    GLFWwindow* window;
    GLFWwindow* renderWindow;
    std::queue<function<void()>> action_queue;

    std::string fname;

    typedef float vec4[4];
    int selected_drag = -1;
    bool show_add_window = false;
    bool show_drag_window = true;
    bool show_edit_window = false;
    // edit state
    bool show_object_list = true;
    vector<ProjPoint> static_data;
    AffVector v;
    // add curve state
    bool addCurveOpen = false;
    int nPoints = 4;
    int order = 4;
    bool clamped = true;
    // add surface state
    bool addSurfaceOpen = false;
    int nPointsS = 5;
    int nPointsT = 5;
    int orderS = 4;
    int orderT = 4;
    bool clampedS = true;
    bool clampedT = true;
    vector<float> knotsS;
    vector<float> knotsT;
    vec3 p0{0, 0, 0}, ps{1, 0, 0}, pt{0, 1, 0};
    const char* knotsChoices[3]{
        "Uniform",
        "Clamped",
        "Manual Entry",
    };
    const char* knotsChoice = knotsChoices[0];

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    vector<std::pair<std::string, std::function<void()>>> restrictions;
    vector<std::string> labels;
    // Callback for actions
    void helpMarker(const char* desc);
    void showDragRestrictMenu(bool* open);
    void showAddMenu(bool* open);
    void showEditMenu(bool* open);
    void showObjectList();
    void addNurbsCurve();
    void addNurbsSurface();
    void applyDragRestriction();
    void resetView();

    const vector<AffPoint> DEFAULT_CURVE_PTS{
        AffPoint(-2, 0, 0),
        AffPoint(-1, 1, 0),
        AffPoint(1, 1, 0),
        AffPoint(2, 0, 0),
    };
};