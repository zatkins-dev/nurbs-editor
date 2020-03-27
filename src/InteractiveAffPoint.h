#ifndef INTERACTIVE_AFF_POINT_H
#define INTERACTIVE_AFF_POINT_H

#include "SceneElement.h"

#include "BasicShape.h"
#include "BasicShapeRenderer.h"

#include <algorithm>

const PhongMaterial red((float[]){1, 0.0, 0.0}, (float[]){1, 0, 0}, (float[]){1, 0, 0}, 1);
const PhongMaterial blue((float[]){0, 0, 1}, (float[]){0, 0, 1}, (float[]){0, 0, 1}, 1);

class InteractiveAffPoint : public SceneElement, public AffPoint {
  public:
    InteractiveAffPoint(const AffPoint& pt);
    InteractiveAffPoint(ShaderIF* shaderIf, const AffPoint& pt);
    virtual ~InteractiveAffPoint() {}
    void getMCBoundingBox(double* xyzLimits);
    void render();
    void renderPoint(bool = false, bool = false);
    AffPoint toLDS() const;
    void updatePoint();

    void setParent(SceneElement* c) { parent = c; }
    ShaderIF* getShader() { return shaderIF; }
    // click detection
    void checkForPick();
    void moveBy(AffVector d);

  protected:
    friend bool closerPoint(InteractiveAffPoint*, InteractiveAffPoint*);
    SceneElement* parent = nullptr;
    BasicShapeRenderer* sphere;
    AffPoint ptBuffer[25];

  private:
    GLuint vao, vbo[1];
};

#endif