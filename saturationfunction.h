#ifndef SATURATIONFUNCTION_H
#define SATURATIONFUNCTION_H

#include "energypeak.h"
#include <vector>
#include <cmath>
#include "TF1.h"
#include "TGraph.h"
#include "TSpline.h"

class SaturationFunction
{
public:
    SaturationFunction(const std::vector<EnergyPeak> &energyPeaks);
//    ~SaturationFunction();

//    double operator() (double *x, double *) {
//        double arg{x[0]};
//        double xx = _par.p.at(0) + _par.p.at(1) * arg;
////        if (arg < nodes_.front() || arg > nodes_.back()) {
////            return xx;
////        }
//        double L{_par.p.at(2)};
//        double k{_par.p.at(3)};
//        double x0{_par.p.at(4)};
//        double val = L / ( 1.0 + TMath::Exp( -1.0 * k * ( xx - x0 ) ) );
//        return val;
//    }

double operator() (double *x, double *) {
//    std::vector<double> xx{
//    284.583,
//    415.76 ,
//    733.577,
//    1412.52,
//    1891.06,
//    2274.95,
//    };

//    std::vector<double> y{
//        847.0,
//    1238.0,
//    2223.0,
//    4438.0,
//    6129.0,
//    7638.35,
//    };
//    std::vector<double> nodes{
//                nodes_.at(0),
//                nodes_.at(1),
//                nodes_.at(2)
//    };


//    TGraph gr(xx.size(), &xx[0], &nodes[0]);
//    TF1 f("f", "[0]+[1]*x", xx.front(), xx.back());
//    gr.Fit(&f, "RQN0");

//    TGraph gr_spline(xx.size(), &xx[0], &y[0]);
//    TSpline3 spline("spline", &gr_spline);
//    auto xxx = spline.Eval(x[0]);
    return spline_->Eval(f_->Eval(x[0]));
//    double p1 =  (xx.at(1) - xx.at(0)) / (nodes_.at(1) - nodes_.at(0));
//    double p0 = xx.at(1) - p1 * nodes_.at(1);

//    return spline.Eval(p0 + p1 * x[0]);
}

private:
    struct Par
    {
        std::vector<double> p;
    };

    std::vector<double> nodes_;

    Par _par;

    TSpline3 *spline_;
    TF1 *f_;

    Par par(const std::vector<EnergyPeak> &energyPeaks);
};

#endif // SATURATIONFUNCTION_H
