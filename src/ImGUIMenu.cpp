#include "ImGUIMenu.h"
#include <fstream>
#include <stdio.h>
ImGUIMenu::ImGUIMenu() : SceneElement(shaderIFManager->get("point")) {
    renderWindow = glfwGetCurrentContext();
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    window = glfwCreateWindow(1280, 720, "Nurbs Editor Menu", NULL, renderWindow);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    // ImVector<ImWchar> ranges;
    // ImFontGlyphRangesBuilder builder;
    // builder.AddRanges(io.Fonts->GetGlyphRangesDefault()); // Add one of the default ranges
    // builder.AddChar(ImU8('☑'));
    // builder.AddChar(ImU8('☐'));
    // builder.BuildRanges(&ranges);
    // io.Fonts->AddFontFromFileTTF("ext/imgui/misc/fonts/ProggyClean.ttf", 14, NULL, ranges.Data);

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsClassic();

    // Setup Platform/Renderer bindings
    const char* glsl_version = "#version 450";
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    initRestrictions();

    glfwMakeContextCurrent(renderWindow);
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
    renderWindow = glfwGetCurrentContext();
    glfwMakeContextCurrent(window);
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    {
        ImGui::Begin("View");
        if (ImGui::Button("Save")) {
            ImGui::OpenPopup("Save Menu");
        }
        if (ImGui::BeginPopupModal("Save Menu", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::InputText("Filename", &fname);
            if (ImGui::Button("Confirm")) {
                std::ofstream out(fname);
                InteractiveController::getCurrentController()->saveAllToStream(out);
                out.close();
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel")) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
        if (ImGui::Button("Load")) {
            ImGui::OpenPopup("Load Menu");
        }
        if (ImGui::BeginPopupModal("Load Menu", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::InputText("Filename", &fname);
            if (ImGui::Button("Confirm")) {
                std::ifstream is(fname);
                glfwMakeContextCurrent(renderWindow);
                InteractiveController::getCurrentController()->loadAllFromStream(is);
                glfwMakeContextCurrent(window);
                is.close();
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel")) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
        ImGui::Separator();
        ImGui::Checkbox("Show Add Window", &show_add_window);
        ImGui::Checkbox("Show Edit Window", &show_edit_window);
        ImGui::Checkbox("Show Drag Window", &show_drag_window);
        ImGui::Separator();
        if (ImGui::Button("Reset View")) {
            resetView();
        }
        if (ImGui::Button("Rotate x -90")) {
            addToGlobalRotationDegrees(-90, 0, 0);
        }
        ImGui::SameLine();
        if (ImGui::Button("Rotate x +90")) {
            addToGlobalRotationDegrees(90, 0, 0);
        }
        if (ImGui::Button("Rotate y -90")) {
            addToGlobalRotationDegrees(0, -90, 0);
        }
        ImGui::SameLine();
        if (ImGui::Button("Rotate y +90")) {
            addToGlobalRotationDegrees(0, 90, 0);
        }
        if (ImGui::Button("Rotate z -90")) {
            addToGlobalRotationDegrees(0, 0, -90);
        }
        ImGui::SameLine();
        if (ImGui::Button("Rotate z +90")) {
            addToGlobalRotationDegrees(0, 0, 90);
        }
        ImGui::Separator();
        ImGui::Text("Current MC Bounding Box: ");
        double xyz[6];
        Controller::getCurrentController()->getOverallMCBoundingBox(xyz);
        for (int i = 0; i < 3; i++)
            ImGui::Text("%c: (%.2f, %.2f)", 'x' + i, xyz[2 * i], xyz[2 * i + 1]);
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

    glfwMakeContextCurrent(renderWindow);
}

void ImGUIMenu::resetView() {
    double xyz[6];
    dynamic_cast<InteractiveController*>(Controller::getCurrentController())->updateMCBoundingBox();
    Controller::getCurrentController()->getOverallMCBoundingBox(xyz);
    SceneElement::setMCRegionOfInterest(xyz);

    // Set center
    double xmid = 0.5 * (xyz[0] + xyz[1]);
    double ymid = 0.5 * (xyz[2] + xyz[3]);
    double zmid = 0.5 * (xyz[4] + xyz[5]);
    cryph::AffPoint center(xmid, ymid, zmid);

    // Set eye
    double maxDelta = xyz[1] - xyz[0];
    double delta = xyz[3] - xyz[2];
    if (delta > maxDelta)
        maxDelta = delta;
    delta = xyz[5] - xyz[4];
    if (delta > maxDelta)
        maxDelta = delta;
    double distEyeCenter = 2.0 * maxDelta;
    cryph::AffPoint eye(xmid, ymid, zmid + distEyeCenter);

    // Set up
    cryph::AffVector up = cryph::AffVector::yu;

    // Notify the ModelView of our MC->EC viewing requests:
    SceneElement::setEyeCenterUp(eye, center, up);

    // Perspective parameters
    double zpp = -(distEyeCenter - 0.5 * maxDelta);
    SceneElement::setProjectionPlaneZ(zpp);
    double zmin = zpp - 2.0 * maxDelta;
    double zmax = zpp + 0.5 * maxDelta;
    SceneElement::setECZminZmax(zmin, zmax);
    SceneElement::setAspectRatioPreservationEnabled(true);
    SceneElement::setProjection(ORTHOGONAL);
    SceneElement::resetGlobalDynamic();
    SceneElement::resetGlobalZoom();
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

void ImGUIMenu::showObjectList() {
    if (ImGui::CollapsingHeader("Object List")) {
        auto objs = InteractiveController::getCurrentController()->getInteractiveModels();
        if (!objs.size()) {
            ImGui::Text("No curves or surfaces in scene.");
            ImGui::Separator();
            return;
        }
        ImGui::Columns(5, "Curves and Surfaces"); // 4-ways, with border
        ImGui::Separator();
        ImGui::Text("ID");
        ImGui::NextColumn();
        ImGui::Text("Name");
        ImGui::NextColumn();
        ImGui::Text("Type");
        ImGui::NextColumn();
        ImGui::Text("# Points");
        ImGui::NextColumn();
        ImGui::Text("Selected");
        ImGui::NextColumn();
        ImGui::Separator();
        int id = 1;
        vector<int> ids;
        vector<string> names;
        vector<string> types;
        vector<int> n;
        vector<bool> selected;

        for (auto o : objs) {
            ids.push_back(id++);
            names.push_back(o->getName());
            switch (o->type()) {
            case Interactive::Curve:
                types.push_back("Curve");
                break;
            case Interactive::Surface:
                types.push_back("Surface");
                break;
            default:
                types.push_back(" ");
                break;
            }
            n.push_back(o->getChildren().size());
            selected.push_back(o == currentlyPickedObject);
        }
        for (int i = 0; i < objs.size(); i++) {
            char idLabel[32];
            sprintf(idLabel, "%02d", ids[i]);
            if (ImGui::Selectable(idLabel, selected[i], ImGuiSelectableFlags_SpanAllColumns)) {
                if (currentlyPickedObject) {
                    static_cast<Interactive*>(currentlyPickedObject)->clearSelection();
                }
                currentlyPickedObject = objs[i];
                objs[i]->selectAll();
            }
            ImGui::NextColumn();
            ImGui::Text(names[i].c_str());
            ImGui::NextColumn();
            ImGui::Text(types[i].c_str());
            ImGui::NextColumn();
            ImGui::Text("%d", n[i]);
            ImGui::NextColumn();
            if (selected[i]) {
                ImVec4 c = ImVec4(0.0, 1.0, 0.0, 1.0);
                ImGui::TextColored(c, "X", selected[i]);
            }
            ImGui::NextColumn();
        }
        ImGui::Columns();
        ImGui::Separator();
    }
}

void ImGUIMenu::showEditMenu(bool* open) {
    ImGui::SetNextWindowSize(ImVec2(430, 450), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Edit Selected Curve", open)) {
        static_data.clear();
        ImGui::End();
        return;
    }

    showObjectList();

    if (currentlyPickedObject == nullptr) {
        static_data.clear();
        ImGui::End();
        return;
    }
    Interactive* parent = static_cast<Interactive*>(currentlyPickedObject);
    vector<InteractivePoint*> selected = parent->getSelectedChildren();
    switch (parent->type()) {
    case Interactive::Curve:
        ImGui::TextColored(ImVec4(.2, 1, .2, 1), "Curve: ");
        break;
    case Interactive::Surface:
        ImGui::TextColored(ImVec4(.2, 1, .2, 1), "Surface: ");
        break;
    default:
        break;
    }
    ImGui::SameLine();
    ImGui::InputText("", &parent->getNameRW());
    ImGui::SameLine();
    if (ImGui::Button("Print")) {
        parent->print(std::cout);
    }
    ImGui::Separator();
    ImGui::Text("Edit Points and Weights");
    helpMarker("Edit points or knots of the selected curve/surface.\n"
               "Hold SHIFT to select multiple points (you must hold SHIFT while dragging multiple "
               "points.)\n"
               "Hold CTRL when selecting a point to toggle its selection state.\n"
               "Hold SHIFT+CTRL when selecting a point to select the entire object\n"
               "Press 'r' or 'c' while points are selected to select all points on their rows or "
               "columns, resp.");
    int i = 0;
    if (static_data.size() != selected.size()) {
        // std::cerr << "static_data reset\n";
        static_data = vector<ProjPoint>(selected.size());
        for (i = 0; i < selected.size(); i++) {
            static_data[i] = *selected[i];
            // std::cerr << static_data[i] << "\n";
        }
    }
    i = 0;
    for (auto& p : static_data) {
        vec4 data;
        data[0] = p.x / p.w;
        data[1] = p.y / p.w;
        data[2] = p.z / p.w;
        data[3] = p.w;

        char label0[20];
        if (parent->type() == Interactive::Type::Surface)
            std::sprintf(label0, "P(%d,%d); w(%d,%d)", selected[i]->s(), selected[i]->t(),
                         selected[i]->s(), selected[i]->t());
        if (parent->type() == Interactive::Type::Curve)
            std::sprintf(label0, "P(%d); w(%d)", selected[i]->t(), selected[i]->t());

        if (ImGui::InputFloat4(label0, data, 2)) {
            // std::cerr << "updating static_data\n";
            if (data[3] > 0) {
                p.x = data[0] * data[3];
                p.y = data[1] * data[3];
                p.z = data[2] * data[3];
                p.w = data[3];
            }
        }

        i++;
    }
    if (ImGui::Button("Update")) {
        static_cast<Interactive*>(currentlyPickedObject)->dirty();
        for (i = 0; i < selected.size(); i++) {
            selected[i]->assign(static_data[i]);
            // std::cerr << *selected[i] << std::endl;
            selected[i]->dirty();
        }
        parent->dirty();
    }
    ImGui::SameLine();
    if (ImGui::Button("Refresh")) {
        static_data = vector<ProjPoint>(selected.size());
        for (i = 0; i < selected.size(); i++) {
            static_data[i] = *selected[i];
            // std::cerr << static_data[i] << "\n";
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

    if (ImGui::Button("Delete Selected Curve/Surface")) {
        ImGui::OpenPopup("Delete?");
    }

    if (ImGui::BeginPopupModal("Delete?", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Are you sure you want to delete selected curve/surface?");
        ImGui::TextColored(ImVec4(1, .2, .2, 1), "This operation cannot be undone.");
        if (ImGui::Button("Yes, delete")) {
            auto c = InteractiveController::getCurrentController();
            c->removeModel(parent);
            c->updateMCBoundingBox();
            glfwMakeContextCurrent(renderWindow);
            delete parent;
            currentlyPickedObject = nullptr;
            c->setModelsChanged();
            glfwMakeContextCurrent(window);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("No, cancel")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

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

    if (ImGui::CollapsingHeader("Add Curve")) {
        addNurbsCurve();
        ImGui::Separator();
    }
    if (ImGui::CollapsingHeader("Add Surface")) {
        addNurbsSurface();
        ImGui::Separator();
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
        // auto addCurve = [this]() {
        glfwMakeContextCurrent(renderWindow);
        vector<AffPoint> pts;
        for (int i = 0; i < nPoints; i++) {
            pts.push_back(AffPoint(-nPoints + 2 * i, 1 - i % 2));
        }
        InteractiveCurve* crv =
            new InteractiveCurve(pts, vector<double>(nPoints, 1.f), clamped, order);
        auto c = InteractiveController::getCurrentController();
        c->addModel(crv);
        c->setModelsChanged();
        crv->select();
        auto children = crv->getChildren();
        for (auto p : children) {
            p->select();
        }
        currentlyPickedObject = crv;

        // };
        // action_queue.push(addCurve);
        glfwMakeContextCurrent(window);
        addCurveOpen = false;
    }
    // static vector<vec4>() for (int i = 0; i < nPoints; i++) {}
}

void ImGUIMenu::addNurbsSurface() {
    if (ImGui::InputInt("# Points, S", &nPointsS)) {
        if (nPointsS < orderS) {
            nPointsS = orderS;
        }
    }
    if (ImGui::InputInt("# Points, T", &nPointsT)) {
        if (nPointsT < orderT) {
            nPointsT = orderT;
        }
    }
    if (ImGui::InputInt("Order S", &orderS)) {
        if (orderS < 2) {
            orderS = 2;
        }
    }
    if (ImGui::InputInt("Order T", &orderT)) {
        if (orderT < 2) {
            orderT = 2;
        }
    }
    ImGui::InputFloat3("Base Corner", p0);
    ImGui::InputFloat3("Corner, T", pt);
    ImGui::InputFloat3("Corner, S", ps);
    if (ImGui::BeginCombo("Knots", knotsChoice)) {
        for (int n = 0; n < IM_ARRAYSIZE(knotsChoices); n++) {
            bool isSelected = (knotsChoices[n] == knotsChoice);
            if (ImGui::Selectable(knotsChoices[n], isSelected)) {
                knotsChoice = knotsChoices[n];
            }
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
    if (knotsChoice == knotsChoices[2]) {
        if (knotsS.size() != nPointsS + orderS) {
            knotsS.clear();
            for (int i = 0; i < orderS + nPointsS; i++)
                knotsS.push_back(i);
        }
        if (knotsT.size() != nPointsT + orderT) {
            knotsT.clear();
            for (int i = 0; i < orderT + nPointsT; i++)
                knotsT.push_back(i);
        }
        if (ImGui::Button("Edit S Knots")) {
            ImGui::OpenPopup("Knots Editor, S");
        }
        if (ImGui::BeginPopupModal("Knots Editor, S", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            float* ss = knotsS.data();
            for (int i = 0; i < knotsS.size(); i++) {
                if (ImGui::InputFloat(("S" + std::to_string(i)).c_str(), &ss[i], 0.1, 1.0,
                                      "%.2f")) {
                    knotsS[i] = ss[i];
                }
            }
            if (ImGui::Button("OK", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        if (ImGui::Button("Edit T Knots")) {
            ImGui::OpenPopup("Knots Editor, T");
        }
        if (ImGui::BeginPopupModal("Knots Editor, T", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            float* tt = knotsT.data();
            for (int i = 0; i < knotsT.size(); i++) {
                if (ImGui::InputFloat(("T" + std::to_string(i)).c_str(), &tt[i], 0.1, 1.0,
                                      "%.2f")) {
                    knotsT[i] = tt[i];
                }
            }
            if (ImGui::Button("OK", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }
    if (ImGui::Button("Submit")) {
        vector<AffPoint> ctrl_pts;
        AffVector dT = (AffPoint(pt) - AffPoint(p0)) / (double)nPointsT;
        AffVector dS = (AffPoint(ps) - AffPoint(p0)) / (double)nPointsS;
        for (int i = 0; i <= nPointsS; i++)
            for (int j = 0; j <= nPointsT; j++)
                ctrl_pts.push_back(AffPoint(p0) + i * dS + j * dT);

        glfwMakeContextCurrent(renderWindow);
        InteractiveSurface* surf;
        if (knotsChoice == knotsChoices[0]) // uniform
            surf = new InteractiveSurface(ctrl_pts, nPointsS, nPointsT, false, orderS, orderT);
        else if (knotsChoice == knotsChoices[1]) // clamped
            surf = new InteractiveSurface(ctrl_pts, nPointsS, nPointsT, true, orderS, orderT);
        else if (knotsChoice == knotsChoices[2]) // custom
            surf = new InteractiveSurface(ctrl_pts, nPointsS, nPointsT, knotsS, knotsT, orderS,
                                          orderT);
        auto c = InteractiveController::getCurrentController();
        c->addModel(surf);
        c->setModelsChanged();
        glfwMakeContextCurrent(window);
    }
}