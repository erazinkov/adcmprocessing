#include "calibration.h"
#include "utils.h"
#include "piecewiselinearfunction.h"
//#include "polynomialfunction.h"
#include "timepeaksfinder.h"
#include "resolutionprocessing.h"

#include <TFile.h>
#include <TTree.h>

#include "consoletable.h"

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

void Calibration::process(const std::string &internalEnergyPeaksFileName, const std::string &externalEnergyPeaksFileName)
{

    fillHistsTimeByGammaAlpha(histogramManager_->histsTimeByGammaAlpha(), false);

    for (size_t ig{0}; ig < std::min(histogramManager_->histsTimeByGammaAlpha().size(), channels_.g.size()); ++ig) {
        for (size_t ia{0}; ia <  std::min(histogramManager_->histsTimeByGammaAlpha().at(0).size(), channels_.a.size()); ++ia) {
            std::pair<uint8_t, uint8_t> p{ig, ia};
            timeCorrections_[p] = TimePeaksFinder::calculateObjPeakPos(histogramManager_->histsTimeByGammaAlpha().at(ig).at(ia).get());
        }
    }

    fillHistsTimeByGammaAlpha(histogramManager_->histsTimeCorrectedByGammaAlpha(), true);

    fillHistsAmpByGammaAlpha(histogramManager_->histsAmpByGammaAlphaSg(), histogramManager_->histsAmpByGammaAlphaBg(), histogramManager_->histsAmpByGammaAlphaRc());

    fillHistsAmpByGamma(histogramManager_->histsAmpByGammaAlphaSg(), histogramManager_->histsAmpByGammaAlphaBg(), histogramManager_->histsAmpByGammaAlphaRc());

    energyPeaks_.clear();
    if (externalEnergyPeaksFileName.empty()) {
        std::cout << "Use" << std::string(GREEN) + " internal " + RESET + "energy peaks positions" << std::endl;
        for (size_t i{0}; i < std::min(histogramManager_->histsAmpByGamma().size(), channels_.g.size()); ++i) {
            energyPeakFinder_.process(histogramManager_->histsAmpByGamma().at(i).get(), histogramManager_->histsAmpByGammaRc().at(i).get());
            energyPeaks_.push_back(energyPeakFinder_.energyPeaks());
//            std::cout << i << " ";
//            for (const auto &p : energyPeaks_.back()) {
//                std::cout << "x " << p.channel() << " " << "y " << p.energy() << " ";
//            }
//            std::cout << std::endl;
        }
        saveEnergyPeaks(internalEnergyPeaksFileName);

    } else {
        std::cout << "Use" << std::string(RED) + " external " + RESET + "energy peaks positions" << std::endl;
        std::vector<std::vector<EnergyPeak>> energyPeaks;
        for (size_t i{0}; i < std::min(histogramManager_->histsAmpByGamma().size(), channels_.g.size()); ++i) {
            energyPeakFinder_.processExternal(histogramManager_->histsAmpByGamma().at(i).get(), histogramManager_->histsAmpByGammaRc().at(i).get());
            energyPeaks.push_back(energyPeakFinder_.energyPeaks());
        }
        loadEnergyPeaks(externalEnergyPeaksFileName);
        for (size_t i{0}; i < energyPeaks.size(); i++) {
            energyPeakFinder_.processExternal(energyPeaks.at(i), energyPeaks_.at(i));
        }
    }


//    fillHistsEnergyByGammaAlpha(histogramManager_->histsEnergyByGammaAlphaSg(), histogramManager_->histsEnergyByGammaAlphaBg(), histogramManager_->histsEnergyByGammaAlphaRc());
//    fillHistsEnergyByGamma(histogramManager_->histsEnergyByGammaAlphaSg(), histogramManager_->histsEnergyByGammaAlphaBg(), histogramManager_->histsEnergyByGammaAlphaRc());
//  for (size_t i{0}; i < std::min(histogramManager_->histsAmpByGamma().size(), channels_.g.size()); ++i) {
//      energyPeakFinder_.check(histogramManager_->histsEnergyByGamma().at(i).get(), histogramManager_->histsEnergyByGammaRc().at(i).get());
//  }
    // !

//    ResolutionProcessing rP;
//    rP.processingEnergy(AppConstants::OUTPUT_PATH + fileName_, histogramManager_->histsEnergyByGamma(), channels_.g.size());
//    fillHistsEnergyByAlpha(histogramManager_->histsEnergyByGammaAlphaSg(), histogramManager_->histsEnergyByGammaAlphaBg());
//    fillHistsTimeByAlpha(histogramManager_->histsTimeCorrectedByGammaAlpha());

    fillHistsTimeWithEnergyCutByGammaAlpha(histogramManager_->histsTimeByGammaAlpha(), true);

    fillHistsTimeWithEnergyCutByGamma(histogramManager_->histsTimeByGammaAlpha());
    auto timeCorrectionsBox{timeCorrections_};
    for (size_t ig{0}; ig < std::min(histogramManager_->histsTimeByGammaAlpha().size(), channels_.g.size()); ++ig) {
        auto timeCorrectionBox{TimePeaksFinder::calculateBoxPeakPos(histogramManager_->histsTimeCorrectedByGamma().at(ig).get())};
        std::cout << "timeCorrectionBox" << " " << ig << " " << timeCorrectionBox << std::endl;
        for (size_t ia{0}; ia <  std::min(histogramManager_->histsTimeByGammaAlpha().at(0).size(), channels_.a.size()); ++ia) {
            std::pair<uint8_t, uint8_t> p{ig, ia};
            timeCorrectionsBox[p] += timeCorrectionBox;
        }
    }

    timeCorrections_ = timeCorrectionsBox;

    fillHistsAmpByGammaAlpha(histogramManager_->histsAmpByGammaAlphaSg(), histogramManager_->histsAmpByGammaAlphaBg(), histogramManager_->histsAmpByGammaAlphaRc());

    fillHistsAmpByGamma(histogramManager_->histsAmpByGammaAlphaSg(), histogramManager_->histsAmpByGammaAlphaBg(), histogramManager_->histsAmpByGammaAlphaRc());

//    rP.processingTime(AppConstants::OUTPUT_PATH + fileName_, histogramManager_->histsTimeCorrectedByGamma(), channels_.g.size());
//    rP.exportToCSV(AppConstants::OUTPUT_PATH + fileName_);

}


