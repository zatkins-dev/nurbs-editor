#ifndef CURVE_H
#define CURVE_H

#include "InteractiveAffPoint.h"
#include "SceneElement.h"

#include <fstream>
#include <set>
#include <string>
#include <vector>

using std::set;
using std::string;
using std::vector;

class Curve : public SceneElement {
  public:
    Curve(ShaderIF* sIF, ShaderIF* pointShader) : SceneElement(sIF), pointShader(pointShader) {}
    virtual ~Curve() {}
    virtual void updatePoint() = 0;
    virtual void clickReleased() {
        if (pickedPts.size())
            pickedPts.clear();
        moveAll = false;
        restrictToVector = false;
    }
    virtual void renderPolygon(ShaderIF*) = 0;
    bool handleCommand(unsigned char key, double ldsX, double ldsY);
    virtual void prepareForMove();
    virtual void restrictDrag(InteractiveAffPoint*) {}
    virtual std::ostream& operator<<(std::ostream&) const = 0;

    friend std::ostream& operator<<(std::ostream&, const Curve&);
    static vector<Curve*> readCurvesFromFile(string, ShaderIF*, ShaderIF*, ShaderIF*);
    static void writeCurves(std::ostream&);

  protected:
    ShaderIF* pointShader;
    static bool showPoints;
    static bool showPolygon;
    static bool moveAll;
    static bool restrictToVector;
    friend class InteractiveAffPoint;
    static set<InteractiveAffPoint*, bool (*)(InteractiveAffPoint*, InteractiveAffPoint*)>
        pickedPts;
    static string lastFilename;
};

#endif