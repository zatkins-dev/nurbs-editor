// SceneElement.c++
#include "SceneElement.h"

#include "OpenGLImageReader.h"

float SceneElement::lightPos[4 * MAX_NUM_LIGHTS] = {0.25, -0.5, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0};

// Are coordinates in "lightPos" stored in MC or EC?
bool SceneElement::lightPosInModelCoordinates[MAX_NUM_LIGHTS] = {true, true};
// The following is the buffer actually sent to GLSL. It will contain a copy of
// the (x,y,z,w) for light sources defined in EC; it will contain the coordinates
// after transformation to EC if the position was originally specified in MC.
float posToGLSL[4 * MAX_NUM_LIGHTS];

float SceneElement::lightStrength[3 * MAX_NUM_LIGHTS] = {0.5, 0.5, 0.5, 0.5, 0.5, 0.5};
float SceneElement::globalLightScale = 1.0; // a factor for 'lightStrength'

float SceneElement::globalAmbient[] = {0.5, 0.5, 0.5};

bool SceneElement::currentPickTriggerIsHover = false;
SceneElement* SceneElement::currentlyPickedObject = nullptr;

double SceneElement::minDist = -1;

AffPoint SceneElement::clickPos;
double SceneElement::ldsXprev, SceneElement::ldsYprev;
AffPoint SceneElement::Q_mc;
AffVector SceneElement::eHat_mc;

int SceneElement::clickMods = 0;

Matrix3x3 SceneElement::dragRestriction = Matrix3x3::IdentityMatrix;

const double SceneElement::clickThreshold = 0.05;

ShaderIFManager* SceneElement::shaderIFManager;

SceneElement::SceneElement(ShaderIF* sIF) : shaderIF(sIF) {}

SceneElement::~SceneElement() {}

void SceneElement::resetMCBoundingBox() {
    AffPoint(999999, 999999, 999999).aCoords(min);
    AffPoint(-999999, -999999, -999999).aCoords(max);
}

void SceneElement::updateMCBoundingBox(AffPoint xyz) {
    for (int i = 0; i < 3; i++) {
        if (xyz[i] < min[i])
            min[i] = xyz[i];
        if (xyz[i] > max[i])
            max[i] = xyz[i];
    }
}

void SceneElement::getMCBoundingBox(double* xyzLimits) const {
    for (int i = 0; i < 3; i++) {
        xyzLimits[2 * i] = min[i];
        xyzLimits[2 * i + 1] = max[i];
    }
}

void SceneElement::getECDeltas(double& dx_ec, double& dy_ec) {
    // Get MC deltas from region of interest:
    double dx = mcRegionOfInterest[1] - mcRegionOfInterest[0];
    double dy = mcRegionOfInterest[3] - mcRegionOfInterest[2];
    double dz = mcRegionOfInterest[5] - mcRegionOfInterest[4];

    double maxDelta = (dx > dy) ? dx : dy;
    if (dz > maxDelta)
        maxDelta = dz;

    // Use maxDelta as eye coordinate x and y deltas. Close enough...
    dx_ec = dy_ec = maxDelta / dynamic_zoomScale;

    if (aspectRatioPreservationEnabled) {
        double wAR = 1.0; // ecDeltaY / ecDeltaX; // height/width
        double vAR = Controller::getCurrentController()->getViewportAspectRatio();
        if (vAR > wAR)
            dy_ec = vAR * dx_ec;
        else
            dx_ec = dy_ec / vAR;
    }
}

// Pick/Drag callback methods

Controller::ClickResponse SceneElement::clickAtCB(Controller::MouseButton b, int mods, double ldsX,
                                                  double ldsY) {
    ldsXprev = ldsX;
    ldsYprev = ldsY;
    currentlyPickedObject = nullptr;
    minDist = -1;
    if (b == Controller::NO_BUTTON)
        currentPickTriggerIsHover = true;
    else if (b == Controller::RIGHT_BUTTON)
        currentPickTriggerIsHover = false;
    else // no picking
        return Controller::CR_ACK;

    getMCRayThroughLDS(ldsX, ldsY, Q_mc, eHat_mc);
    clickMods = mods;
    return Controller::CR_SEND_TO_ALL_MV_INSTANCES;
}

