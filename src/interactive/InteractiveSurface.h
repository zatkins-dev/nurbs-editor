#pragma once

#include "Interactive.h"
#include "InteractivePoint.h"

#include <array>
#include <functional>

typedef float vec4[4];

class InteractiveSurface : public Interactive {
  public:
    InteractiveSurface();
    InteractiveSurface(AffPoint base, AffPoint tmax, AffPoint smax, int nS, int nT, bool clamped,
                       int orderS, int orderT);
    InteractiveSurface(vector<AffPoint> ctrl_pts, int nS, int nT, bool clamped = true,
                       int orderS = 4, int orderT = 4, vector<double> weights = vector<double>());
    InteractiveSurface(vector<AffPoint> ctrl_pts, int nS, int nT, vector<float> knotsS,
                       vector<float> knotsT, int orderS = 4, int orderT = 4,
                       vector<double> weights = vector<double>());
    virtual ~InteractiveSurface() {}
    bool handleCommand(unsigned char key, double ldsX, double ldsY);

    virtual Type type() { return Surface; }

    static vector<ShaderIF::ShaderSpec> shaders;
    virtual void print(std::ostream&) const;
    int at(int i, int j) { return i * (nT + 1) + j; }
    void selectRow(int i);
    void selectCol(int j);
    friend std::ostream& operator<<(std::ostream&, const InteractiveSurface&);

  protected:
    void initGeometry(vector<AffPoint> ctrl_pts, vector<double> weights);
    virtual void p_clickReleased();
    virtual void p_checkForPick();
    virtual void p_update();
    virtual void p_render();
    virtual void p_renderPoly();

  private:
    GLuint ssboT, ssboS, ssboP, ssboDebug, vao, vbo, *ebo;
    int nS = 4, nT = 4;
    int kS = 4, kT = 4;
    bool test = false;
    // vector<vector<InteractivePoint*>> orderedPoints;
    vector<float> knotsS;
    vector<float> knotsT;

    PhongMaterial* mat_select = new PhongMaterial(PhongMaterial::polishedGold);
    PhongMaterial* mat_deselect = new PhongMaterial(PhongMaterial::polishedSilver);
};