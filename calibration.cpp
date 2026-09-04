#include "calibration.h"
#include "utils.h"
#include "piecewiselinearfunction.h"
#include "timepeaksfinder.h"
#include "resolutionprocessing.h"

Calibration::Calibration(HistogramManager *histogramManager)
    : histogramManager_{histogramManager}
{
    for (auto ig{0}; ig < histogramManager->gammaNumber(); ++ig) {
        for (auto ia{0}; ia < histogramManager->alphaNumber(); ++ia) {
            std::pair<uint8_t, uint8_t> p{ig, ia};
            timeCorrections_[p] = 0.0;
        }
    }
}

void Calibration::process()
{

    fillHistsTimeByGammaAlpha(histogramManager_->histsTimeByGammaAlpha(), false);

    const auto igMax = std::min(histogramManager_->histsTimeByGammaAlpha().size(), channels_.g.size());
    const auto iaMax = std::min(histogramManager_->histsTimeByGammaAlpha().at(0).size(), channels_.a.size());
    for (size_t ig{0}; ig < igMax; ++ig) {
        for (size_t ia{0}; ia <  iaMax; ++ia) {
            std::pair<uint8_t, uint8_t> p{ig, ia};
            timeCorrections_[p] = TimePeaksFinder::calculatePeakPos(histogramManager_->histsTimeByGammaAlpha().at(ig).at(ia));
        }
    }

    fillHistsTimeByGammaAlpha(histogramManager_->histsTimeCorrectedByGammaAlpha(), true);

    fillHistsAmpByGammaAlpha(histogramManager_->histsAmpByGammaAlphaSg(), histogramManager_->histsAmpByGammaAlphaBg(), histogramManager_->histsAmpByGammaAlphaRc());

    fillHistsAmpByGamma(histogramManager_->histsAmpByGammaAlphaSg(), histogramManager_->histsAmpByGammaAlphaBg(), histogramManager_->histsAmpByGammaAlphaRc());

    energyPeaks_.clear();

    for (size_t i{0}; i < std::min(histogramManager_->histsAmpByGamma().size(), channels_.g.size()); ++i) {
        energyPeakFinder_.process(histogramManager_->histsAmpByGamma().at(i), histogramManager_->histsAmpByGammaRc().at(i));
        energyPeaks_.push_back(energyPeakFinder_.energyPeaks());
    }



    fillHistsEnergyByGammaAlpha(histogramManager_->histsEnergyByGammaAlphaSg(), histogramManager_->histsEnergyByGammaAlphaBg());
    fillHistsEnergyByGamma(histogramManager_->histsEnergyByGammaAlphaSg(), histogramManager_->histsEnergyByGammaAlphaBg());
  for (size_t i{0}; i < std::min(histogramManager_->histsAmpByGamma().size(), channels_.g.size()); ++i) {
      energyPeakFinder_.check(histogramManager_->histsEnergyByGamma().at(i), histogramManager_->histsEnergyByGamma().at(i));
  }
    // !

//    ResolutionProcessing rP;
//    rP.processingEnergy(AppConstants::OUTPUT_PATH + fileName_, histogramManager_->histsEnergyByGamma(), channels_.g.size());
//    fillHistsEnergyByAlpha(histogramManager_->histsEnergyByGammaAlphaSg(), histogramManager_->histsEnergyByGammaAlphaBg());
//    fillHistsTimeByAlpha(histogramManager_->histsTimeCorrectedByGammaAlpha());

//    fillHistsTimeWithEnergyCutByGammaAlpha(histogramManager_->histsTimeByGammaAlpha());
//    fillHistsTimeWithEnergyCutByGamma(histogramManager_->histsTimeByGammaAlpha());
//    rP.processingTime(AppConstants::OUTPUT_PATH + fileName_, histogramManager_->histsTimeCorrectedByGamma(), channels_.g.size());
//    rP.exportToCSV(AppConstants::OUTPUT_PATH + fileName_);

}


void Calibration::fillHistsTimeByGammaAlpha(const std::vector<std::vector<TH1D *> > &hists, bool isCorrected)
{
    std::vector<std::function<void()>> tasks;
    for (size_t i{0}; i < std::min(hists.size(), channels_.g.size()); ++i) {
        for (size_t j{0}; j <  std::min(hists.size(), channels_.a.size()); ++j) {
            tasks.push_back([this, &hists, i, j, isCorrected](){
                hists.at(i).at(j)->Reset();
                std::pair<uint8_t, uint8_t> p{*std::next(channels_.g.begin(), i), *std::next(channels_.a.begin(), j)};
                if (events_m_.find(p) != events_m_.end()) {
                    fillHistTime(events_m_.at(p), hists.at(i).at(j), isCorrected ? timeCorrections_[{i, j}] : 0.0);
                }
            });
        }
    }
    func_async(tasks.begin(), tasks.end());
    tasks.clear();
}

