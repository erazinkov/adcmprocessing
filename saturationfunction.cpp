#include "saturationfunction.h"

#include "TF1.h"
#include "TGraphErrors.h"
#include "TVirtualFitter.h"

SaturationFunction::SaturationFunction(const std::vector<EnergyPeak> &energyPeaks)
{
    _par = par(energyPeaks);
}

SaturationFunction::Par SaturationFunction::par(const std::vector<EnergyPeak> &energyPeaks)
{
    for (size_t i{0}; i < energyPeaks.size(); ++i) {
        nodes_.push_back(energyPeaks.at(i).channel());
    }
//    std::sort(nodes_.begin(), nodes_.end());
    TVirtualFitter::SetDefaultFitter("Minuit");
    Par par;
    auto ff = [] (double *x, double *par) {
        double arg{x[0]};
        double xx = par[0] + par[1] * arg;
        double L{par[2]};
        double k{par[3]};
        double x0{par[4]};
        double fitval = L / ( 1.0 + TMath::Exp( -1.0 * k * ( xx - x0 ) ) );

        return fitval;
    };

    TF1 f{"f", ff, energyPeaks.front().channel(),  energyPeaks.back().channel(), 5};
//    TF1 f{"f", "[0]+[1]*x+[2]*x*x" , energyPeaks.front().channel(),  energyPeaks.back().channel()};
    TGraphErrors g(static_cast<int>(energyPeaks.size()));

    for (size_t i{0}; i < energyPeaks.size(); ++i) {
        g.SetPoint(static_cast<int>(i), energyPeaks.at(i).channel(), energyPeaks.at(i).energy());
//        g.SetPointError(static_cast<int>(i), energyPeaks.at(i).channelErr(), TMath::Sqrt(energyPeaks.at(i).energy()) * 1.2);
    }


    // Initial parameter guesses based on your data
    f.SetParameter(0, 0.0);        // intercept
    f.SetParameter(1, 3.0);        // slope
    f.SetParameter(2, 8'500.0);    // plateau (slightly above max y)
    f.SetParameter(3, 0.001);      // steepness
    f.SetParameter(4, 1'500.0);    // midpoint region

    // Optional: set parameter limits
    f.SetParLimits(0, -100.0, 100.0);
    f.SetParLimits(1, 2.5, 3.5);
    f.SetParLimits(2, 7'000.0, 15'000.0);  // L should be > max y
    f.SetParLimits(3, 0, 1.0);             // k > 0

    g.Fit(&f, "RN0");

    for (auto i{0}; i < f.GetNpar(); ++i) {
        std::cout << f.GetParameter(i) << " ";
        par.p.push_back(f.GetParameter(i));
    }
    std::cout << std::endl;

    return par;
}
