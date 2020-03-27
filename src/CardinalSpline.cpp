#include "CardinalSpline.h"

const ShaderIF::ShaderSpec CardinalSpline::splineShaders[]{
    {"shaders/basic.vert", GL_VERTEX_SHADER},
    {"shaders/pixelLength.tesc", GL_TESS_CONTROL_SHADER},
    {"shaders/splines.tese", GL_TESS_EVALUATION_SHADER},
    {"shaders/phong.frag", GL_FRAGMENT_SHADER},
};

const PhongMaterial CardinalSpline::selectedMat =
    PhongMaterial((float[]){1, 0.0, 0.0}, (float[]){1, 0, 0}, (float[]){1, 0, 0}, 1);

const PhongMaterial CardinalSpline::unselectedMat =
    PhongMaterial((float[]){0, 0, 1}, (float[]){0, 0, 1}, (float[]){0, 0, 1}, 1);

CardinalSpline::CardinalSpline(ShaderIF* shaderIF, ShaderIF* sIFpoint,
                               vector<InteractiveAffPoint*> controlPoints, float alpha)
    : Curve(shaderIF, sIFpoint), alpha(alpha) {
    for (auto p : controlPoints) {
        P.push_back(p);
        p->setParent(this);
        SceneElement::updateMCBoundingBox(*p);
    }
    pointShader = new ShaderIF("shaders/basic.vert", "shaders/phong.frag");
    shaderIF = new ShaderIF(splineShaders, 4);
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo[0]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    vec3 buf[P.size()];
    int i = 0;
    for (auto p : P) {
        p->aCoords(buf, i++);
    }
    glBufferData(GL_ARRAY_BUFFER, P.size() * sizeof(vec3), buf, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(shaderIF->pvaLoc("mcPosition"), 3, GL_FLOAT, false, 0, 0);
    glEnableVertexAttribArray(shaderIF->pvaLoc("mcPosition"));
    glDisableVertexAttribArray(shaderIF->pvaLoc("mcNormal"));
    glDisableVertexAttribArray(shaderIF->pvaLoc("texCoords"));
}

void CardinalSpline::updatePoint() {
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    vec3 buf[P.size()];
    for (int i = 0; i < P.size(); i++) {
        P[i]->aCoords(buf, i);
    }
    glBufferSubData(GL_ARRAY_BUFFER, 0, P.size() * sizeof(vec3), buf);
}

void CardinalSpline::checkForPick() {
    for (auto* p : P) {
        p->checkForPick();
    }
}

void CardinalSpline::prepareForMove() {
    Curve::prepareForMove();
    if (!(pickedPts.size() && restrictToVector))
        return;
    auto picked = *pickedPts.begin();
    int n = P.size();
    AffVector image;
    if (picked == P[0])
        image = *P[0] - *P[1];
    else if (picked == P[n - 1])
        image = *P[n - 1] - *P[n - 2];
    else {
        int i;
        for (i = 1; i < n - 2; i++) {
            if (picked == P[i])
                break;
        }
        image = (1 - alpha) * (*P[i] - *P[i - 1]) + alpha * (*P[i + 1] - *P[i]);
    }
    image.normalize();
    dragRestriction = Matrix3x3::tensorProductMatrix(image, image);
}

void CardinalSpline::moveBy(AffVector v) {
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

void CardinalSpline::render() {
    GLint pgm;
    glGetIntegerv(GL_CURRENT_PROGRAM, &pgm);
    glUseProgram(shaderIF->getShaderPgmID());

    establishView();
    establishLightingEnvironment();
    renderCurve();
    glUseProgram(pgm);
    if (showPoints) {
        renderPoints();
        // renderPolygon();
    }
}

void CardinalSpline::renderPoints() {
    for (auto* p : P) {
        p->renderPoint(p == P.front(), this == currentlyPickedObject);
    }
}

void CardinalSpline::renderPolygon(ShaderIF* sIF) {
    glBindVertexArray(vao);
    if (first) {
        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
        glVertexAttribPointer(sIF->pvaLoc("mcPosition"), 3, GL_FLOAT, false, 0, 0);
        first = false;
    }
    glVertexAttrib3f(sIF->pvaLoc("mcNormal"), 0, 0, -1);
    glVertexAttrib2f(sIF->pvaLoc("texCoords"), 0, 0);
    glDrawArrays(GL_LINE_STRIP, 0, P.size());
}

void CardinalSpline::renderCurve() {

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
    glUniform1f(shaderIF->ppuLoc("a"), alpha);
    glUniform1i(shaderIF->ppuLoc("applyLDSinVert"), 0);

    glBindVertexArray(vao);
    glPatchParameteri(GL_PATCH_VERTICES, 4);
    glVertexAttrib3f(shaderIF->pvaLoc("mcNormal"), 0, 0, -1);
    glVertexAttrib2f(shaderIF->pvaLoc("texCoords"), 0, 0);
    // Pass P(i-1),P(i),P(i+1),P(i+2) for i=1...n-3
    for (int i = 0; i <= P.size() - 4; i++)
        glDrawArrays(GL_PATCHES, i, 4);

    glUniform1i(shaderIF->ppuLoc("applyLDSinVert"), 1);
}

std::ostream& CardinalSpline::operator<<(std::ostream& os) const {
    os << "s " << P.size() << " " << alpha << std::endl;
    for (auto p : P) {
        os << *p << std::endl;
    }
    return os;
}
