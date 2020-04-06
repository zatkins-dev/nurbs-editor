#include "ImGUIMenu.h"
#include <stdio.h>
ImGUIMenu::ImGUIMenu() : SceneElement(shaderIFManager->get("point")) {
    GLFWwindow* old_window = glfwGetCurrentContext();
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    window = glfwCreateWindow(1280, 720, "Nurbs Editor Menu", NULL, old_window);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsClassic();

    // Setup Platform/Renderer bindings
    const char* glsl_version = "#version 450";
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    initRestrictions();

    glfwMakeContextCurrent(old_window);
}

ImGUIMenu::~ImGUIMenu() {
    // Cleanup
    GLFWwindow* prevCtx = glfwGetCurrentContext();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwMakeContextCurrent(prevCtx);
}

void ImGUIMenu::initRestrictions() {
    auto restrictPerpToVector = [](AffVector v) {
        return [v]() {
            dragRestriction = Matrix3x3::IdentityMatrix - Matrix3x3::tensorProductMatrix(v, v);
        };
    };
    auto restrictParaToVector = [](AffVector v) {
        return [v]() { dragRestriction = Matrix3x3::tensorProductMatrix(v, v); };
    };
    restrictions.push_back(
        std::pair("unrestricted", []() { dragRestriction = Matrix3x3::IdentityMatrix; }));
    restrictions.push_back(std::pair("x-axis", restrictParaToVector(AffVector::xu)));
    restrictions.push_back(std::pair("y-axis", restrictParaToVector(AffVector::yu)));
    restrictions.push_back(std::pair("z-axis", restrictParaToVector(AffVector::zu)));
    restrictions.push_back(std::pair("xy-plane", restrictPerpToVector(AffVector::zu)));
    restrictions.push_back(std::pair("xz-plane", restrictPerpToVector(AffVector::yu)));
    restrictions.push_back(std::pair("yz-plane", restrictPerpToVector(AffVector::xu)));
    // debug
    // std::cerr << "Restriction options: \n";
    // for (auto restriction : restrictions) {
    //     std::cerr << '\t' << restriction.first.c_str() << '\n';
    // }
}

void ImGUIMenu::render() {
    GLFWwindow* prevCtx = glfwGetCurrentContext();
    glfwMakeContextCurrent(window);
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    {
        ImGui::Begin("View");

        ImGui::Checkbox("Show Add Window", &show_add_window);
        ImGui::Checkbox("Show Edit Window", &show_edit_window);
        ImGui::Checkbox("Show Drag Window", &show_drag_window);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                    1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();
    }

    if (show_add_window) {
        showAddMenu(&show_add_window);
    }
    if (show_drag_window) {
        showDragRestrictMenu(&show_drag_window);
    }
    if (show_edit_window) {
        showEditMenu(&show_edit_window);
    }

    // Rendering
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);

    glfwMakeContextCurrent(prevCtx);

    // execute actions in context
    while (!action_queue.empty()) {
        action_queue.front()();
        action_queue.pop();
    }
}

