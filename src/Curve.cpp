#include "Curve.h"

#include "BezierCurve.h"
#include "CardinalSpline.h"
bool closerPoint(InteractiveAffPoint* first, InteractiveAffPoint* second) {
    if (!second)
        return true;
    return first->distanceTo(InteractiveAffPoint::Q_mc) <
           second->distanceTo(InteractiveAffPoint::Q_mc);
}
bool (*iptCompare)(InteractiveAffPoint*, InteractiveAffPoint*) = closerPoint;
set<InteractiveAffPoint*, bool (*)(InteractiveAffPoint*, InteractiveAffPoint*)>
    Curve::pickedPts(iptCompare);

bool Curve::moveAll = false;
bool Curve::restrictToVector = false;
bool Curve::showPoints = true;

void Curve::prepareForMove() {
    if (clickMods & MVC_CTRL_BIT)
        moveAll = true; // Keep overlapping points overlapping

    if (clickMods & MVC_SHIFT_BIT)
        restrictToVector = true;
}

string Curve::lastFilename = "";
vector<Curve*> Curve::readCurvesFromFile(string fName, ShaderIF* sIFpoints, ShaderIF* sIFbezier,
                                         ShaderIF* sIFspline) {
    int countBezier = 0, countSpline = 0, countPoints = 0;
    lastFilename = fName;
    std::ifstream input;
    input.open(fName);
    vector<Curve*> crvs;
    while (true) {
        char next = input.peek();
        if (next == EOF)
            break;
        if (next == '\n') {
            input.seekg(1, input.cur);
            continue;
        }
        if (input.peek() == '#') {
            std::string comment;
            std::getline(input, comment);
            continue;
        }
        char type;
        input >> type;
        if (type == 'b') {
            AffPoint pt;
            InteractiveAffPoint* ipts[4];
            for (int i = 0; i < 4; i++) {
                input >> pt;
                ipts[i] = new InteractiveAffPoint(sIFpoints, pt);
                countPoints++;
            }
            crvs.push_back(new BezierCurve(sIFbezier, sIFpoints, ipts));
            countBezier++;
            // std::cerr << "[CURVE] Added bezier curve\n";
        } else if (type == 's') {
            int n;
            input >> n;
            float alpha;
            input >> alpha;
            AffPoint pt;
            vector<InteractiveAffPoint*> ipts;
            for (int i = 0; i < n; i++) {
                input >> pt;
                ipts.push_back(new InteractiveAffPoint(sIFpoints, pt));
                countPoints++;
            }
            crvs.push_back(new CardinalSpline(sIFspline, sIFpoints, ipts, alpha));
            countSpline++;
            // std::cerr << "[CURVE] Added cardinal spline\n";
        } else
            throw(std::runtime_error("Bad file format, check input."));
    }
    lastFilename = fName;
    string successMessage = "Successfully imported curves from '" + lastFilename + "'";
    string curveDetails = "Bezier Curves: " + std::to_string(countBezier) +
                          ", Cardinal Splines: " + std::to_string(countSpline) +
                          ", Total Points: " + std::to_string(countPoints);
    std::cout
        << "┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓\n";
    std::cout << "┃ " << successMessage << std::string(77 - successMessage.length(), ' ') << "┃\n";
    std::cout << "┃ " << curveDetails << std::string(77 - curveDetails.length(), ' ') << "┃\n";
    std::cout
        << "┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛\n";
    return crvs;
}

void Curve::writeCurves(std::ostream& os) {
    auto c = Controller::getCurrentController();
    int n = c->getNumModels();
    for (int i = 0; i < n; i++) {
        try {
            auto crv = reinterpret_cast<Curve*>(c->getModel(i));
            if (crv) {
                os << "# " << i + 1 << std::endl;
                os << *crv;
            }
        } catch (std::exception e) {
            // std::cout << e.what() << std::endl;
        }
    }
}

bool Curve::handleCommand(unsigned char key, double ldsX, double ldsY) {
    if (key == 's') {
        if (lastFilename.empty())
            lastFilename = "curves.cvs";
        std::cout << "enter new filename [" << lastFilename << "]: \n";

        string fname;
        std::getline(std::cin, fname);
        if (fname.empty())
            fname = lastFilename;
        else
            lastFilename = fname;
        std::ofstream output(fname);
        writeCurves(output);
        output.close();
        return false;
    }
    if (key == 'c') {
        showPoints = !showPoints;
        return false;
    }

    return true;
}

std::ostream& operator<<(std::ostream& os, const Curve& c) { return c.operator<<(os); }