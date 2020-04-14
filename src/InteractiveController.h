#pragma once

#include "GLFWController.h"
#include "interactive/Interactive.h"

#include <iostream>
#include <sstream>

class InteractiveController : public GLFWController {
  public:
    InteractiveController(const std::string& title, int flags = 0) : GLFWController(title, flags) {}
    virtual ~InteractiveController() {}
    virtual void updateMCBoundingBox();
    virtual void addModel(Interactive* imodel, int pos = 32767);
    virtual void addModel(SceneElement* imodel, int pos = 32767);
    vector<Interactive*> getInteractiveModels();
    virtual void removeModel(SceneElement* m);

    static InteractiveController* getCurrentController();
    void setModelsChanged() { modelsChanged = true; }
    bool saveAllToStream(std::ostream& os);
    bool loadAllFromStream(std::istream& is);

  protected:
    bool modelsChanged;
    virtual void renderAllModels();
    vector<Interactive*> interactiveModels;
};