void ImGUIMenu::helpMarker(const char* desc) {
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

void ImGUIMenu::showDragRestrictMenu(bool* open) {
    ImGui::SetNextWindowSize(ImVec2(430, 450), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Drag Restriction", open)) {
        ImGui::End();
        return;
    }

    helpMarker("Select one of the following options to restrict dragging to be"
               "along that axis or plane.");

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
    ImGui::Separator();
    for (int n = 0; n < restrictions.size(); n++) {
        if (ImGui::Selectable(restrictions[n].first.c_str(), selected_drag == n)) {
            restrictions[n].second();
            selected_drag = n;
        }
    }
    ImGui::Separator();
    std::string matrix;
    for (int i = 0; i < 3; i++) {
        matrix += "( ";
        for (int j = 0; j < 3; j++) {
            matrix += std::to_string(dragRestriction.elementAt(i, j)) + " ";
        }
        matrix += ")\n";
    }
    ImGui::TextColored(ImVec4(0.8, 0.1, 0.1, 1), "Drag Restriction Matrix:\n");
    ImGui::Text(matrix.c_str());
    ImGui::PopStyleVar();
    ImGui::End();
}

void ImGUIMenu::showEditMenu(bool* open) {
    ImGui::SetNextWindowSize(ImVec2(430, 450), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Edit Selected Curve", open) || currentlyPickedObject == nullptr) {
        static_data.clear();
        ImGui::End();
        return;
    }
    helpMarker("Edit points or knots of the selected curve/surface.\n"
               "Hold SHIFT+CTRL when selecting a point to select the entire object");
    ImGui::Separator();
    ImGui::Text("Edit Points and Weights");
    Interactive* parent = static_cast<Interactive*>(currentlyPickedObject);
    vector<InteractivePoint*> selected = parent->getSelectedChildren();
    int i = 0;
    if (static_data.size() != selected.size()) {
        std::cerr << "static_data reset\n";
        static_data = vector<ProjPoint>(selected.size());
        for (i = 0; i < selected.size(); i++) {
            static_data[i] = *selected[i];
            std::cerr << static_data[i] << "\n";
        }
    }
    i = 0;
    for (auto& p : static_data) {
        vec4 data;
        p.pCoords(data);
        char label0[20];
        std::sprintf(label0, "(x%d, y%d, z%d), w%d\0", i, i, i, i);
        if (ImGui::InputFloat4(label0, data, 2)) {
            std::cerr << "updating static_data\n";
            p.x = data[0] * data[3];
            p.y = data[1] * data[3];
            p.z = data[2] * data[3];
            p.w = data[3];
        }

        i++;
    }
    if (ImGui::Button("Update")) {
        static_cast<Interactive*>(currentlyPickedObject)->dirty();
        for (i = 0; i < selected.size(); i++) {
            selected[i]->assign(static_data[i]);
            std::cerr << *selected[i] << std::endl;
            selected[i]->dirty();
        }
        parent->dirty();
    }
    ImGui::SameLine();
    if (ImGui::Button("Refresh")) {
        static_data = vector<ProjPoint>(selected.size());
        for (i = 0; i < selected.size(); i++) {
            static_data[i] = *selected[i];
            std::cerr << static_data[i] << "\n";
        }
    }
    ImGui::Text("Edit All Selected Points:");
    vec3 data;
    v.vComponents(data);
    if (ImGui::InputFloat3("Move all by vector", data, 2)) {
        v.assign(data[0], data[1], data[2]);
    }
    if (ImGui::Button("Apply To All")) {
        for (auto p : selected) {
            *p += p->w * ProjPoint(v.dx, v.dy, v.dz, 0);
            p->dirty();
        }
        parent->dirty();
        v = AffVector::zeroVector;
    }
    ImGui::Separator();

    ImGui::End();
}

void ImGUIMenu::showAddMenu(bool* open) {
    ImGui::SetNextWindowSize(ImVec2(430, 450), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Add New Curve or Surface", open)) {
        ImGui::End();
        return;
    }
    helpMarker("Add a new curve or surface to the scene and select it for editing.");
    ImGui::Separator();

    if (ImGui::Button("Add Curve") || addCurveOpen) {
        addCurveOpen = true;
        ImGui::Separator();
        addNurbsCurve();
    } else if (ImGui::Button("Add Surface") || addSurfaceOpen) {
        ImGui::Separator();
        addNurbsSurface();
    }
    ImGui::End();
}

void ImGUIMenu::addNurbsCurve() {
    ImGui::InputInt("# Points (N+1)", &nPoints);
    auto getPoints = [](int n) {
        vector<AffPoint> pts;
        for (int i = 0; i < n; i++) {
            pts.push_back(AffPoint(-n + 2 * i, 1 - i % 2));
        }
        return pts;
    };
    ImGui::InputInt("Order (k)", &order);
    ImGui::Checkbox("Clamped", &clamped);
    if (ImGui::Button("Submit")) {
        if (currentlyPickedObject) {
            static_cast<Interactive*>(currentlyPickedObject)->clearSelection();
        }
        auto addCurve = [this, getPoints]() {
            InteractiveCurve* crv = new InteractiveCurve(
                getPoints(nPoints), vector<double>(nPoints, 1.f), clamped, order);
            Controller::getCurrentController()->addModel(crv);
            crv->select();
            currentlyPickedObject = crv;
        };
        action_queue.push(addCurve);
        addCurveOpen = false;
    }
    // static vector<vec4>() for (int i = 0; i < nPoints; i++) {}
}
void ImGUIMenu::addNurbsSurface() {}