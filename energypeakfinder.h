#ifndef ENERGYPEAKFINDER_H
#define ENERGYPEAKFINDER_H

#include <TMath.h>
#include <TH1.h>
#include <TF1.h>

#include "energypeak.h"

class EnergyPeakFinder
{
public:
    EnergyPeakFinder();
    ~EnergyPeakFinder();
    void process(TH1D *hist, TH1D * histRc);
    void check(TH1D *hist, TH1D * histRc);
    void processRaw(TH1D *hist);
    const std::vector<EnergyPeak> &energyPeaks() const;

    const EnergyPeak &energyPeak() const;

    static std::pair<double, double> calculateResolution(TH1 *hist);

private:
    double calib_;
    double offset_;

    double getE(double ch) { return ch * calib_ + offset_; }
    double getdE(double dCh) { return dCh * calib_; }
    double getCh(double e) { return (e - offset_) / calib_; }
    double getdCh(double de) { return de / calib_; }


    double getFerrum7631Pos(TH1 *h, double appPos);

    std::vector<EnergyPeak>  energyPeaks_;
    EnergyPeak energyPeak_{EnergyPeak(EnergyPeak::Id::FE847, 0.0)};

    double getFerrum847PosApprox(TH1 *h);

    void findPeakPos(EnergyPeak &peak, TH1 *h);

    TF1 *fCalib_;

    std::map<EnergyPeak::Id, std::function<double(TH1 *hist, TH1 *histRc)>> findPeakPosFunctions_;

    double findPeakPosFerrum847Approx(TH1 *h, const double A);
    double findPeakPosFerrum847(TH1 *h, const double A);
    double findPeakPosFerrum1238(TH1 *h, const double A);
    double findPeakPosHydrogen(TH1 *h, const double A);
    double findPeakPosCarbon(TH1 *h, const double A);
    double findPeakPosOxygen(TH1 *h, const double A);

};

#endif // ENERGYPEAKFINDER_H
