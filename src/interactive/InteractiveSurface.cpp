
#include "InteractiveSurface.h"

vector<ShaderIF::ShaderSpec> InteractiveSurface::shaders{
    {"shaders/nurbs.vert", GL_VERTEX_SHADER},
    {"shaders/pixelLengthSurface.tesc", GL_TESS_CONTROL_SHADER},
    {"shaders/nurbsSurface.tese", GL_TESS_EVALUATION_SHADER},
    {"shaders/addNormal.geom", GL_GEOMETRY_SHADER},
    {"shaders/phong.frag", GL_FRAGMENT_SHADER},
};

std::ostream& operator<<(std::ostream& os, const InteractiveSurface& c) {
    c.print(os);
    return os;
}

InteractiveSurface::InteractiveSurface() : Interactive(shaderIFManager->get("nurbsSurface")) {
    vector<AffPoint> ctrl_pts;
    for (int i = 0; i <= nS; i++)
        for (int j = 0; j <= nT; j++)
            ctrl_pts.push_back(AffPoint(i, j, 0));
    vector<double> weights(ctrl_pts.size(), 1);

    for (int i = 0; i < kT; i++)
        knotsT.push_back(0.0);
    for (int i = 1; i <= nT - (kT - 1); i++)
        knotsT.push_back(i);
    for (int i = 0; i < kT; i++)
        knotsT.push_back(nT + 1 - (kT - 1));

    for (int i = 0; i < kS; i++)
        knotsS.push_back(0.0);
    for (int i = 1; i <= nS - (kS - 1); i++)
        knotsS.push_back(i);
    for (int i = 0; i < kS; i++)
        knotsS.push_back(nS + 1 - (kS - 1));

    initGeometry(ctrl_pts, weights);
}

InteractiveSurface::InteractiveSurface(AffPoint base, AffPoint tmax, AffPoint smax, int nS, int nT,
                                       bool clamped, int orderS, int orderT)
    : Interactive(shaderIFManager->get("nurbsSurface")), nS(nS), nT(nT), kS(orderS), kT(orderT) {
    vector<AffPoint> ctrl_pts;
    AffVector dT = (tmax - base) / (double)nT;
    AffVector dS = (smax - base) / (double)nS;
    for (int i = 0; i <= nS; i++)
        for (int j = 0; j <= nT; j++)
            ctrl_pts.push_back(base + i * dS + j * dT);

    vector<double> weights(ctrl_pts.size(), 1);

    if (clamped) {
        for (int i = 0; i < kT; i++)
            knotsT.push_back(0.0);
        for (int i = 1; i <= nT - (kT - 1); i++)
            knotsT.push_back(i);
        for (int i = 0; i < kT; i++)
            knotsT.push_back(nT + 1 - (kT - 1));

        for (int i = 0; i < kS; i++)
            knotsS.push_back(0.0);
        for (int i = 1; i <= nS - (kS - 1); i++)
            knotsS.push_back(i);
        for (int i = 0; i < kS; i++)
            knotsS.push_back(nS + 1 - (kS - 1));
    } else {
        for (int i = 0; i <= nT + kT; i++)
            knotsT.push_back(i);
        for (int i = 0; i <= nS + kS; i++)
            knotsS.push_back(i);
    }

    initGeometry(ctrl_pts, weights);
}

// Creates uniformly spaced knots that are clamped if @param `clamped` is true
InteractiveSurface::InteractiveSurface(vector<AffPoint> ctrl_pts, int nS, int nT, bool clamped,
                                       int orderS, int orderT, vector<double> weights)
    : Interactive(shaderIFManager->get("nurbsSurface")), kT(orderT), kS(orderS), nT(nT), nS(nS) {
    if (clamped) {
        for (int i = 0; i < kT; i++)
            knotsT.push_back(0.0);
        for (int i = 1; i <= nT - (kT - 1); i++)
            knotsT.push_back(i);
        for (int i = 0; i < kT; i++)
            knotsT.push_back(nT + 1 - (kT - 1));

        for (int i = 0; i < kS; i++)
            knotsS.push_back(0.0);
        for (int i = 1; i <= nS - (kS - 1); i++)
            knotsS.push_back(i);
        for (int i = 0; i < kS; i++)
            knotsS.push_back(nS + 1 - (kS - 1));
    } else {
        for (int i = 0; i <= nT + kT; i++)
            knotsT.push_back(i);
        for (int i = 0; i <= nS + kS; i++)
            knotsS.push_back(i);
    }
    if (weights.size() != ctrl_pts.size()) {
        weights.clear();
        weights = vector<double>(ctrl_pts.size(), 1);
    }

    initGeometry(ctrl_pts, weights);
}

