#include "polynomialfunction.h"

#include "TF1.h"
#include "TGraphErrors.h"
#include "TVirtualFitter.h"

PolynomialFunction::PolynomialFunction(const std::vector<EnergyPeak> &energyPeaks)
{
    _par = par(energyPeaks);
}

PolynomialFunction::Par PolynomialFunction::par(const std::vector<EnergyPeak> &energyPeaks)
{
    TVirtualFitter::SetDefaultFitter("Minuit");
    Par par;
    auto ff = [] (double *x, double *par) {
        double arg{x[0]};
        double fitval = par[0] + par[1] * arg + par[2] * arg * arg;

        return fitval;
    };

    TF1 f{"f", ff , energyPeaks.front().channel(),  energyPeaks.back().channel(), 3};

    TGraphErrors g(static_cast<int>(energyPeaks.size()));

    for (size_t i{0}; i < energyPeaks.size(); ++i) {
        g.SetPoint(static_cast<int>(i), energyPeaks.at(i).channel(), energyPeaks.at(i).energy());
//        g.SetPointError(static_cast<int>(i), energyPeaks.at(i).channelErr(), TMath::Sqrt(energyPeaks.at(i).energy()) * 1.2);
    }
//    f.SetParameter(2, 0.0001);
//    f.SetParLimits(2, 0.0001, 0.0002);
    g.Fit(&f, "RN0");

    for (auto i{0}; i < f.GetNpar(); ++i) {
        std::cout << f.GetParameter(i) << " ";
        par.p.push_back(f.GetParameter(i));
    }
    std::cout << std::endl;

    return par;
}
