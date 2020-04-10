#pragma once

#include "Interactive.h"
#include "InteractivePoint.h"

#include <array>
#include <functional>

typedef float vec4[4];

class InteractiveSurface : public Interactive {
  public:
    InteractiveSurface();
    InteractiveSurface(vector<AffPoint> ctrl_pts, vector<double> weights = vector<double>(),
                       bool clamped = true, int order = 4);
    InteractiveSurface(vector<AffPoint> ctrl_pts, vector<float> knots,
                       vector<double> weights = vector<double>(), int order = 4);
    virtual ~InteractiveSurface() {}

    static vector<ShaderIF::ShaderSpec> shaders;
    virtual void print(std::ostream&) const;
    friend std::ostream& operator<<(std::ostream&, const InteractiveSurface&);

  protected:
    void initGeometry(vector<AffPoint> ctrl_pts, vector<double> weights);
    virtual void p_clickReleased();
    virtual void p_checkForPick();
    virtual void p_update();
    virtual void p_render();
    // virtual void p_renderPoly();

  private:
    GLuint ssboT, ssboS, ssboP, ssboDebug, vao, vbo;
    int nS = 4, nT = 4;
    int kS = 4, kT = 4;
    bool test = false;
    // vector<vector<InteractivePoint*>> orderedPoints;
    vector<float> knotsS;
    vector<float> knotsT;
};