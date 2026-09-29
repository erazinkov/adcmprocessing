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
    std::vector<double> xx{
        282.6,
    412.108,
    727.323,
    1402.03,
    1881.01,
    2263.03,
    };

    std::vector<double> y{
        847.0,
    1238.0,
    2223.0,
    4438.0,
    6129.0,
    7638.35,
    };
    TGraph gr(xx.size(), &xx[0], &y[0]);
    TSpline3 spline("spline", &gr);
    double p1 =  (xx.at(1) - xx.at(0)) / (nodes_.at(1) - nodes_.at(0));
    double p0 = xx.at(1) - p1 * nodes_.at(1);

    return spline.Eval(p0 + p1 * x[0]);
}

private:
    struct Par
    {
        std::vector<double> p;
    };

    std::vector<double> nodes_;

    Par _par;

    Par par(const std::vector<EnergyPeak> &energyPeaks);
};

#endif // SATURATIONFUNCTION_H
