#include "Interactive.h"
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

vector<InteractivePoint*> Interactive::getSelectedChildren() {
    vector<InteractivePoint*> selectedChildren;
    std::copy_if(children.begin(), children.end(), std::back_inserter(selectedChildren),
                 [](InteractivePoint* i) { return i ? i->isSelected() : false; });
    return selectedChildren;
}

void Interactive::update() {
    for (auto c : children)
        if (c->dirtyBit) {
            c->update();
            c->dirtyBit = false;
        }
    p_update();
    dirtyBit = false;
}

void Interactive::render() {
    if (dirtyBit)
        update();
    for (auto c : children) {
        c->render();
    }
    p_render();
}

Interactive* Interactive::getParent() { return parent; }

vector<InteractivePoint*> Interactive::getChildren() { return children; }