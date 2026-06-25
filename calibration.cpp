#include "calibration.h"

#include <TCanvas.h>

#include "utils.h"
#include "constants.h"
#include "piecewiselinearfunction.h"

#include "timepeaksfinder.h"
#include "resolutionprocessing.h"

Calibration::Calibration(const std::string &fileName, HistogramManager *histogramManager)
    : histogramManager_{histogramManager}, fileName_{fileName}
{
    for (auto ig{0}; ig < AppConstants::MAX_GAMMA_NUMBER; ++ig) {
        for (auto ia{0}; ia < AppConstants::MAX_ALPHA_NUMBER; ++ia) {
            std::pair<uint8_t, uint8_t> p{ig, ia};
            timeCorrections_[p] = 0.0;
        }
    }
}

void Calibration::process()
{

    fillHistsTimeByGammaAlpha(histogramManager_->histsTimeByGammaAlpha(), false);
    // TODO
    const std::string psName_t{AppConstants::OUTPUT_PATH + fileName_ + "_time_peaks_finder_result.ps"};
    std::unique_ptr<TCanvas> c_t{new TCanvas("c_t", "c_t", 1024, 960)};
    c_t.get()->Print((psName_t + '[').c_str());
    const auto igMax = std::min(histogramManager_->histsTimeByGammaAlpha().size(), channels_.g.size());
    const auto iaMax = std::min(histogramManager_->histsTimeByGammaAlpha().at(0).size(), channels_.a.size());
    for (size_t ig{0}; ig < igMax; ++ig) {
        auto cd{static_cast<int>(std::ceil(std::sqrt(histogramManager_->histsTimeByGammaAlpha().at(0).size())))};
        c_t.get()->Divide(cd, cd);
        for (size_t ia{0}; ia <  iaMax; ++ia) {
            c_t.get()->cd(static_cast<int>(ia) + 1);
            std::pair<uint8_t, uint8_t> p{ig, ia};
            timeCorrections_[p] = TimePeaksFinder::calculatePeakPos(histogramManager_->histsTimeByGammaAlpha().at(ig).at(ia));
            histogramManager_->histsTimeByGammaAlpha().at(ig).at(ia)->Draw();
            auto listOfFunctionsSg{histogramManager_->histsTimeByGammaAlpha().at(ig).at(ia)->GetListOfFunctions()};
            for (auto *item : *listOfFunctionsSg) {
                item->Draw("SAME");
            }
        }

        c_t.get()->Print(psName_t.c_str());
        c_t.get()->Clear();
    }
    c_t.get()->Print((psName_t + ']').c_str());

    //            timeCorrections_[p] = histogramManager_->histsTimeByGammaAlpha().at(ig).at(ia)->GetBinCenter(histogramManager_->histsTimeByGammaAlpha().at(ig).at(ia)->GetMaximumBin());

    fillHistsTimeByGammaAlpha(histogramManager_->histsTimeCorrectedByGammaAlpha(), true);

    fillHistsAmpByGammaAlpha(histogramManager_->histsAmpByGammaAlphaSg(), histogramManager_->histsAmpByGammaAlphaBg(), histogramManager_->histsAmpByGammaAlphaRc());


    fillHistsAmpByGamma(histogramManager_->histsAmpByGammaAlphaSg(), histogramManager_->histsAmpByGammaAlphaBg(), histogramManager_->histsAmpByGammaAlphaRc());

    energyPeaks_.clear();
    const std::string psName{AppConstants::OUTPUT_PATH + fileName_ + "_energy_peaks_finder_result.ps"};
    TCanvas *c  = new TCanvas("c", "c", 1024, 960);
    c->Print((psName + '[').c_str());
    c->Divide(1, 2);
    for (size_t i{0}; i < std::min(histogramManager_->histsAmpByGamma().size(), channels_.g.size()); ++i) {
        energyPeakFinder_.process(histogramManager_->histsAmpByGamma().at(i), histogramManager_->histsAmpByGammaRc().at(i));
        energyPeaks_.push_back(energyPeakFinder_.energyPeaks());
        c->cd(1);
        histogramManager_->histsAmpByGamma().at(i)->GetXaxis()->SetRangeUser(0.0, 2'000.0);
        histogramManager_->histsAmpByGamma().at(i)->Draw();
        auto listOfFunctionsSg{histogramManager_->histsAmpByGamma().at(i)->GetListOfFunctions()};
        for (auto *item : *listOfFunctionsSg) {
            item->Draw("SAME");
        }
        c->cd(2);
        histogramManager_->histsAmpByGammaRc().at(i)->GetXaxis()->SetRangeUser(0.0, 2'000.0);
        histogramManager_->histsAmpByGammaRc().at(i)->Draw();
        auto listOfFunctionsRc{histogramManager_->histsAmpByGammaRc().at(i)->GetListOfFunctions()};
        for (auto *item : *listOfFunctionsRc) {
            item->Draw("SAME");
        }
        c->Print(psName.c_str());
    }
    c->Print((psName + ']').c_str());
    delete c;
//    energyPeaksRaw_.clear();
//    for (size_t i{0}; i < std::min(histogramManager_->histsAmpByGamma().size(), channels_.g.size()); ++i) {
//        peakFinder_.processRaw(histogramManager_->histsAmpByGammaRc().at(i));
//        energyPeaksRaw_.push_back(peakFinder_.energyPeak());
//    }

    fillHistsEnergyByGammaAlpha(histogramManager_->histsEnergyByGammaAlphaSg(), histogramManager_->histsEnergyByGammaAlphaBg());
    fillHistsEnergyByGamma(histogramManager_->histsEnergyByGammaAlphaSg(), histogramManager_->histsEnergyByGammaAlphaBg());

    ResolutionProcessing rP;
    rP.processingEnergy(AppConstants::OUTPUT_PATH + fileName_, histogramManager_->histsEnergyByGamma(), channels_.g.size());
    fillHistsEnergyByAlpha(histogramManager_->histsEnergyByGammaAlphaSg(), histogramManager_->histsEnergyByGammaAlphaBg());
    fillHistsTimeByAlpha(histogramManager_->histsTimeCorrectedByGammaAlpha());
    // !
    fillHistsTimeWithEnergyCutByGammaAlpha(histogramManager_->histsTimeByGammaAlpha());
    fillHistsTimeWithEnergyCutByGamma(histogramManager_->histsTimeByGammaAlpha());
    rP.processingTime(AppConstants::OUTPUT_PATH + fileName_, histogramManager_->histsTimeCorrectedByGamma(), channels_.g.size());
    rP.exportToCSV(AppConstants::OUTPUT_PATH + fileName_);

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
    for (size_t i{0}; i < std::min(hists.size(), channels_.g.size()); ++i)
    {
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

void Calibration::fillHistTime(const std::vector<dec_ev_m_t> &events, TH1 *h, double correction)
{
    for (const auto & item : events) {
        h->Fill(item.tdc - correction);
    }
}

void Calibration::fillHistTimeWithEnergyCut(const std::vector<dec_ev_m_t> &events,
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

void Calibration::fillHistAmp(const std::vector<dec_ev_m_t> &events, TH1 *h, double minT, double maxT, bool exclude)
{
    for (const auto & item : events) {
        auto t{static_cast<double>(item.tdc)};
        auto a{static_cast<double>(item.amp)};
        if (exclude)
        {
            if (t < minT || maxT < t)
            {
                h->Fill(a);
            }
        }
        else
        {
            if (minT <= t && t <= maxT)
            {
                h->Fill(a);
            }
        }
    }
}

void Calibration::fillHistEnergy(const std::vector<dec_ev_m_t> &events, TH1 *h, double minT, double maxT, bool exclude, TF1 f)
{
    for (const auto & item : events) {
        auto t{static_cast<double>(item.tdc)};
        auto a{static_cast<double>(item.amp)};
        if (exclude)
        {
            if (t < minT || maxT < t)
            {
                h->Fill(f.Eval(a));
            }
        }
        else
        {
            if (minT <= t && t <= maxT)
            {
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

//    auto ff = [] (double *x, double *par) {
//        double xx = x[0];
//        return par[0] + par[1] * xx;
//    };

//    std::vector<TF1> fs;
//    for (size_t i{0}; i < std::min(histsSg.size(), channels_.g.size()); ++i) {
//        TF1 f("f", ff, 0, 4'000, 2);
//        f.SetParameters(0.0, energyPeaksRaw_.at(i).energy() / energyPeaksRaw_.at(i).channel());
//        fs.push_back(f);
//    }

    std::vector<TF1> fs;
    for (size_t i{0}; i < std::min(histsSg.size(), channels_.g.size()); ++i)
    {
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
//                PiecewiseLinearFunction fObj(energyPeaks_.at(i));
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

void Calibration::setNewData(const std::map<std::pair<uint8_t, uint8_t>, std::vector<dec_ev_m_t> > &events,
                             const dec_ch_t &channels,
                             double time,
                             const std::map<uint8_t, uint32_t> &counters)
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

void Calibration::setNewData_o(const std::unordered_map<std::pair<uint8_t, uint8_t>, std::vector<dec_ev_m_t>, PairHash> &events_o, const dec_ch_t &channels, double time, const std::map<uint8_t, uint32_t> &counters)
{
    time_ += time;
    for (const auto& [key, vec] : events_o) {
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

void Calibration::resetData()
{
    std::cout << "Reset events - ";
    for (auto& [key, vec] : events_m_) {
        vec.clear();
    }
    std::cout << "Done" << std::endl;
    std::cout << "Reset time - ";
    time_ = 0.0;
    std::cout << "Done" << std::endl;
    std::cout << "Reset counters - ";
    for (auto& [key, value] : counters_) {
        value = 0;
    }
    std::cout << "Done" << std::endl;
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