void SceneElement::clickReleasedCB(Controller::MouseButton b) {
    if (currentlyPickedObject)
        currentlyPickedObject->clickReleased();
    minDist = -1;
}

bool SceneElement::doneSendingClickToInstancesCB() {
    if (currentPickTriggerIsHover || (currentlyPickedObject == nullptr))
        return false; // don't  send position updates
    if (currentlyPickedObject)
        currentlyPickedObject->prepareForMove();
    return true; // drag the selected object
}

void SceneElement::reportMouseMovedToCB(double ldsX, double ldsY) {
    if (currentlyPickedObject != nullptr) {
        Matrix4x4 mc_ec, ec_lds;
        getMatrices(mc_ec, ec_lds);

        double ec_dx, ec_dy;
        getECDeltas(ec_dx, ec_dy);
        double ldsDX = ldsX - ldsXprev;
        double ldsDY = ldsY - ldsYprev;
        double ecDX = 0.5 * ldsDX * ec_dx;
        double ecDY = 0.5 * ldsDY * ec_dy;

        AffVector u(mc_ec.elementAt(0, 0), mc_ec.elementAt(0, 1), mc_ec.elementAt(0, 2));
        AffVector v(mc_ec.elementAt(1, 0), mc_ec.elementAt(1, 1), mc_ec.elementAt(1, 2));
        AffVector drag = ecDX * u + ecDY * v;
        currentlyPickedObject->moveBy(dragRestriction * drag);
    }
    ldsXprev = ldsX;
    ldsYprev = ldsY;
}

// END: Pick/Drag callback methods

