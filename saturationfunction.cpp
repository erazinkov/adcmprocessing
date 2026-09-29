#include "saturationfunction.h"

#include "TF1.h"
#include "TGraphErrors.h"
#include "TVirtualFitter.h"
#include "TSpline.h"

SaturationFunction::SaturationFunction(const std::vector<EnergyPeak> &energyPeaks)
{
    _par = par(energyPeaks);
}

//SaturationFunction::~SaturationFunction()
//{
//    if (spline_) {
//        delete spline_;
//        spline_ = nullptr;
//    }
//}

SaturationFunction::Par SaturationFunction::par(const std::vector<EnergyPeak> &energyPeaks)
{

    for (size_t i{0}; i < energyPeaks.size(); ++i) {
        nodes_.push_back(energyPeaks.at(i).channel());
    }
    std::sort(nodes_.begin(), nodes_.end());



    std::vector<double> xx{
    284.583,
    415.76 ,
    733.577,
    1412.52,
    1746.28,
    1890.82,
    2274.8,
    };

    std::vector<double> y{
        847.0,
    1238.0,
    2223.0,
    4438.0,
    5618.0,
    6129.0,
    7638.35,
    };
    std::vector<double> nodes{
                nodes_.at(0),
                nodes_.at(1),
                nodes_.at(2)
    };

    TGraph gr1(nodes.size(), &xx[0], &nodes[0]);
    f_ = new TF1("f", "[0]+[1]*x+[2]*x*x", xx.front(), xx.back());
    gr1.Fit(f_, "RQN0");

    TGraph gr(xx.size(), xx.data(), y.data());
    spline_ = new TSpline3("sp", &gr);

    double p1 =  (xx.at(1) - xx.at(0)) / (nodes_.at(1) - nodes_.at(0));
    double p0 = xx.at(1) - p1 * nodes_.at(1);

    std::cout << "reference: " << xx.at(0) << " " << "current: " << nodes_.at(0) << std::endl;
    std::cout << "reference: " << xx.at(1) << " " << "current: " << nodes_.at(1) << std::endl;
    std::cout << "p0: " << f_->GetParameter(0) << " " << "p1: " << f_->GetParameter(1) << std::endl;

    TVirtualFitter::SetDefaultFitter("Minuit");
    Par par;
//    auto ff = [] (double *x, double *par) {
//        double arg{x[0]};
//        double xx = par[0] + par[1] * arg;
//        double L{par[2]};
//        double k{par[3]};
//        double x0{par[4]};
//        double fitval = L / ( 1.0 + TMath::Exp( -1.0 * k * ( xx - x0 ) ) );

//        return fitval;
//    };

//    TF1 f{"f", ff, energyPeaks.front().channel(),  energyPeaks.back().channel(), 5};
////    TF1 f{"f", "[0]+[1]*x+[2]*x*x" , energyPeaks.front().channel(),  energyPeaks.back().channel()};
//    TGraphErrors g(static_cast<int>(energyPeaks.size()));

//    for (size_t i{0}; i < energyPeaks.size(); ++i) {
//        g.SetPoint(static_cast<int>(i), energyPeaks.at(i).channel(), energyPeaks.at(i).energy());
////        g.SetPointError(static_cast<int>(i), energyPeaks.at(i).channelErr(), TMath::Sqrt(energyPeaks.at(i).energy()) * 1.2);
//    }


//    // Initial parameter guesses based on your data
//    f.SetParameter(0, 0.0);        // intercept
//    f.SetParameter(1, 3.0);        // slope
//    f.SetParameter(2, 8'500.0);    // plateau (slightly above max y)
//    f.SetParameter(3, 0.001);      // steepness
//    f.SetParameter(4, 1'500.0);    // midpoint region

//    // Optional: set parameter limits
//    f.SetParLimits(0, -100.0, 100.0);
//    f.SetParLimits(1, 2.5, 3.5);
//    f.SetParLimits(2, 7'000.0, 15'000.0);  // L should be > max y
//    f.SetParLimits(3, 0, 1.0);             // k > 0

//    g.Fit(&f, "RN0");

//    for (auto i{0}; i < f.GetNpar(); ++i) {
//        std::cout << f.GetParameter(i) << " ";
//        par.p.push_back(f.GetParameter(i));
//    }
//    std::cout << std::endl;

    return par;
}
