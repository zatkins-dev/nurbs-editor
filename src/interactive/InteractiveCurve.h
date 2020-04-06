#pragma once

#include "Interactive.h"
#include "InteractivePoint.h"

#include <functional>

typedef float vec4[4];

class InteractiveCurve : public Interactive {
  public:
    InteractiveCurve(vector<AffPoint> ctrl_pts, vector<double> weights = vector<double>(),
                     bool clamped = true, int order = 4);
    InteractiveCurve(vector<AffPoint> ctrl_pts, vector<float> knots,
                     vector<double> weights = vector<double>(), int order = 4);
    virtual ~InteractiveCurve() {}

    static vector<ShaderIF::ShaderSpec> shaders;
    virtual void print(std::ostream&) const;
    friend std::ostream& operator<<(std::ostream&, const InteractiveCurve&);

  protected:
    void initGeometry(vector<AffPoint> ctrl_pts, vector<double> weights);
    virtual void p_clickReleased();
    virtual void p_checkForPick();
    virtual void p_update();
    virtual void p_render();

  private:
    GLuint knotsSSB, vao, vbo;
    std::function<int()> N = [this]() { return children.size(); };
    int K;
    bool test = false;
    vector<float> knots;
};