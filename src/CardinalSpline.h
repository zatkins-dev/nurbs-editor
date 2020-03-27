#ifndef CARDINAL_SPLINE_H
#define CARDINAL_SPLINE_H

#include "Curve.h"
#include "InteractiveAffPoint.h"
#include "SceneElement.h"

class CardinalSpline : public Curve {
  public:
    CardinalSpline(ShaderIF* shaderIF, ShaderIF* sIFpoint,
                   vector<InteractiveAffPoint*> controlPoints, float alpha = 0.5);
    void render();
    void checkForPick();
    void prepareForMove();
    void moveBy(AffVector);
    void renderPoints();
    void renderCurve();
    void renderPolygon(ShaderIF*);
    void updatePoint();
    const static ShaderIF::ShaderSpec splineShaders[];
    std::ostream& operator<<(std::ostream&) const;

  private:
    bool first = true;
    const static PhongMaterial selectedMat;
    const static PhongMaterial unselectedMat;
    vector<InteractiveAffPoint*> P;
    float alpha;
    GLuint vao, vbo[1];
};

#endif