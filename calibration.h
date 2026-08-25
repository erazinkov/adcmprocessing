#ifndef CALIBRATION_H
#define CALIBRATION_H

#include "adcm_df.h"

#include "histogrammanager.h"
#include "energypeak.h"
#include "energypeakfinder.h"

#include "decoder.h"

class Calibration
{
public:
    Calibration(HistogramManager *histogramManager);

    void process();
    void setNewData(const std::unordered_map<std::pair<uint8_t, uint8_t>,
                          std::vector<dec_ev_t>, PairHash> &events,
                    const dec_ch_t &channels,
                    double time,
                    const std::map<uint8_t, uint32_t> &counters);
    double time() const;

    const std::map<uint8_t, double> &counters() const;

    const std::map<uint8_t, double> &countersG() const;

    const std::map<uint8_t, double> &countersA() const;

private:
    HistogramManager *histogramManager_;
    std::map<std::pair<uint8_t, uint8_t>, std::vector<dec_ev_t>> events_m_;
    double time_;
    std::map<uint8_t, double> counters_;
    std::map<uint8_t, double> countersG_;
    std::map<uint8_t, double> countersA_;
    std::map<std::pair<uint8_t, uint8_t>, double> timeCorrections_;
    dec_ch_t channels_;
    std::vector<std::vector<EnergyPeak>>  energyPeaks_;
    std::vector<EnergyPeak>  energyPeaksRaw_;

    EnergyPeakFinder energyPeakFinder_;

    void fillHistTime(const std::vector<dec_ev_t> &events, TH1 *h, double correction);
    void fillHistTimeWithEnergyCut(const std::vector<dec_ev_t> &events, TH1 *h, double correction, double minE, double maxE, TF1 f);

    void fillHistAmp(const std::vector<dec_ev_t> &events, TH1 *h, double minT, double maxT, bool exclude);
    void fillHistEnergy(const std::vector<dec_ev_t> &events, TH1 *h, double minT, double maxT, bool exclude, TF1 f);

    void fillHistsTimeByGammaAlpha(const std::vector<std::vector<TH1D *>> &hists, bool isCorrected = false);
    void fillHistsTimeWithEnergyCutByGammaAlpha(const std::vector<std::vector<TH1D *>> &hists);
    void fillHistTimeWithEnergyCutTotal(const std::vector<std::vector<TH1D *>> &hists);
    void fillHistsTimeWithEnergyCutByGamma(const std::vector<std::vector<TH1D *>> &hists);
    void fillHistsTimeByAlpha(const std::vector<std::vector<TH1D *>> &hists);

    void fillHistsAmpByGammaAlpha(const std::vector<std::vector<TH1D *> > &histsSg,
                                  const std::vector<std::vector<TH1D *> > &histsBg,
                                  const std::vector<std::vector<TH1D *> > &histsRc);
    void fillHistsAmpByAlpha(const std::vector<std::vector<TH1D *>> &histsSg, const std::vector<std::vector<TH1D *> > &histsBg);
    void fillHistsAmpByGamma(const std::vector<std::vector<TH1D *> > &histsSg,
                             const std::vector<std::vector<TH1D *> > &histsBg,
                             const std::vector<std::vector<TH1D *> > &histsRc);


    void fillHistsEnergyByGammaAlpha(const std::vector<std::vector<TH1D *> > &histsSg,
                                  const std::vector<std::vector<TH1D *> > &histsBg);

    void fillHistsEnergyByGamma(const std::vector<std::vector<TH1D *> > &histsSg,
                                  const std::vector<std::vector<TH1D *> > &histsBg);
    void fillHistsEnergyByAlpha(const std::vector<std::vector<TH1D *> > &histsSg,
                                  const std::vector<std::vector<TH1D *> > &histsBg);

};

#endif // CALIBRATION_H
