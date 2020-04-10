// SceneElement.h - a base class that factors out many common data and
//                  method implementations to support a Phong local
//                  lighting model.

#ifndef SCENEELEMENT_H
#define SCENEELEMENT_H
#ifdef __APPLE__
#include "GLFW/glfw3.h"
#else
#if !defined(GL_GLEXT_PROTOTYPES)
#define GL_GLEXT_PROTOTYPES
#endif // GL_GLEXT_PROTOTYPES

#include <GL/gl.h>
#include <GL/glext.h>
#endif

#include "ModelView.h"
#include "PhongMaterial.h"
#include "ShaderIF.h"
#include "ShaderIFManager.h"

#include "Matrix3x3.h"

using namespace cryph;
typedef float vec3[3];

static const int MAX_NUM_LIGHTS = 3; // MUST BE KEPT SAME AS IN the shader program

class SceneElement : public ModelView {
  public:
    // The "sIF" parameter pointer is retained. Caller must ensure that it lives
    // for the life of this SceneElement instance.
    SceneElement(ShaderIF* sIF);
    virtual ~SceneElement();

    void establishLightingEnvironment();
    void establishMaterialProperties(const PhongMaterial& matl);
    void establishTexture(int activeTexture, GLuint texID);
    void establishView();
    bool handleCommand(unsigned char key, double ldsX, double ldsY);
    bool handleCommand(Controller::SpecialKey key, double ldsX, double ldsY, int mods);
    void printKeyboardKeyList(bool firstCall) const;
    bool toggleLightDefinedInMC(int lightIndex);

    static GLuint defineTexture(const char* texImageSource);
    static void reportBoundingBox(std::ostream& os, std::string label, double xyz[]);

    // callbacks for picking and dragging
    static Controller::ClickResponse clickAtCB(Controller::MouseButton b, int mods, double ldsX,
                                               double ldsY);
    static void clickReleasedCB(Controller::MouseButton b);
    static bool doneSendingClickToInstancesCB();
    static void reportMouseMovedToCB(double ldsX, double ldsY);
    // END: callbacks for picking and dragging

    void resetMCBoundingBox();
    void updateMCBoundingBox(AffPoint pt);
    void getMCBoundingBox(double* xyzLimits) const;
    virtual void prepareForMove() {}
    // move object by vector
    virtual void moveBy(AffVector dist) {}
    virtual void clickReleased() {}
    static void setShaderManager(ShaderIFManager* sIFmanager);

  protected:
    static void getECDeltas(double& ec_dx, double& ec_dy);
    static double ldsXprev, ldsYprev;
    ShaderIF* shaderIF;
    static ShaderIFManager* shaderIFManager;
    static bool currentPickTriggerIsHover;
    static SceneElement* currentlyPickedObject;
    static AffPoint clickPos;
    static AffPoint Q_mc;
    static AffVector eHat_mc;
    static int clickMods;
    static double minDist;
    static Matrix3x3 dragRestriction;
    const static double clickThreshold; // MC
    vec3 min{0, 0, 0}, max{0, 0, 0};

  private:
    // lighting environment
    static float lightPos[4 * MAX_NUM_LIGHTS];              // (x,y,z,w) for each light
    static bool lightPosInModelCoordinates[MAX_NUM_LIGHTS]; // pos is in MC or EC?
    static float lightStrength[3 * MAX_NUM_LIGHTS];         // (r,g,b) for each light
    static float globalLightScale;                          // factor for 'lightStrength'
    static float globalAmbient[3];                          // (r,g,b) for ambient term, A
};

#endif
