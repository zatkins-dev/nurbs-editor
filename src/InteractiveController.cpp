#include "InteractiveController.h"

#include "interactive/InteractiveSurface.h"

void InteractiveController::updateMCBoundingBox() {
    overallMCBoundingBox[0] = overallMCBoundingBox[2] = overallMCBoundingBox[4] = 1.0;
    overallMCBoundingBox[1] = overallMCBoundingBox[3] = overallMCBoundingBox[5] = -1.0;
    for (auto m : models) {
        Controller::updateMCBoundingBox(m);
    }
}

void InteractiveController::addModel(Interactive* imodel, int pos) {
    if (pos > interactiveModels.size())
        interactiveModels.push_back(imodel);
    else {
        auto m = interactiveModels.begin();
        if (pos <= 0)
            interactiveModels.insert(m, imodel);
        else
            interactiveModels.insert(m + pos, imodel);
    }
    GLFWController::addModel(imodel, pos);
}

void InteractiveController::addModel(SceneElement* m, int pos) { GLFWController::addModel(m, pos); }

vector<Interactive*> InteractiveController::getInteractiveModels() { return interactiveModels; }

InteractiveController* InteractiveController::getCurrentController() {
    auto c = Controller::getCurrentController();
    if (c == nullptr)
        return nullptr;
    return static_cast<InteractiveController*>(c);
}

void InteractiveController::renderAllModels() {
    // draw the collection of models
    for (auto it : models)
        if (!modelsChanged)
            it->render();
        else {
            modelsChanged = false;
            break;
        }
}

void InteractiveController::removeModel(SceneElement* m) {
    for (auto it = interactiveModels.begin(); it < interactiveModels.end(); it++)
        if (m == *it)
            interactiveModels.erase(it);
    Controller::removeModel(m);
}

bool InteractiveController::saveAllToStream(std::ostream& os) {
    os.clear();
    os << interactiveModels.size() << std::endl;
    for (auto m : interactiveModels) {
        if (m->type() == Interactive::Curve)
            continue;
        os << "BEGIN" << std::endl;
        m->print(os);
    }
    return os.fail();
}

bool InteractiveController::loadAllFromStream(std::istream& is) {
    is.clear();
    int nModels;
    std::string n;
    std::getline(is, n);
    try {
        nModels = std::stoi(n);
    } catch (const std::invalid_argument& e) {
        std::cerr << "error: invalid format\n";
    }
    vector<Interactive*> newModels;

    for (int i = 0; i < nModels; i++) {
        std::string line;
        std::getline(is, line);
        if (line == " ")
            std::getline(is, line);
        if (line == "BEGIN") {
            std::getline(is, line);
            auto head = std::stringstream(line);
            char type;
            string name;
            head >> type;
            std::getline(head, name);

            if (type == 'S') {
                int nS, nT;
                int kS, kT;
                is >> nS >> nT >> kS >> kT;
                vector<AffPoint> ctrl_pts;
                vector<double> weights;
                for (int npts = 0; (npts < (nS + 1) * (nT + 1)); npts++) {
                    double x, y, z, w;
                    is >> x >> y >> z >> w;
                    ctrl_pts.push_back(AffPoint(x, y, z));
                    weights.push_back(w);
                }

                vector<float> knotsS;
                vector<float> knotsT;
                for (int j = 0; j <= nS + kS; j++) {
                    float s;
                    is >> s;
                    knotsS.push_back(s);
                }
                for (int j = 0; j <= nT + kT; j++) {
                    float t;
                    is >> t;
                    knotsT.push_back(t);
                }
                auto surf =
                    new InteractiveSurface(ctrl_pts, nS, nT, knotsS, knotsT, kS, kT, weights);
                surf->setName(name);
                newModels.push_back(surf);
            }
        }
    }
    for (auto* m : newModels) {
        addModel(m);
    }
    return true;
}