void Calibration::fillHistsTimeByGammaAlpha(const std::vector<std::vector<std::unique_ptr<TH1D>> > &hists, bool isCorrected)
{
    std::vector<std::function<void()>> tasks;
    for (size_t i{0}; i < std::min(hists.size(), channels_.g.size()); ++i) {
        for (size_t j{0}; j <  std::min(hists.size(), channels_.a.size()); ++j) {
            tasks.push_back([this, &hists, i, j, isCorrected](){
                hists.at(i).at(j)->Reset();
                std::pair<uint8_t, uint8_t> p{*std::next(channels_.g.begin(), i), *std::next(channels_.a.begin(), j)};
                if (events_m_.find(p) != events_m_.end()) {
                    fillHistTime(events_m_.at(p), hists.at(i).at(j).get(), isCorrected ? timeCorrections_[{i, j}] : 0.0);
                }
            });
        }
    }
    func_async(tasks.begin(), tasks.end());
    tasks.clear();
}

void Calibration::fillHistsTimeWithEnergyCutByGammaAlpha(const std::vector<std::vector<std::unique_ptr<TH1D>> > &hists, bool isCorrected)
{
    std::vector<TF1> fs;
    for (size_t i{0}; i < std::min(hists.size(), channels_.g.size()); ++i) {
        PiecewiseLinearFunction fObj(energyPeaks_.at(i));
//        PolynomialFunction fObj(energyPeaks_.at(i));
        TF1 f("f", fObj, 0, 4'000, 0);
        fs.push_back(f);
    }
    std::vector<std::function<void()>> tasks;
    for (size_t i{0}; i < std::min(hists.size(), channels_.g.size()); ++i) {
        for (size_t j{0}; j <  std::min(hists.size(), channels_.a.size()); ++j) {
            tasks.push_back([this, &hists, i, j, isCorrected, &fs](){
                hists.at(i).at(j)->Reset();
                std::pair<uint8_t, uint8_t> p{*std::next(channels_.g.begin(), i), *std::next(channels_.a.begin(), j)};
                if (events_m_.find(p) != events_m_.end()) {
                    fillHistTimeWithEnergyCut(events_m_.at(p),
                                              hists.at(i).at(j).get(),
                                              isCorrected ? timeCorrections_[{i, j}] : 0.0,
                                              4438.0 - 2.0 * 250.0,
                                              4438.0 + 2.0 * 250.0,
                                              fs.at(i));
                }
            });
        }
    }
    func_async(tasks.begin(), tasks.end());
    tasks.clear();
}

void Calibration::fillHistTimeWithEnergyCutTotal(const std::vector<std::vector<std::unique_ptr<TH1D>> > &hists)
{
    histogramManager_->histTimeTotal()->Reset();
    for (size_t i{0}; i < hists.size(); ++i) {
        for (size_t j{0}; j <  hists.at(i).size(); ++j) {
            histogramManager_->histTimeTotal()->Add(hists.at(i).at(j).get());
        }
    }
}

void Calibration::fillHistsTimeWithEnergyCutByGamma(const std::vector<std::vector<std::unique_ptr<TH1D>> > &hists)
{
    for (size_t i{0}; i <  histogramManager_->histsTimeCorrectedByGamma().size(); ++i) {
        histogramManager_->histsTimeCorrectedByGamma()[i]->Reset();
    }
    for (size_t i{0}; i < hists.size(); ++i) {
        for (size_t j{0}; j <  hists.at(i).size(); ++j) {
            histogramManager_->histsTimeCorrectedByGamma()[i]->Add(hists.at(i).at(j).get());
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
        } else {
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
        } else {
            if (minT <= t && t <= maxT) {
                h->Fill(f.Eval(a));
            }
        }
    }
}

void Calibration::fillHistsTimeByAlpha(const std::vector<std::vector<std::unique_ptr<TH1D>> > &hists)
{
    for (size_t i{0}; i <  histogramManager_->histsTimeCorrectedByAlpha().size(); ++i) {
        histogramManager_->histsTimeCorrectedByAlpha()[i]->Reset();
    }
    for (size_t i{0}; i < hists.size(); ++i) {
        for (size_t j{0}; j <  hists.at(i).size(); ++j) {
            histogramManager_->histsTimeCorrectedByAlpha()[j]->Add(hists.at(i).at(j).get());
        }
    }
}

void Calibration::fillHistsAmpByGammaAlpha(const std::vector<std::vector<std::unique_ptr<TH1D>> > &histsSg,
                                           const std::vector<std::vector<std::unique_ptr<TH1D>> > &histsBg,
                                           const std::vector<std::vector<std::unique_ptr<TH1D>> > &histsRc)
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
                    fillHistAmp(events_m_.at(p), histsSg.at(i).at(j).get(), minT_sg, maxT_sg, false);
                    fillHistAmp(events_m_.at(p), histsRc.at(i).at(j).get(), minT_sg, maxT_sg, true);
                    auto minT_bg{timeCorrections_.at({i, j}) - 30.0};
                    auto maxT_bg{timeCorrections_.at({i, j}) - 20.0};
                    fillHistAmp(events_m_.at(p), histsBg.at(i).at(j).get(), minT_bg, maxT_bg, false);
                }
            });
        }
    }
    func_async(tasks.begin(), tasks.end());
    tasks.clear();
}



