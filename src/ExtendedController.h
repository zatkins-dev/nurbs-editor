#pragma once

#include "GLFWController.h"
#include "ModelView.h"

class ExtendedController : public GLFWController {
  public:
    ExtendedController(const std::string& title, int flags = 0) : GLFWController(title, flags) {}
    void updateMCBoundingBox() {
        overallMCBoundingBox[0] = overallMCBoundingBox[2] = overallMCBoundingBox[4] = 1.0;
        overallMCBoundingBox[1] = overallMCBoundingBox[3] = overallMCBoundingBox[5] = -1.0;
        for (auto m : models) {
            Controller::updateMCBoundingBox(m);
        }
    }

  protected:
    virtual void renderAllModels() {
        // draw the collection of models
        for (auto it : models)
            it->render();
    }
};