// Uses provided knot vector
InteractiveSurface::InteractiveSurface(vector<AffPoint> ctrl_pts, int nS, int nT,
                                       vector<float> knotsS, vector<float> knotsT, int orderS,
                                       int orderT, vector<double> weights)
    : Interactive(shaderIFManager->get("nurbsSurface")), nS(nS), nT(nT), knotsS(knotsS), kS(orderS),
      knotsT(knotsT), kT(orderT) {

    if (weights.size() != ctrl_pts.size()) {
        weights.clear();
        weights = vector<double>(ctrl_pts.size(), 1);
    }

    initGeometry(ctrl_pts, weights);
}

void InteractiveSurface::initGeometry(vector<AffPoint> ctrl_pts, vector<double> weights) {
    for (int i = 0; i < ctrl_pts.size(); i++) {
        children.push_back(
            new InteractivePoint(ctrl_pts[i], weights[i], i / (nT + 1), i % (nT + 1)));
        children[i]->setParent(this);
        SceneElement::updateMCBoundingBox(ctrl_pts[i]);
    }
    GLint pgm;
    glGetIntegerv(GL_CURRENT_PROGRAM, &pgm);
    glUseProgram(shaderIFManager->get("nurbsSurface")->getShaderPgmID());

    vec4 buf[children.size()];
    int i = 0;
    for (auto p : children) {
        p->pCoords(buf, i++);
    }
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glDisableVertexAttribArray(shaderIFManager->get("nurbsSurface")->pvaLoc("mcPosition"));

    glGenBuffers(1, &ssboP);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboP);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(vec4) * children.size(), buf, GL_STATIC_READ);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssboP);

    glGenBuffers(1, &ssboS);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboS);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * knotsS.size(), knotsS.data(),
                 GL_STATIC_READ);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssboS);

    glGenBuffers(1, &ssboT);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboT);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * knotsT.size(), knotsT.data(),
                 GL_STATIC_READ);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssboT);

    // struct Debug {
    //     // int st[4][4][2];
    //     vec4 v[4][5];
    // } debug;
    // for (int i = 0; i < 4; i++) {
    //     for (int j = 0; j < 5; j++) {
    //         // debug.st[i][j][0] = debug.st[i][j][1] = 0;
    //         for (int k = 0; k < 3; k++)
    //             debug.v[i][j][k] = 0;
    //         debug.v[i][j][3] = 1;
    //     }
    // }

    // glGenBuffers(1, &ssboDebug);
    // glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboDebug);
    // glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(debug), &debug, GL_STATIC_READ);
    // glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, ssboDebug);

    /*
     * Set up control polygon VAO/VBO/EBO
     **/
    glUseProgram(shaderIFManager->get("point")->getShaderPgmID());

    glGenVertexArrays(1, &vaoPoly);
    glBindVertexArray(vaoPoly);

    glGenBuffers(1, &vboPoly);
    glBindBuffer(GL_ARRAY_BUFFER, vboPoly);
    vec3 buf3[children.size()];
    i = 0;
    for (auto p : children) {
        p->aCoords(buf3, i++);
    }
    GLuint eboBuf[nT + 1][nS + 1];
    for (uint i = 0; i <= nT; i++)
        for (uint j = 0; j <= nS; j++) {
            eboBuf[i][j] = at(j, i);
        }
    auto point = shaderIFManager->get("point");
    glBufferData(GL_ARRAY_BUFFER, children.size() * sizeof(vec3), buf3, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(point->pvaLoc("mcPosition"), 3, GL_FLOAT, false, 0, 0);
    glEnableVertexAttribArray(point->pvaLoc("mcPosition"));

    ebo = new GLuint[nT + 1];
    glGenBuffers(nT + 1, ebo);
    for (int i = 0; i <= nT; i++) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo[i]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * (nS + 1), eboBuf[i], GL_STATIC_DRAW);
    }

    glDisableVertexAttribArray(point->pvaLoc("mcNormal"));
    glDisableVertexAttribArray(point->pvaLoc("texCoords"));

    glUseProgram(pgm);
}

void InteractiveSurface::p_checkForPick() {
    if (currentlyPickedObject == this) {
        currentlyPickedObject = nullptr;
    }
    selected = false;
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

void InteractiveSurface::p_clickReleased() {
    selected = false;
    for (auto c : children) {
        if (c->isSelected()) {
            selected = true;
            currentlyPickedObject = this;
            break;
        }
    }
    if (!selected) {
        if (currentlyPickedObject == this) {
            currentlyPickedObject = nullptr;
        }
    }
}

void InteractiveSurface::p_update() {

    GLint pgm;
    glGetIntegerv(GL_CURRENT_PROGRAM, &pgm);
    glUseProgram(shaderIFManager->get("nurbsSurface")->getShaderPgmID());

    vec4 buf[children.size()];
    for (int i = 0; i < children.size(); i++)
        children[i]->pCoords(buf, i);

    glBindVertexArray(vao);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboP);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, children.size() * sizeof(vec4), buf);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssboP);

    glUseProgram(shaderIFManager->get("point")->getShaderPgmID());

    vec3 buf3[children.size()];
    for (int i = 0; i < children.size(); i++)
        children[i]->aCoords(buf3, i);

    glBindVertexArray(vaoPoly);
    glBindBuffer(GL_ARRAY_BUFFER, vboPoly);
    glBufferSubData(GL_ARRAY_BUFFER, 0, children.size() * sizeof(vec3), buf3);

    glUseProgram(pgm);
}

