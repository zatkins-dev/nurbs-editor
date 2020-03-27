#include "InteractiveAffPoint.h"

#include "Curve.h"

InteractiveAffPoint::InteractiveAffPoint(ShaderIF* shaderIf, const AffPoint& pt)
    : SceneElement(shaderIf), AffPoint(pt) {
    updateMCBoundingBox(*this);
    BasicShape* point = BasicShape::makeSphere(*this, clickThreshold);
    sphere = new BasicShapeRenderer(shaderIf, point);
}

AffPoint InteractiveAffPoint::toLDS() const {
    float scaleTrans[4];
    compute2DScaleTrans(scaleTrans);
    return AffPoint(scaleTrans[0] * x + scaleTrans[1], scaleTrans[2] * y + scaleTrans[3]);
}

void InteractiveAffPoint::checkForPick() {
    Curve::pickedPts.erase(this);
    double distFromClick = this->distanceFromLine(Q_mc, eHat_mc);
    if (clickThreshold > distFromClick) {
        double distFromProjPlane = this->distanceTo(Q_mc);
        if (minDist == -1 || minDist > distFromProjPlane) {
            minDist = distFromProjPlane;
            // std::cout << "IAP: Picked point [" << *this << "]\n";
            Curve::pickedPts.insert(Curve::pickedPts.begin(), this);
            currentlyPickedObject = parent;
            if (clickMods & MVC_CTRL_BIT) {
                try {
                    reinterpret_cast<Curve*>(parent)->restrictDrag(this);
                } catch (std::exception e) {
                    std::cerr << e.what() << std::endl;
                    exit(1);
                }
            }

        } else {
            Curve::pickedPts.insert(this);
        }
    }
}

void InteractiveAffPoint::moveBy(AffVector d) {
    // std::cout << "IAP: move point [" << *this << "] by <" << d << ">\n";
    operator+=(d);
    if (parent) {
        reinterpret_cast<Curve*>(parent)->updatePoint();
    }
    updatePoint();
}

void InteractiveAffPoint::updatePoint() {
    delete sphere;
    BasicShape* point = BasicShape::makeSphere(*this, clickThreshold);
    sphere = new BasicShapeRenderer(shaderIF, point);
}

void InteractiveAffPoint::getMCBoundingBox(double* xyzLimits) {
    SceneElement::getMCBoundingBox(xyzLimits);
}

void InteractiveAffPoint::render() {
    GLint pgm;
    glGetIntegerv(GL_CURRENT_PROGRAM, &pgm);
    glUseProgram(shaderIF->getShaderPgmID());

    establishView();
    establishLightingEnvironment();

    renderPoint();
    reinterpret_cast<Curve*>(parent)->renderPolygon(shaderIF);

    glUseProgram(pgm);
}

void InteractiveAffPoint::renderPoint(bool renderPoly, bool isSelected) {
    GLint pgm;
    glGetIntegerv(GL_CURRENT_PROGRAM, &pgm);
    glUseProgram(shaderIF->getShaderPgmID());

    establishView();
    establishLightingEnvironment();

    glUseProgram(shaderIF->getShaderPgmID());

    if (*Curve::pickedPts.begin() == this || (Curve::moveAll && Curve::pickedPts.count(this))) {
        establishMaterialProperties(red);
    } else {
        establishMaterialProperties(blue);
    }

    sphere->renderShape();

    if (renderPoly) {
        if (isSelected) {
            establishMaterialProperties(red);
        } else {
            establishMaterialProperties(blue);
        }
        reinterpret_cast<Curve*>(parent)->renderPolygon(shaderIF);
    }

    glUseProgram(pgm);
}