void Calibration::fillHistsAmpByAlpha(const std::vector<std::vector<std::unique_ptr<TH1D>>> &histsSg, const std::vector<std::vector<std::unique_ptr<TH1D>> > &histsBg)
{
    for (size_t i{0}; i <  histogramManager_->histsAmpByAlpha().size(); ++i) {
        histogramManager_->histsAmpByAlpha()[i]->Reset();
    }
    for (size_t i{0}; i < histsSg.size(); ++i) {
        for (size_t j{0}; j <  histsSg.at(i).size(); ++j) {
            histogramManager_->histsAmpByAlpha()[j]->Add(histsSg.at(i).at(j).get());
            histogramManager_->histsAmpByAlpha()[j]->Add(histsBg.at(i).at(j).get(), -1.0 * 6.0 / 10.0);
        }
    }
}



void Calibration::fillHistsEnergyByGammaAlpha(const std::vector<std::vector<std::unique_ptr<TH1D>> > &histsSg, const std::vector<std::vector<std::unique_ptr<TH1D>> > &histsBg, const std::vector<std::vector<std::unique_ptr<TH1D>> > &histsRc)
{
    std::vector<TF1> fs;
    for (size_t i{0}; i < std::min(histsSg.size(), channels_.g.size()); ++i) {
        PiecewiseLinearFunction fObj(energyPeaks_.at(i));
//        PolynomialFunction fObj(energyPeaks_.at(i));
        TF1 f("f", fObj, 0, 4'000, 0);
        fs.push_back(f);
    }
    std::vector<std::function<void()>> tasks;
    for (size_t i{0}; i < std::min(histsSg.size(), channels_.g.size()); ++i) {
        for (size_t j{0}; j <  std::min(histsSg.at(i).size(), channels_.a.size()); ++j) {
            tasks.push_back([this, &histsSg, &histsBg, &histsRc, i, j, &fs](){
                histsSg.at(i).at(j)->Reset();
                histsBg.at(i).at(j)->Reset();
                histsRc.at(i).at(j)->Reset();
                std::pair<uint8_t, uint8_t> p{*std::next(channels_.g.begin(), i), *std::next(channels_.a.begin(), j)};
                if (events_m_.find(p) != events_m_.end()) {
                    auto minT_sg{timeCorrections_.at({i, j}) - 3.0};
                    auto maxT_sg{timeCorrections_.at({i, j}) + 3.0};
                    fillHistEnergy(events_m_[p], histsSg.at(i).at(j).get(), minT_sg, maxT_sg, false, fs.at(i));
                    auto minT_bg{timeCorrections_.at({i, j}) - 30.0};
                    auto maxT_bg{timeCorrections_.at({i, j}) - 20.0};
                    fillHistEnergy(events_m_[p], histsBg.at(i).at(j).get(), minT_bg, maxT_bg, false, fs.at(i));
                    fillHistEnergy(events_m_[p], histsRc.at(i).at(j).get(), minT_bg, maxT_bg, true, fs.at(i));
                }
            });
        }
    }
    func_async(tasks.begin(), tasks.end());
    tasks.clear();
}

void Calibration::fillHistsAmpByGamma(const std::vector<std::vector<std::unique_ptr<TH1D>> > &histsSg,
                                      const std::vector<std::vector<std::unique_ptr<TH1D>> > &histsBg,
                                      const std::vector<std::vector<std::unique_ptr<TH1D>> > &histsRc)
{
    for (size_t i{0}; i <  histogramManager_->histsAmpByGamma().size(); ++i) {
        histogramManager_->histsAmpByGamma()[i]->Reset();
        histogramManager_->histsAmpByGammaRc()[i]->Reset();
    }
    for (size_t i{0}; i < std::min(histsSg.size(), channels_.g.size()); ++i) {
        for (size_t j{0}; j <  std::min(histsSg.at(i).size(), channels_.a.size()); ++j) {
            histogramManager_->histsAmpByGamma()[i]->Add(histsSg.at(i).at(j).get());
            histogramManager_->histsAmpByGamma()[i]->Add(histsBg.at(i).at(j).get(), -1.0 * 6.0 / 10.0);

            histogramManager_->histsAmpByGammaRc()[i]->Add(histsRc.at(i).at(j).get());
        }
    }
}

void Calibration::fillHistsEnergyByGamma(const std::vector<std::vector<std::unique_ptr<TH1D>> > &histsSg,
                                         const std::vector<std::vector<std::unique_ptr<TH1D>> > &histsBg,
                                         const std::vector<std::vector<std::unique_ptr<TH1D>> > &histsRc)
{
    for (size_t i{0}; i <  histogramManager_->histsEnergyByGamma().size(); ++i) {
        histogramManager_->histsEnergyByGamma()[i]->Reset();
        histogramManager_->histsEnergyByGammaRc()[i]->Reset();
    }
    for (size_t i{0}; i < histsSg.size(); ++i) {
        for (size_t j{0}; j <  histsSg.at(i).size(); ++j) {
            histogramManager_->histsEnergyByGamma()[i]->Add(histsSg.at(i).at(j).get());
            histogramManager_->histsEnergyByGamma()[i]->Add(histsBg.at(i).at(j).get(), -1.0 * 6.0 / 10.0);

            histogramManager_->histsEnergyByGammaRc()[i]->Add(histsRc.at(i).at(j).get());

//            histogramManager_->histEnergyTotal()->Add(histsSg.at(i).at(j).get());
//            histogramManager_->histEnergyTotal()->Add(histsBg.at(i).at(j).get(), -1.0 * 6.0 / 10.0);
        }
    }
}

void Calibration::fillHistsEnergyByAlpha(const std::vector<std::vector<std::unique_ptr<TH1D>> > &histsSg, const std::vector<std::vector<std::unique_ptr<TH1D>> > &histsBg)
{
    for (size_t i{0}; i <  histogramManager_->histsEnergyByAlpha().size(); ++i) {
        histogramManager_->histsEnergyByAlpha()[i]->Reset();
    }
    for (size_t i{0}; i < histsSg.size(); ++i) {
        for (size_t j{0}; j <  histsSg.at(i).size(); ++j) {
            histogramManager_->histsEnergyByAlpha()[j]->Add(histsSg.at(i).at(j).get());
            histogramManager_->histsEnergyByAlpha()[j]->Add(histsBg.at(i).at(j).get(), -1.0 * 6.0 / 10.0);
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

void Calibration::saveEnergyPeaks(const std::string &fileName)
{
    std::unique_ptr<TFile> file = std::make_unique<TFile>(fileName.c_str(), "RECREATE");
    if (!file || file->IsZombie()) {
        std::cout << "Can\'t open file " << fileName << std::endl;
        return;
    }
    std::unique_ptr<TTree> tree = std::make_unique<TTree>("energyPeaks", "energyPeaks");
    tree->SetDirectory(nullptr);

    std::vector<double> energy, channel;
    std::vector<int> id;
    int eventIdx{0};

    tree->Branch("eventIdx", &eventIdx);
    tree->Branch("id", &id);
    tree->Branch("energy", &energy);
    tree->Branch("channel", &channel);

    for (size_t i{0}; i < energyPeaks_.size(); i++) {
        eventIdx = i;
        id.clear();energy.clear();channel.clear();
        for (const auto& p : energyPeaks_.at(i)) {
            id.push_back(static_cast<int>(p.id()));
            energy.push_back(p.energy());
            channel.push_back(p.channel());
        }
        tree->Fill();
    }
    file->cd();
    tree->Write();
}

void Calibration::loadEnergyPeaks(const std::string &fileName)
{
    std::unique_ptr<TFile> file = std::make_unique<TFile>(fileName.c_str(), "READ");
    if (!file || file->IsZombie()) {
        std::cout << "Can\'t open file " << fileName << std::endl;
        return;
    }
    TTree *tree{nullptr};
    file->GetObject("energyPeaks", tree);
    if (!tree || tree->IsZombie()) {
        std::cout << "Can\'t load tree from file " << fileName << std::endl;
        return;
    }

    int eventIdx{0};
    std::vector<int>* id{nullptr};
    std::vector<double>* energy{nullptr};
    std::vector<double>* channel{nullptr};

    tree->SetBranchAddress("eventIdx", &eventIdx);
    tree->SetBranchAddress("id", &id);
    tree->SetBranchAddress("energy", &energy);
    tree->SetBranchAddress("channel", &channel);

    Long64_t nEntries = tree->GetEntries();
    std::vector<std::vector<EnergyPeak>> energyPeaks;

    for (Long64_t i{0}; i < nEntries; i++) {
        tree->GetEntry(i);
        if (static_cast<int>(energyPeaks.size()) <= eventIdx) {
            energyPeaks.resize(eventIdx + 1);
        }
        auto& peaks = energyPeaks[eventIdx];
        peaks.clear();
        for (size_t k{0}; k < id->size(); k++) {
            peaks.push_back(EnergyPeak{static_cast<EnergyPeak::Id>((*id)[k]), (*channel)[k]});
        }
    }

    energyPeaks_ = energyPeaks;

}