void Calibration::fillHistsTimeWithEnergyCutByGammaAlpha(const std::vector<std::vector<TH1D *> > &hists)
{
    std::vector<TF1> fs;
    for (size_t i{0}; i < std::min(hists.size(), channels_.g.size()); ++i) {
        PiecewiseLinearFunction fObj(energyPeaks_.at(i));
        TF1 f("f", fObj, 0, 4'000, 0);
        fs.push_back(f);
    }
    std::vector<std::function<void()>> tasks;
    for (size_t i{0}; i < std::min(hists.size(), channels_.g.size()); ++i) {
        for (size_t j{0}; j <  std::min(hists.size(), channels_.a.size()); ++j) {
            tasks.push_back([this, &hists, i, j, &fs](){
                hists.at(i).at(j)->Reset();
                std::pair<uint8_t, uint8_t> p{*std::next(channels_.g.begin(), i), *std::next(channels_.a.begin(), j)};
                if (events_m_.find(p) != events_m_.end()) {
                    fillHistTimeWithEnergyCut(events_m_.at(p),
                                              hists.at(i).at(j),
                                              timeCorrections_.at({i, j}),
                                              4438.0 - 250.0,
                                              4438.0 + 250.0,
                                              fs.at(i));
                }
            });
        }
    }
    func_async(tasks.begin(), tasks.end());
    tasks.clear();
}

void Calibration::fillHistTimeWithEnergyCutTotal(const std::vector<std::vector<TH1D *> > &hists)
{
    histogramManager_->histTimeTotal()->Reset();
    for (size_t i{0}; i < hists.size(); ++i) {
        for (size_t j{0}; j <  hists.at(i).size(); ++j) {
            histogramManager_->histTimeTotal()->Add(hists.at(i).at(j));
        }
    }
}

void Calibration::fillHistsTimeWithEnergyCutByGamma(const std::vector<std::vector<TH1D *> > &hists)
{
    for (size_t i{0}; i <  histogramManager_->histsTimeCorrectedByGamma().size(); ++i) {
        histogramManager_->histsTimeCorrectedByGamma()[i]->Reset();
    }
    for (size_t i{0}; i < hists.size(); ++i) {
        for (size_t j{0}; j <  hists.at(i).size(); ++j) {
            histogramManager_->histsTimeCorrectedByGamma()[i]->Add(hists.at(i).at(j));
        }
    }
}

void Calibration::fillHistTime(const std::vector<dec_ev_t> &events, TH1 *h, double correction)
{
    for (const auto & item : events) {
        h->Fill(item.tdc - correction);
    }
}

void Calibration::fillHistTimeWithEnergyCut(const std::vector<dec_ev_t> &events,
                                            TH1 *h,
                                            double correction,
                                            double minE,
                                            double maxE,
                                            TF1 f)
{
    for (const auto & item : events) {
        auto t{static_cast<double>(item.tdc)};
        auto a{static_cast<double>(item.amp)};
        if (minE <= f.Eval(a) && f.Eval(a) <= maxE) {
            h->Fill(t - correction);
        }

    }
}

void Calibration::fillHistAmp(const std::vector<dec_ev_t> &events, TH1 *h, double minT, double maxT, bool exclude)
{
    for (const auto & item : events) {
        auto t{static_cast<double>(item.tdc)};
        auto a{static_cast<double>(item.amp)};
        if (exclude) {
            if (t < minT || maxT < t) {
                h->Fill(a);
            }
        }
        else {
            if (minT <= t && t <= maxT) {
                h->Fill(a);
            }
        }
    }
}

void Calibration::fillHistEnergy(const std::vector<dec_ev_t> &events, TH1 *h, double minT, double maxT, bool exclude, TF1 f)
{
    for (const auto & item : events) {
        auto t{static_cast<double>(item.tdc)};
        auto a{static_cast<double>(item.amp)};
        if (exclude) {
            if (t < minT || maxT < t) {
                h->Fill(f.Eval(a));
            }
        }
        else {
            if (minT <= t && t <= maxT) {
                h->Fill(f.Eval(a));
            }
        }
    }
}

void Calibration::fillHistsTimeByAlpha(const std::vector<std::vector<TH1D *> > &hists)
{
    for (size_t i{0}; i <  histogramManager_->histsTimeCorrectedByAlpha().size(); ++i) {
        histogramManager_->histsTimeCorrectedByAlpha()[i]->Reset();
    }
    for (size_t i{0}; i < hists.size(); ++i) {
        for (size_t j{0}; j <  hists.at(i).size(); ++j) {
            histogramManager_->histsTimeCorrectedByAlpha()[j]->Add(hists.at(i).at(j));
        }
    }
}

void Calibration::fillHistsAmpByGammaAlpha(const std::vector<std::vector<TH1D *> > &histsSg,
                                           const std::vector<std::vector<TH1D *> > &histsBg,
                                           const std::vector<std::vector<TH1D *> > &histsRc)
{
    std::vector<std::function<void()>> tasks;
    for (size_t i{0}; i < std::min(histsSg.size(), channels_.g.size()); ++i) {
        for (size_t j{0}; j <  std::min(histsSg.at(i).size(), channels_.a.size()); ++j) {
            tasks.push_back([this, &histsSg, &histsBg, &histsRc, i, j](){
                histsSg.at(i).at(j)->Reset();
                histsBg.at(i).at(j)->Reset();
                histsRc.at(i).at(j)->Reset();
                std::pair<uint8_t, uint8_t> p{*std::next(channels_.g.begin(), i), *std::next(channels_.a.begin(), j)};
                if (events_m_.find(p) != events_m_.end()) {
                    auto minT_sg{timeCorrections_.at({i, j}) - 3.0};
                    auto maxT_sg{timeCorrections_.at({i, j}) + 3.0};
                    fillHistAmp(events_m_.at(p), histsSg.at(i).at(j), minT_sg, maxT_sg, false);
                    fillHistAmp(events_m_.at(p), histsRc.at(i).at(j), minT_sg, maxT_sg, true);
                    auto minT_bg{timeCorrections_.at({i, j}) - 30.0};
                    auto maxT_bg{timeCorrections_.at({i, j}) - 20.0};
                    fillHistAmp(events_m_.at(p), histsBg.at(i).at(j), minT_bg, maxT_bg, false);
                }
            });
        }
    }
    func_async(tasks.begin(), tasks.end());
    tasks.clear();
}



void Calibration::fillHistsAmpByAlpha(const std::vector<std::vector<TH1D *>> &histsSg, const std::vector<std::vector<TH1D *> > &histsBg)
{
    for (size_t i{0}; i <  histogramManager_->histsAmpByAlpha().size(); ++i) {
        histogramManager_->histsAmpByAlpha()[i]->Reset();
    }
    for (size_t i{0}; i < histsSg.size(); ++i) {
        for (size_t j{0}; j <  histsSg.at(i).size(); ++j) {
            histogramManager_->histsAmpByAlpha()[j]->Add(histsSg.at(i).at(j));
            histogramManager_->histsAmpByAlpha()[j]->Add(histsBg.at(i).at(j), -1.0 * 6.0 / 10.0);
        }
    }
}

void Calibration::fillHistsAmpByGamma(const std::vector<std::vector<TH1D *> > &histsSg,
                                      const std::vector<std::vector<TH1D *> > &histsBg,
                                      const std::vector<std::vector<TH1D *> > &histsRc)
{
    for (size_t i{0}; i <  histogramManager_->histsAmpByGamma().size(); ++i) {
        histogramManager_->histsAmpByGamma()[i]->Reset();
        histogramManager_->histsAmpByGammaRc()[i]->Reset();
    }
    for (size_t i{0}; i < std::min(histsSg.size(), channels_.g.size()); ++i) {
        for (size_t j{0}; j <  std::min(histsSg.at(i).size(), channels_.a.size()); ++j) {
            histogramManager_->histsAmpByGamma()[i]->Add(histsSg.at(i).at(j));
            histogramManager_->histsAmpByGamma()[i]->Add(histsBg.at(i).at(j), -1.0 * 6.0 / 10.0);

            histogramManager_->histsAmpByGammaRc()[i]->Add(histsRc.at(i).at(j));
        }
    }
}

void Calibration::fillHistsEnergyByGammaAlpha(const std::vector<std::vector<TH1D *> > &histsSg, const std::vector<std::vector<TH1D *> > &histsBg)
{
    std::vector<TF1> fs;
    for (size_t i{0}; i < std::min(histsSg.size(), channels_.g.size()); ++i) {
        PiecewiseLinearFunction fObj(energyPeaks_.at(i));
        TF1 f("f", fObj, 0, 4'000, 0);
        fs.push_back(f);
    }
    std::vector<std::function<void()>> tasks;
    for (size_t i{0}; i < std::min(histsSg.size(), channels_.g.size()); ++i) {
        for (size_t j{0}; j <  std::min(histsSg.at(i).size(), channels_.a.size()); ++j) {
            tasks.push_back([this, &histsSg, &histsBg, i, j, &fs](){
                histsSg.at(i).at(j)->Reset();
                histsBg.at(i).at(j)->Reset();
                std::pair<uint8_t, uint8_t> p{*std::next(channels_.g.begin(), i), *std::next(channels_.a.begin(), j)};
                if (events_m_.find(p) != events_m_.end()) {
                    auto minT_sg{timeCorrections_.at({i, j}) - 3.0};
                    auto maxT_sg{timeCorrections_.at({i, j}) + 3.0};
                    fillHistEnergy(events_m_[p], histsSg.at(i).at(j), minT_sg, maxT_sg, false, fs.at(i));
                    auto minT_bg{timeCorrections_.at({i, j}) - 30.0};
                    auto maxT_bg{timeCorrections_.at({i, j}) - 20.0};
                    fillHistEnergy(events_m_[p], histsBg.at(i).at(j), minT_bg, maxT_bg, false, fs.at(i));
                }
            });
        }
    }
    func_async(tasks.begin(), tasks.end());
    tasks.clear();
}

void Calibration::fillHistsEnergyByGamma(const std::vector<std::vector<TH1D *> > &histsSg, const std::vector<std::vector<TH1D *> > &histsBg)
{
    for (size_t i{0}; i <  histogramManager_->histsEnergyByGamma().size(); ++i) {
        histogramManager_->histsEnergyByGamma()[i]->Reset();
    }
    for (size_t i{0}; i < histsSg.size(); ++i) {
        for (size_t j{0}; j <  histsSg.at(i).size(); ++j) {
            histogramManager_->histsEnergyByGamma()[i]->Add(histsSg.at(i).at(j));
            histogramManager_->histsEnergyByGamma()[i]->Add(histsBg.at(i).at(j), -1.0 * 6.0 / 10.0);
            histogramManager_->histEnergyTotal()->Add(histsSg.at(i).at(j));
            histogramManager_->histEnergyTotal()->Add(histsBg.at(i).at(j), -1.0 * 6.0 / 10.0);
        }
    }
}

void Calibration::fillHistsEnergyByAlpha(const std::vector<std::vector<TH1D *> > &histsSg, const std::vector<std::vector<TH1D *> > &histsBg)
{
    for (size_t i{0}; i <  histogramManager_->histsEnergyByAlpha().size(); ++i) {
        histogramManager_->histsEnergyByAlpha()[i]->Reset();
    }
    for (size_t i{0}; i < histsSg.size(); ++i) {
        for (size_t j{0}; j <  histsSg.at(i).size(); ++j) {
            histogramManager_->histsEnergyByAlpha()[j]->Add(histsSg.at(i).at(j));
            histogramManager_->histsEnergyByAlpha()[j]->Add(histsBg.at(i).at(j), -1.0 * 6.0 / 10.0);
        }
    }
};

void Calibration::setNewData(const std::unordered_map<std::pair<uint8_t, uint8_t>, std::vector<dec_ev_t>, PairHash> &events, const dec_ch_t &channels, double time, const std::map<uint8_t, uint32_t> &counters)
{
    time_ += time;
    for (const auto& [key, vec] : events) {
        auto [it, inserted] = events_m_.try_emplace(key, vec);
        if (!inserted) {
            it->second.insert(it->second.end(), vec.begin(), vec.end());
        }
    }
    for (const auto& [key, value] : counters) {
        auto [it, inserted] = counters_.try_emplace(key, value);
        if (!inserted) {
            it->second = value;
        }
    }
    countersG_.clear();
    countersA_.clear();
    channels_ = channels;
    for (size_t i{0}; i < channels_.g.size(); ++i) {
        uint8_t key{*std::next(channels_.g.begin(), i)};
        auto it = counters_.find(key);
        if (it != counters_.end()) {
            auto value{it->second};
            auto [itt, inserted] = countersG_.try_emplace(i, value);
            if (!inserted) {
                itt->second = value;
            }
        }
    }
    for (size_t i{0}; i < channels_.a.size(); ++i) {
        uint8_t key{*std::next(channels_.a.begin(), i)};
        auto it = counters_.find(key);
        if (it != counters_.end()) {
            auto value{it->second};
            auto [itt, inserted] = countersA_.try_emplace(i, value);
            if (!inserted) {
                itt->second = value;
            }
        }
    }
}

double Calibration::time() const
{
    return time_;
}

const std::map<uint8_t, double> &Calibration::counters() const
{
    return counters_;
}

const std::map<uint8_t, double> &Calibration::countersG() const
{
    return countersG_;
}

const std::map<uint8_t, double> &Calibration::countersA() const
{
    return countersA_;
}


