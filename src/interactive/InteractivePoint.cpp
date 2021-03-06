#include "InteractivePoint.h"

vector<ShaderIF::ShaderSpec> InteractivePoint::shaders{
    {"shaders/basic.vert", GL_VERTEX_SHADER},
    {"shaders/phong.frag", GL_FRAGMENT_SHADER},
};

InteractivePoint::InteractivePoint(AffPoint p, double weight, int indexS, int indexT)
    : Interactive(shaderIFManager->get("point")), ProjPoint(p, weight), tIndex(indexT),
      sIndex(indexS) {

    updateMCBoundingBox(p);
    BasicShape* point = BasicShape::makeSphere(p, clickThreshold);
    sphere = new BasicShapeRenderer(shaderIF, point);
}

void InteractivePoint::p_checkForPick() {
    if (!(clickMods & MVC_SHIFT_BIT || clickMods & MVC_CTRL_BIT || currentPickTriggerIsHover)) {
        selected = false;
    }
    double distFromClick = P().distanceFromLine(Q_mc, eHat_mc);
    if (distFromClick > clickThreshold)
        return;
    if ((clickMods & MVC_SHIFT_BIT) && !currentPickTriggerIsHover) {
        selected = true;
        return;
    }
    if ((clickMods & MVC_CTRL_BIT) && !currentPickTriggerIsHover) {
        select();
        return;
    }
    double distFromProjPlane = P().distanceTo(Q_mc);
    if ((minDist == -1 || minDist > distFromProjPlane) && !currentPickTriggerIsHover) {
        minDist = distFromProjPlane;
        select();
    }
}

void InteractivePoint::p_moveBy(AffVector dist) {
    AffPoint newP = P() + dist;
    x = w * newP.x;
    y = w * newP.y;
    z = w * newP.z;
}

void InteractivePoint::p_update() {
    GLint pgm;
    glGetIntegerv(GL_CURRENT_PROGRAM, &pgm);
    glUseProgram(shaderIFManager->get("point")->getShaderPgmID());
    updateMCBoundingBox(P());
    if (sphere)
        delete sphere;
    BasicShape* point = BasicShape::makeSphere(P(), clickThreshold);
    sphere = new BasicShapeRenderer(shaderIFManager->get("point"), point);
    glUseProgram(pgm);
}

void InteractivePoint::p_render() {
    GLint pgm;
    glGetIntegerv(GL_CURRENT_PROGRAM, &pgm);
    glUseProgram(shaderIF->getShaderPgmID());

    establishView();
    establishLightingEnvironment();

    if (selected) {
        establishMaterialProperties(selectedMat);
    } else {
        establishMaterialProperties(unselectedMat);
    }

    sphere->renderShape();

    glUseProgram(pgm);
}
