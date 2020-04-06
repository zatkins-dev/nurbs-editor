#pragma once

#include "Interactive.h"
#include "ProjPoint.h"

#include "BasicShape.h"
#include "BasicShapeRenderer.h"

#include <functional>

class InteractivePoint : public Interactive, public ProjPoint {
  public:
    InteractivePoint(AffPoint, double);
    virtual ~InteractivePoint() {
        if (sphere)
            delete sphere;
    }
    static vector<ShaderIF::ShaderSpec> shaders;
    void print(std::ostream& os) const {
        os << x / w << " " << y / w << " " << z / w << " " << w << std::endl;
    }
    void assign(double* xyzw) {
        x = xyzw[0] * xyzw[3];
        y = xyzw[1] * xyzw[3];
        z = xyzw[2] * xyzw[3];
        w = xyzw[3];
    }
    void assign(ProjPoint xyzw) {
        x = xyzw.x * xyzw.w;
        y = xyzw.y * xyzw.w;
        z = xyzw.z * xyzw.w;
        w = xyzw.w;
    }

  protected:
    void p_checkForPick();
    void p_moveBy(AffVector dist);
    void p_update();
    void p_render();

  private:
    std::function<AffPoint()> P = [this]() { return AffPoint(x, y, z) / w; };
    BasicShapeRenderer* sphere;
};