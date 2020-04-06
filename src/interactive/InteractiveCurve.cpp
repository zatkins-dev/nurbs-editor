
#include "InteractiveCurve.h"

vector<ShaderIF::ShaderSpec> InteractiveCurve::shaders{
    {"shaders/nurbs.vert", GL_VERTEX_SHADER},
    {"shaders/pixelLengthCurve.tesc", GL_TESS_CONTROL_SHADER},
    {"shaders/nurbsCurve.tese", GL_TESS_EVALUATION_SHADER},
    {"shaders/phong.frag", GL_FRAGMENT_SHADER},
};

std::ostream& operator<<(std::ostream& os, const InteractiveCurve& c) {
    c.print(os);
    return os;
}

// Creates uniformly spaced knots that are clamped if @param `clamped` is true
InteractiveCurve::InteractiveCurve(vector<AffPoint> ctrl_pts, vector<double> weights, bool clamped,
                                   int order)
    : Interactive(shaderIFManager->get("nurbsCurve")), K(order) {
    if (K <= 0)
        return;
    if (clamped) {
        for (int i = 0; i < K; i++)
            knots.push_back(0.0);

        for (int i = 1; i <= ctrl_pts.size() - K; i++)
            knots.push_back(i);

        for (int i = 0; i < K; i++)
            knots.push_back(ctrl_pts.size() - K + 1);
    } else {
        for (int i = 0; i <= ctrl_pts.size() + K; i++)
            knots.push_back(i);
    }
    if (weights.size() != ctrl_pts.size()) {
        weights.clear();
        weights = vector<double>(ctrl_pts.size(), 1);
    }

    initGeometry(ctrl_pts, weights);
}

// Uses provided knot vector
InteractiveCurve::InteractiveCurve(vector<AffPoint> ctrl_pts, vector<float> knots,
                                   vector<double> weights, int order)
    : Interactive(shaderIFManager->get("nurbsCurve")), knots(knots), K(order) {
    if (K <= 0 || knots.size() != (ctrl_pts.size() + K + 1))
        return;
    if (weights.size() != ctrl_pts.size()) {
        weights.clear();
        weights = vector<double>(ctrl_pts.size(), 1);
    }

    initGeometry(ctrl_pts, weights);
}

void InteractiveCurve::initGeometry(vector<AffPoint> ctrl_pts, vector<double> weights) {
    for (int i = 0; i < ctrl_pts.size(); i++) {
        children.push_back(new InteractivePoint(ctrl_pts[i], weights[i]));
        children[i]->setParent(this);
        SceneElement::updateMCBoundingBox(ctrl_pts[i]);
    }
    GLint pgm;
    glGetIntegerv(GL_CURRENT_PROGRAM, &pgm);
    glUseProgram(shaderIFManager->get("nurbsCurve")->getShaderPgmID());
    glGenBuffers(1, &knotsSSB);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, knotsSSB);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * knots.size(), knots.data(),
                 GL_STATIC_READ);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, knotsSSB);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    vec4 buf[children.size()];
    int i = 0;
    for (auto p : children) {
        p->pCoords(buf, i++);
    }
    glBufferData(GL_ARRAY_BUFFER, children.size() * sizeof(vec4), buf, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(shaderIFManager->get("nurbsCurve")->pvaLoc("mcPosition"), 4, GL_FLOAT,
                          false, 0, 0);
    glEnableVertexAttribArray(shaderIFManager->get("nurbsCurve")->pvaLoc("mcPosition"));
    glUseProgram(pgm);
}

void InteractiveCurve::p_checkForPick() {
    if (currentlyPickedObject == this) {
        selected = false;
        currentlyPickedObject = nullptr;
    }
    for (auto c : children) {
        if (c->isSelected()) {
            selected = true;
            currentlyPickedObject = this;
            break;
        }
    }
    if (selected && (clickMods & MVC_SHIFT_BIT) && (clickMods & MVC_CTRL_BIT)) {

        for (auto c : children) {
            if (!c->isSelected())
                c->select();
        }
        return;
    }
};

void InteractiveCurve::p_clickReleased() {}

void InteractiveCurve::p_update() {
    GLint pgm;
    glGetIntegerv(GL_CURRENT_PROGRAM, &pgm);
    glUseProgram(shaderIFManager->get("nurbsCurve")->getShaderPgmID());
    // std::cout << "shader program id: " << shaderIFManager->get("nurbsCurve")->getShaderPgmID()
    //           << "\n";

    vec4 buf[children.size()];
    for (int i = 0; i < children.size(); i++) {
        children[i]->pCoords(buf, i);
    }

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, children.size() * sizeof(vec4), buf);

    glUseProgram(pgm);
}

void InteractiveCurve::p_render() {
    GLint pgm;
    glGetIntegerv(GL_CURRENT_PROGRAM, &pgm);
    glUseProgram(shaderIFManager->get("nurbsCurve")->getShaderPgmID());

    establishView();
    establishLightingEnvironment();

    if (selected) {
        establishMaterialProperties(selectedMat);
    } else {
        establishMaterialProperties(unselectedMat);
    }

    GLint vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);
    glUniform2i(shaderIFManager->get("nurbsCurve")->ppuLoc("vpWidthHeight"), vp[2], vp[3]);
    glUniform1f(shaderIFManager->get("nurbsCurve")->ppuLoc("maxPixelLength"), 10);
    glUniform1i(shaderIFManager->get("nurbsCurve")->ppuLoc("order"), K);

    glBindVertexArray(vao);

    glPatchParameteri(GL_PATCH_VERTICES, K);

    for (int i = 0; i < N() - K + 1; i++) {
        if (!test) {
            std::cout << "knotIndex=" << i + K - 1 << '\n';
            std::cout << "Ctrl Points P_" << i << " - P_" << i + K << "\n";
        }
        glUniform1i(shaderIFManager->get("nurbsCurve")->ppuLoc("knotIndex"), i + K - 1);
        glDrawArrays(GL_PATCHES, i, K);
    }
    test = true;
    glUseProgram(pgm);
}

void InteractiveCurve::print(std::ostream& os) const {
    os << "c " << N() << " " << K << std::endl;
    for (auto* pt : children)
        os << pt->x / pt->w << " " << pt->y / pt->w << " " << pt->z / pt->w << " " << pt->w << '\n';

    os << "t ";
    for (float t : knots)
        os << t << " ";
    os << '\n';
}