#ifndef BEZIER_CURVE_H
#define BEZIER_CURVE_H

#include "Curve.h"
#include "InteractiveAffPoint.h"
#include "SceneElement.h"

class BezierCurve : public Curve {
  public:
    BezierCurve(ShaderIF* shaderIF, ShaderIF* sIFpoint, InteractiveAffPoint* controlPoints[4]);
    ~BezierCurve() {}
    void render();
    void checkForPick();
    void moveBy(AffVector);
    void renderPoints();
    void renderCurve();
    void renderPolygon(ShaderIF*);
    void updatePoint();
    void prepareForMove();
    const static ShaderIF::ShaderSpec bezShaders[];
    std::ostream& operator<<(std::ostream&) const;

  private:
    bool first = true;
    const static PhongMaterial selectedMat;
    const static PhongMaterial unselectedMat;
    InteractiveAffPoint* P[4];
    GLuint vao, vbo[1];
};

#endif