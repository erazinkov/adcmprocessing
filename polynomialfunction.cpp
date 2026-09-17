#include "polynomialfunction.h"

#include "TF1.h"
#include "TGraphErrors.h"

PolynomialFunction::PolynomialFunction(const std::vector<EnergyPeak> &energyPeaks)
{
    _par = par(energyPeaks);
}

PolynomialFunction::Par PolynomialFunction::par(const std::vector<EnergyPeak> &energyPeaks)
{
    Par par;
    TF1 f("f", "[0]+[1]*x+[2]*x*x" , energyPeaks.front().channel(),  energyPeaks.back().channel());
    TGraphErrors g(static_cast<int>(energyPeaks.size()));

    for (size_t i{0}; i < energyPeaks.size(); ++i) {
        g.SetPoint(static_cast<int>(i), energyPeaks.at(i).channel(), energyPeaks.at(i).energy());
        g.SetPointError(static_cast<int>(i), energyPeaks.at(i).channelErr(), 0.1);
    }
    g.Fit(&f, "RQN0");

    for (auto i{0}; i < f.GetNpar(); ++i) {
        par.p.push_back(f.GetParameter(i));
    }

    return par;
}
