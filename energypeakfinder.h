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
    struct PeakPos {
        double pos;
        double err;
    };
    double calib_;
    double offset_;

    std::vector<EnergyPeak>  energyPeaks_;
    EnergyPeak energyPeak_{EnergyPeak(EnergyPeak::Id::FE847, 0.0)};

    void findPeakPos(EnergyPeak &peak, TH1 *h);

    std::unique_ptr<TF1> fCalib_;

    std::map<EnergyPeak::Id, std::function<PeakPos(TH1 *hist, TH1 *histRc)>> findPeakPosFunctions_;

    double findPeakPosFerrum847Approx(TH1 *h);
    PeakPos findPeakPosFerrum847(TH1 *h, const double A);
    PeakPos findPeakPosFerrum1238(TH1 *h, const double A);
    PeakPos findPeakPosHydrogen(TH1 *h, const double A);
    PeakPos findPeakPosSilicon(TH1 *h, const double A);
    PeakPos findPeakPosCarbon(TH1 *h, const double A);
    PeakPos findPeakPosOxygen(TH1 *h, const double A);
    PeakPos findPeakPosFerrum7631(TH1 *h, const double A);

};

#endif // ENERGYPEAKFINDER_H
