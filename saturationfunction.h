#ifndef SATURATIONFUNCTION_H
#define SATURATIONFUNCTION_H

#include "energypeak.h"
#include <vector>
#include <cmath>
#include "TF1.h"

class SaturationFunction
{
public:
    SaturationFunction(const std::vector<EnergyPeak> &energyPeaks);

    double operator() (double *x, double *) {
        double arg{x[0]};
        double xx = _par.p.at(0) + _par.p.at(1) * arg;
//        if (arg < nodes_.front() || arg > nodes_.back()) {
//            return xx;
//        }
        double L{_par.p.at(2)};
        double k{_par.p.at(3)};
        double x0{_par.p.at(4)};
        double val = L / ( 1.0 + TMath::Exp( -1.0 * k * ( xx - x0 ) ) );
        return val;
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
