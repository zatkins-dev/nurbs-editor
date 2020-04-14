// main.c++ - main program for complex square root visualization

#include "ImGUIMenu.h"

#include <fstream>
#include <stdlib.h>

#include "InteractiveController.h"
#include "SceneElement.h"
#include "ShaderIF.h"
#include "interactive/InteractiveCurve.h"
#include "interactive/InteractiveSurface.h"

using std::string;

void createNewScene(InteractiveController& c, string filename = "") {
    // vector<AffPoint> pts{
    //     AffPoint(-1, 0, 0), AffPoint(-1, 1, 0), AffPoint(0, 1, 0),
    //     AffPoint(1, 1, 0),  AffPoint(1, 0, 0),
    // };
    // InteractiveCurve* crv = new InteractiveCurve(pts);

    // c.addModel(crv);

    // vector<AffPoint> pts1{
    //     AffPoint(-1, 0, 1),
    //     AffPoint(-1, 1, 1),
    //     AffPoint(0, 1, 1),
    //     AffPoint(1, 1, 1),
    // };
    // InteractiveCurve* crv1 = new InteractiveCurve(pts1, vector<double>(4, 1.0), true);
    auto surf = new InteractiveSurface();
    c.addModel(surf);
}

void set3DViewingInformation(double xyz[6]) {
    // Simplest case: Just tell the ModelView we want to see it all:
    ModelView::setMCRegionOfInterest(xyz);

    // Set center
    double xmid = 0.5 * (xyz[0] + xyz[1]);
    double ymid = 0.5 * (xyz[2] + xyz[3]);
    double zmid = 0.5 * (xyz[4] + xyz[5]);
    cryph::AffPoint center(xmid, ymid, zmid);

    // Set eye
    double maxDelta = xyz[1] - xyz[0];
    double delta = xyz[3] - xyz[2];
    if (delta > maxDelta)
        maxDelta = delta;
    delta = xyz[5] - xyz[4];
    if (delta > maxDelta)
        maxDelta = delta;
    double distEyeCenter = 2.0 * maxDelta;
    cryph::AffPoint eye(xmid, ymid, zmid + distEyeCenter);

    // Set up
    cryph::AffVector up = cryph::AffVector::yu;

    // Notify the ModelView of our MC->EC viewing requests:
    ModelView::setEyeCenterUp(eye, center, up);

    // Perspective parameters
    double zpp = -(distEyeCenter - 0.5 * maxDelta);
    ModelView::setProjectionPlaneZ(zpp);
    double zmin = zpp - 2.0 * maxDelta;
    double zmax = zpp + 0.5 * maxDelta;
    ModelView::setECZminZmax(zmin, zmax);
}

void initGUI(InteractiveController& c) { c.addModel(new ImGUIMenu()); }

void initShaders() {
    ShaderIFManager* manager = new ShaderIFManager();
    manager->createShader("point", InteractivePoint::shaders.data(),
                          InteractivePoint::shaders.size());
    manager->createShader("nurbsCurve", InteractiveCurve::shaders.data(),
                          InteractiveCurve::shaders.size());
    manager->createShader("nurbsSurface", InteractiveSurface::shaders.data(),
                          InteractiveSurface::shaders.size());
    SceneElement::setShaderManager(manager);
}

int main(int argc, char* argv[]) {
    InteractiveController c("Surface Editor", MVC_USE_DEPTH_BIT);
    c.reportVersions(std::cout);
    // ShaderIF* sIF = new ShaderIF(BezierCurve::bezShaders, 4);
    // ShaderIF* sIFpoints = new ShaderIF("shaders/basic.vert", "shaders/phong.frag");
    // ShaderIF* sIFbezier = new ShaderIF(BezierCurve::bezShaders, 4);
    // ShaderIF* sIFspline = new ShaderIF(CardinalSpline::splineShaders, 4);

    // createScene(c, sIFpoints, sIFbezier, sIFspline, fname);
    initShaders();
    string fname;
    if (argc > 1) {
        fname = argv[1];
        std::cout << "Provided file: " << fname << '\n';
        std::ifstream is(fname);
        c.loadAllFromStream(is);
        is.close();
    } else
        createNewScene(c);
    glClearColor(1.0, 1.0, 1.0, 1.0);

    // callbacks for picking and dragging
    Controller::setClickAtCB(SceneElement::clickAtCB);
    Controller::setClickReleasedCB(SceneElement::clickReleasedCB);
    Controller::setDoneSendingClickToInstancesCB(SceneElement::doneSendingClickToInstancesCB);
    Controller::setReportMouseMovedToCB(SceneElement::reportMouseMovedToCB);
    // END: callbacks for picking and dragging

    double xyz[6]; // xyz limits, even though this is 2D
    c.getOverallMCBoundingBox(xyz);
    std::cout << "Bounding box: " << xyz[0] << " <= x <= " << xyz[1] << '\n';
    std::cout << "              " << xyz[2] << " <= y <= " << xyz[3] << '\n';
    std::cout << "              " << xyz[4] << " <= z <= " << xyz[5] << "\n\n";
    set3DViewingInformation(xyz);
    // c.setDebug(true);
    SceneElement::setProjection(ORTHOGONAL);
    initGUI(c);
    c.run();

    return 0;
}