void InteractiveSurface::p_render() {
    GLint pgm;
    glGetIntegerv(GL_CURRENT_PROGRAM, &pgm);
    glUseProgram(shaderIFManager->get("nurbsSurface")->getShaderPgmID());

    establishView();
    establishLightingEnvironment();

    if (selected) {
        establishMaterialProperties(*mat_select);
    } else {
        establishMaterialProperties(*mat_deselect);
    }
    glBindVertexArray(vao);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssboP);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssboS);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssboT);
    // glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, ssboDebug);
    GLint vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);
    glUniform2i(shaderIFManager->get("nurbsSurface")->ppuLoc("vpWidthHeight"), vp[2], vp[3]);
    glUniform1f(shaderIFManager->get("nurbsSurface")->ppuLoc("maxPixelLength"), 10);
    glUniform1i(shaderIFManager->get("nurbsSurface")->ppuLoc("orderS"), kS);
    glUniform1i(shaderIFManager->get("nurbsSurface")->ppuLoc("orderT"), kT);

    glUniform1i(shaderIFManager->get("nurbsSurface")->ppuLoc("nS"), nS);
    glUniform1i(shaderIFManager->get("nurbsSurface")->ppuLoc("nT"), nT);

    glBindVertexArray(vao);

    glPatchParameteri(GL_PATCH_VERTICES, 1);
    glVertexAttrib4f(shaderIFManager->get("nurbsSurface")->pvaLoc("mcPosition"), 0, 0, 0, 1);
    glDrawArrays(GL_PATCHES, 0, 1);

    glUseProgram(pgm);
}

void InteractiveSurface::p_renderPoly() {
    GLint pgm;
    glGetIntegerv(GL_CURRENT_PROGRAM, &pgm);
    shaderIF = shaderIFManager->get("point");
    glUseProgram(shaderIF->getShaderPgmID());

    establishView();
    establishLightingEnvironment();

    if (selected) {
        establishMaterialProperties(selectedMat);
    } else {
        establishMaterialProperties(unselectedMat);
    }

    glBindVertexArray(vaoPoly);

    glVertexAttrib3f(shaderIF->pvaLoc("mcNormal"), 0, 0, 1);
    glVertexAttrib2f(shaderIF->pvaLoc("texCoords"), 0, 0);
    for (int i = 0; i <= nS; i++) {
        glDrawArrays(GL_LINE_STRIP, at(i, 0), nT + 1);
    }
    for (int i = 0; i <= nT; i++) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo[i]);
        glDrawElements(GL_LINE_STRIP, nS + 1, GL_UNSIGNED_INT, nullptr);
    }
    shaderIF = shaderIFManager->get("nurbsSurface");
    glUseProgram(pgm);
}

bool InteractiveSurface::handleCommand(unsigned char key, double ldsX, double ldsY) {
    if (currentlyPickedObject == this) {
        if (key == 'r') {
            for (auto p : getSelectedChildren())
                selectRow(p->s());
        } else if (key == 'c') {
            for (auto p : getSelectedChildren())
                selectCol(p->t());
        } else
            return true;
        return false;
    }
    return true;
}

void InteractiveSurface::selectRow(int i) {
    if (i < 0 || i > nS)
        return; // fail quietly

    for (int j = 0; j <= nT; j++)
        children[at(i, j)]->setSelected();
}

void InteractiveSurface::selectCol(int j) {
    if (j < 0 || j > nT)
        return; // fail quietly

    for (int i = 0; i <= nS; i++)
        children[at(i, j)]->setSelected();
}

void InteractiveSurface::print(std::ostream& os) const {
    os << "S" << getName() << std::endl;
    os << nS << " " << nT << std::endl;
    os << kS << " " << kT << std::endl;
    for (auto pt : children)
        os << pt->x / pt->w << " " << pt->y / pt->w << " " << pt->z / pt->w << " " << pt->w << '\n';
    for (float s : knotsS)
        os << s << " ";
    os << std::endl;
    for (float t : knotsT)
        os << t << " ";
    os << std::endl;
}