#include "BezierCurve.h"

const ShaderIF::ShaderSpec BezierCurve::bezShaders[]{
    {"shaders/basic.vert", GL_VERTEX_SHADER},
    {"shaders/pixelLength.tesc", GL_TESS_CONTROL_SHADER},
    {"shaders/bezier.tese", GL_TESS_EVALUATION_SHADER},
    {"shaders/phong.frag", GL_FRAGMENT_SHADER},
};

const PhongMaterial BezierCurve::selectedMat =
    PhongMaterial((float[]){1, 0.0, 0.0}, (float[]){1, 0, 0}, (float[]){1, 0, 0}, 1);

const PhongMaterial BezierCurve::unselectedMat =
    PhongMaterial((float[]){204 / 255, 204 / 255, 1}, (float[]){204 / 255, 204 / 255, 1},
                  (float[]){204 / 255, 204 / 255, 1}, 1);

BezierCurve::BezierCurve(ShaderIF* shaderIF, ShaderIF* sIFpoint,
                         InteractiveAffPoint* controlPoints[4])
    : Curve(shaderIF, sIFpoint) {
    for (int i = 0; i < 4; i++) {
        P[i] = controlPoints[i];
        P[i]->setParent(this);
        SceneElement::updateMCBoundingBox(*P[i]);
    }
    // shaderIF = new ShaderIF(bezShaders, 4);
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    vec3 buf[4];
    for (int i = 0; i < 4; i++) {
        P[i]->aCoords(buf, i);
    }
    glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(vec3), buf, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(shaderIF->pvaLoc("mcPosition"), 3, GL_FLOAT, false, 0, 0);
    glEnableVertexAttribArray(shaderIF->pvaLoc("mcPosition"));
    glDisableVertexAttribArray(shaderIF->pvaLoc("mcNormal"));
    glDisableVertexAttribArray(shaderIF->pvaLoc("texCoords"));
}

void BezierCurve::updatePoint() {
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    vec3 buf[4];
    for (int i = 0; i < 4; i++) {
        P[i]->aCoords(buf, i);
    }
    glBufferSubData(GL_ARRAY_BUFFER, 0, 4 * sizeof(vec3), buf);
}

void BezierCurve::checkForPick() {
    for (int i = 0; i < 4; i++) {
        P[i]->checkForPick();
    }
}

void BezierCurve::prepareForMove() {
    Curve::prepareForMove();
    if (!(pickedPts.size() && restrictToVector))
        return;
    auto picked = *pickedPts.begin();
    AffVector image;
    if (P[0] == picked || P[1] == picked)
        image = *P[0] - *P[1];
    else
        image = *P[3] - *P[2];
    image.normalize();
    dragRestriction = Matrix3x3::tensorProductMatrix(image, image);
}

void BezierCurve::moveBy(AffVector v) {
    if (currentlyPickedObject == this) {
        if (moveAll) {
            (*pickedPts.begin())->moveBy(v);
            for (auto p : pickedPts) {
                p->moveBy(*(*pickedPts.begin()) - *p);
            }
        } else {
            (*pickedPts.begin())->moveBy(v);
        }
    }
}

void BezierCurve::render() {
    GLint pgm;
    glGetIntegerv(GL_CURRENT_PROGRAM, &pgm);
    glUseProgram(shaderIF->getShaderPgmID());

    establishView();
    establishLightingEnvironment();

    renderCurve();
    glUseProgram(pgm);
    if (showPoints) {
        renderPoints();
    }
}
void BezierCurve::renderPolygon(ShaderIF* sIF) {
    glBindVertexArray(vao);
    if (first) {
        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
        glVertexAttribPointer(sIF->pvaLoc("mcPosition"), 3, GL_FLOAT, false, 0, 0);
        first = false;
    }
    glVertexAttrib3f(sIF->pvaLoc("mcNormal"), 0, 0, -1);
    glVertexAttrib2f(sIF->pvaLoc("texCoords"), 0, 0);
    glDrawArrays(GL_LINE_STRIP, 0, 4);
}
void BezierCurve::renderPoints() {
    for (int i = 0; i < 4; i++) {
        P[i]->renderPoint(i == 0, this == currentlyPickedObject);
    }
}

void BezierCurve::renderCurve() {

    if (currentlyPickedObject == this) {
        establishMaterialProperties(selectedMat);
    } else {
        establishMaterialProperties(unselectedMat);
    }
    GLint vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);
    glUniform2i(shaderIF->ppuLoc("vpWidthHeight"), vp[2], vp[3]);
    glUniform1f(shaderIF->ppuLoc("maxPixelLength"), 10);
    glUniform1i(shaderIF->ppuLoc("degree"), 3);
    glUniform1i(shaderIF->ppuLoc("applyLDSinVert"), 1);

    glBindVertexArray(vao);
    glPatchParameteri(GL_PATCH_VERTICES, 3 + 1);
    glVertexAttrib3f(shaderIF->pvaLoc("mcNormal"), 0, 0, -1);
    glVertexAttrib2f(shaderIF->pvaLoc("texCoords"), 0, 0);
    glDrawArrays(GL_PATCHES, 0, 3 + 1);
    glUniform1i(shaderIF->ppuLoc("applyLDSinVert"), 1);
}

std::ostream& BezierCurve::operator<<(std::ostream& os) const {
    os << "b" << std::endl;
    for (auto p : P) {
        os << *p << std::endl;
    }
    return os;
}
