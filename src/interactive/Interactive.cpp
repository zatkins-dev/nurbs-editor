#include "Interactive.h"

#include "../InteractiveController.h"
#include "InteractivePoint.h"

const PhongMaterial Interactive::selectedMat((float[]){1, 0.0, 0.0}, (float[]){1, 0, 0},
                                             (float[]){1, 0, 0}, 1);
const PhongMaterial Interactive::unselectedMat((float[]){0, 0, 1}, (float[]){0, 0, 1},
                                               (float[]){0, 0, 1}, 1);

std::ostream& operator<<(std::ostream& os, const Interactive& object) {
    object.print(os);
    return os;
}

void Interactive::prepareForMove() {
    p_prepareForMove();
    for (auto c : children)
        c->prepareForMove();
}

void Interactive::moveBy(AffVector dist) {
    p_moveBy(dist);
    dirtyBit = true;
    for (auto c : children)
        if (c->selected) {
            c->moveBy(dist);
            c->dirtyBit = true;
        }
}

void Interactive::clickReleased() {
    p_clickReleased();
    for (auto c : children)
        c->clickReleased();
}

void Interactive::checkForPick() {
    for (auto c : children)
        c->checkForPick();

    p_checkForPick();
}

bool Interactive::isSelected() const { return selected; }

void Interactive::clearSelection() {
    if (selected) {
        selected = false;
        for (auto c : children) {
            c->selected = false;
        }
        currentlyPickedObject = nullptr;
    }
};

void Interactive::selectAll() {
    for (auto c : children) {
        c->selected = true;
    }
    selected = true;
}

vector<InteractivePoint*> Interactive::getSelectedChildren() {
    vector<InteractivePoint*> selectedChildren;
    std::copy_if(children.begin(), children.end(), std::back_inserter(selectedChildren),
                 [](InteractivePoint* i) { return i ? i->isSelected() : false; });
    return selectedChildren;
}

void Interactive::update() {
    resetMCBoundingBox();
    for (auto c : children) {
        if (c->dirtyBit) {
            c->update();
        }
        updateMCBoundingBox(c->P());
    }
    p_update();
    if (!parent) {
        dynamic_cast<InteractiveController*>(Controller::getCurrentController())
            ->updateMCBoundingBox();
    }
    dirtyBit = false;
}

void Interactive::render() {
    if (dirtyBit)
        update();
    if (renderObject)
        p_render();
    if (renderPts)
        for (auto c : children) {
            c->render();
        }
    if (renderPoly)
        p_renderPoly();
}

Interactive* Interactive::getParent() { return parent; }

vector<InteractivePoint*> Interactive::getChildren() { return children; }