GLuint SceneElement::defineTexture(const char* texImageSource) {
    if (texImageSource == nullptr) {
        std::cerr << "No texture given.\n";
        return 0;
    }
    ImageReader* texImage = ImageReader::create(texImageSource);
    if (texImage == nullptr) {
        std::cerr << "Could not create a texture from " << texImageSource << '\n';
        return 0;
    }
    OpenGLImageReader oglIR(texImage);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    GLuint locID[1];
    glGenTextures(1, locID);
    glBindTexture(GL_TEXTURE_2D, locID[0]);
    float borderColor[] = {0.0, 1.0, 1.0, 1.0};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    GLint level = 0;
    int pw = oglIR.getWidth(), ph = oglIR.getHeight();
    GLint iFormat = oglIR.getInternalFormat();
    GLenum format = oglIR.getFormat();
    GLenum type = oglIR.getType();
    const GLint border = 0; // must be zero (only present for backwards compatibility)
    const void* pixelData = oglIR.getTexture();
    glTexImage2D(GL_TEXTURE_2D, level, iFormat, pw, ph, border, format, type, pixelData);
    if (oglIR.getNumChannels() > 3) {
        std::cout << "Image has alpha channel; enabling blending.\n";
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    delete texImage; // all relevant information has been copied to OpenGL

    glBindTexture(GL_TEXTURE_2D, 0); // it will be bound again when needed

    return locID[0];
}

void SceneElement::establishLightingEnvironment() {
    glUniform1i(shaderIF->ppuLoc("actualNumLights"), MAX_NUM_LIGHTS);
    cryph::Matrix4x4 mc_ec, ec_lds;
    getMatrices(mc_ec, ec_lds);
    int basePosLoc = 0;
    int baseStrengthLoc = 0;
    float lStrength[3 * MAX_NUM_LIGHTS];
    for (int i = 0; i < MAX_NUM_LIGHTS; i++, basePosLoc += 4, baseStrengthLoc += 3) {
        if (lightPosInModelCoordinates[i])
            mc_ec.multiply(&lightPos[basePosLoc], &posToGLSL[basePosLoc]);
        else
            for (int k = 0; k < 4; k++)
                posToGLSL[basePosLoc + k] = lightPos[basePosLoc + k];
        for (int j = 0; j < 3; j++)
            lStrength[baseStrengthLoc + j] = globalLightScale * lightStrength[baseStrengthLoc + j];
    }
    glUniform4fv(shaderIF->ppuLoc("p_ecLightPosition"), MAX_NUM_LIGHTS, posToGLSL);
    glUniform3fv(shaderIF->ppuLoc("lightStrength"), MAX_NUM_LIGHTS, lStrength);
    glUniform3fv(shaderIF->ppuLoc("globalAmbient"), 1, globalAmbient);
}

void SceneElement::establishMaterialProperties(const PhongMaterial& matl) {
    glUniform3fv(shaderIF->ppuLoc("ka"), 1, matl.ka);
    glUniform3fv(shaderIF->ppuLoc("kd"), 1, matl.kd);
    glUniform3fv(shaderIF->ppuLoc("ks"), 1, matl.ks);
    glUniform1f(shaderIF->ppuLoc("m"), matl.m);
    glUniform1f(shaderIF->ppuLoc("alpha"), matl.alpha);
}

void SceneElement::establishTexture(int activeTexture, GLuint texID) {
    if (activeTexture >= 0) {
        if (texID > 0) {
            glActiveTexture(GL_TEXTURE0 + activeTexture);
            glBindTexture(GL_TEXTURE_2D, texID);
            // set the uniforms affecting the texture map
            // following can be done just once in defineTexture. Setting them here
            // allows them to be adjusted interactively.
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }
    }
}

void SceneElement::establishView() {
    // Line of sight, dynamic view controls, 3D-2D projection, & mapping to LDS:
    cryph::Matrix4x4 mc_ec, ec_lds;
    ModelView::getMatrices(mc_ec, ec_lds);
    float m[16];
    glUniformMatrix4fv(shaderIF->ppuLoc("mc_ec"), 1, false, mc_ec.extractColMajor(m));
    glUniformMatrix4fv(shaderIF->ppuLoc("ec_lds"), 1, false, ec_lds.extractColMajor(m));
}

bool SceneElement::handleCommand(unsigned char key, double ldsX, double ldsY) {
    if ((key >= '0') && (key <= '2'))
        return toggleLightDefinedInMC(static_cast<int>(key) - static_cast<int>('0'));
    if (key == 'g')
        globalLightScale /= 1.1;
    else if (key == 'G')
        globalLightScale *= 1.1;
    else
        return true; // not handled here; maybe someone else will
    return false;    // handled here; no one else should
}

bool SceneElement::handleCommand(Controller::SpecialKey key, double ldsX, double ldsY, int mods) {
    return ModelView::handleCommand(key, ldsX, ldsY, mods);
}

void SceneElement::printKeyboardKeyList(bool firstCall) const {
    if (!firstCall)
        return;
    ModelView::printKeyboardKeyList(firstCall);
    std::cout << "SceneElement:\n";
    std::cout << "\t0, 1, 2: toggle light i defined in EC<->MC\n";
    std::cout << "\tg - decrease global light scale\n";
    std::cout << "\tG - increase global light scale\n";
    std::cout << "\tt - cycle through texture application options\n";
}

void SceneElement::reportBoundingBox(std::ostream& os, std::string label, double xyz[]) {
    os << "Bounding box for " << label << ":\n";
    os << '\t' << xyz[0] << " <= x <= " << xyz[1] << '\n';
    os << '\t' << xyz[2] << " <= y <= " << xyz[3] << '\n';
    os << '\t' << xyz[4] << " <= z <= " << xyz[5] << "\n\n";
}

bool SceneElement::toggleLightDefinedInMC(int lightIndex) {
    if ((lightIndex < 0) || (lightIndex >= MAX_NUM_LIGHTS))
        return false;
    std::cout << "Switching light index " << lightIndex << " to ";
    if (lightPosInModelCoordinates[lightIndex])
        std::cout << "EC\n";
    else
        std::cout << "MC\n";
    lightPosInModelCoordinates[lightIndex] = !lightPosInModelCoordinates[lightIndex];
    return true;
}

void SceneElement::setShaderManager(ShaderIFManager* manager) { shaderIFManager = manager; }