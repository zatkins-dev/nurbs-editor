#pragma once

#include "../SceneElement.h"
#include "../ShaderIFManager.h"
#include "AffPoint.h"
#include "ProjPoint.h"

#include <algorithm>

using cryph::AffPoint, cryph::ProjPoint;
using std::vector, std::string;

class InteractivePoint;

class Interactive : public SceneElement {
  public:
    enum Type {
        Point,
        Curve,
        Surface,
    };

    Interactive(ShaderIF* sIf) : SceneElement(sIf), parent(nullptr) {}
    virtual ~Interactive() {}
    virtual Type type() = 0;
    virtual string getName() const { return name; }
    virtual void setName(const string& _name) { name = _name; }
    virtual string& getNameRW() { return name; }
    // Interactivity
    virtual void prepareForMove();
    virtual void moveBy(AffVector dist);
    virtual void clickReleased();
    virtual void checkForPick();
    virtual bool isSelected() const;
    virtual void select() { selected = !selected; }
    virtual void selectAll();
    virtual vector<InteractivePoint*> getSelectedChildren();
    virtual Interactive* getParent();
    virtual vector<InteractivePoint*> getChildren();
    virtual void update();
    virtual void setParent(Interactive* p) { parent = p; }
    virtual void dirty() { dirtyBit = true; }
    virtual void clearSelection();
    // SceneElement
    virtual void render();
    virtual void print(std::ostream&) const {};

  protected:
    GLuint vaoPoly, vboPoly;
    Interactive* parent = nullptr;
    vector<InteractivePoint*> children;
    bool selected = false;
    bool dirtyBit = false;
    bool renderPts = true;
    bool renderPoly = true;
    bool renderObject = true;
    string name;

    virtual void p_prepareForMove(){};
    virtual void p_moveBy(AffVector dist){};
    virtual void p_clickReleased(){};
    virtual void p_checkForPick(){};
    virtual void p_update(){};
    virtual void p_render(){};
    virtual void p_renderPoly(){};

    const static PhongMaterial selectedMat;
    const static PhongMaterial unselectedMat;
};