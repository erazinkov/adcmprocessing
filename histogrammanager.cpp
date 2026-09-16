#include "histogrammanager.h"

HistogramManager::HistogramManager(const int &gammaNumber, const int &alphaNumber, std::optional<std::string> outputDirectory)
    : gammaNumber_{gammaNumber}, alphaNumber_{alphaNumber}, outputDirectory_{outputDirectory}
{
    histTimeTotal_ = std::make_unique<TH1D>("hist_time_total", "hist_time_total", BINS_TIME, XLOW_TIME, XUP_TIME);
    histTimeTotal_->Sumw2();
    histTimeTotal_->SetDirectory(nullptr);

    histEnergyTotal_ = std::make_unique<TH1D>("hist_energy_total", "hist_energy_total", BINS_ENERGY, XLOW_ENERGY, XUP_ENERGY);
    histEnergyTotal_->Sumw2();
    histEnergyTotal_->SetDirectory(nullptr);


    auto addHistogramToGrid = [](std::vector<std::vector<std::unique_ptr<TH1D>>>& hists,
                           size_t rowIndex,
                           const std::string& name,
                           const std::string& title,
                           int nbinsx, double xlow, double xup) {
        if (rowIndex >= hists.size()) {
            hists.resize(rowIndex + 1);
        }
        auto h = std::make_unique<TH1D>(name.c_str(), title.c_str(), nbinsx, xlow, xup);
        h->SetDirectory(nullptr);
        h->Sumw2();
        hists[rowIndex].push_back(std::move(h));
    };

    auto addHistogramToRow = [](std::vector<std::unique_ptr<TH1D>>& hists,
                           const std::string& name,
                           const std::string& title,
                           int nbinsx, double xlow, double xup) {
        auto h = std::make_unique<TH1D>(name.c_str(), title.c_str(), nbinsx, xlow, xup);
        h->SetDirectory(nullptr);
        h->Sumw2();
        hists.push_back(std::move(h));
    };

    for (auto ig{0}; ig < gammaNumber_; ig++) {
        for (auto ia{0}; ia < alphaNumber_; ia++) {
            addHistogramToGrid(histsAmpByGammaAlphaSg_,
                         ig,
                         Form("hist_amp_by_gamma_sg_%d_alpha_%d", ig, ia),
                         Form("hist_amp_by_gamma_sg_%d_alpha_%d", ig, ia),
                         BINS_CHANNEL, XLOW_CHANNEL, XUP_CHANNEL);
            addHistogramToGrid(histsAmpByGammaAlphaBg_,
                         ig,
                         Form("hist_amp_by_gamma_bg_%d_alpha_%d", ig, ia),
                         Form("hist_amp_by_gamma_bg_%d_alpha_%d", ig, ia),
                         BINS_CHANNEL, XLOW_CHANNEL, XUP_CHANNEL);
            addHistogramToGrid(histsAmpByGammaAlphaRc_,
                         ig,
                         Form("hist_amp_by_gamma_rc_%d_alpha_%d", ig, ia),
                         Form("hist_amp_by_gamma_rc_%d_alpha_%d", ig, ia),
                         BINS_CHANNEL, XLOW_CHANNEL, XUP_CHANNEL);
            addHistogramToGrid(histsTimeByGammaAlpha_,
                         ig,
                         Form("hist_time_by_gamma_%d_alpha_%d", ig, ia),
                         Form("hist_time_by_gamma_%d_alpha_%d", ig, ia),
                         BINS_TIME, XLOW_TIME, XUP_TIME);
            addHistogramToGrid(histsTimeCorrectedByGammaAlpha_,
                         ig,
                         Form("hist_time_corrected_by_gamma_%d_alpha_%d", ig, ia),
                         Form("hist_time_corrected_by_gamma_%d_alpha_%d", ig, ia),
                         BINS_TIME, XLOW_TIME, XUP_TIME);
            addHistogramToGrid(histsEnergyByGammaAlphaSg_,
                         ig,
                         Form("hist_energy_by_gamma_sg_%d_alpha_%d", ig, ia),
                         Form("hist_energy_by_gamma_sg_%d_alpha_%d", ig, ia),
                         BINS_ENERGY, XLOW_ENERGY, XUP_ENERGY);
            addHistogramToGrid(histsEnergyByGammaAlphaBg_,
                         ig,
                         Form("hist_energy_by_gamma_bg_%d_alpha_%d", ig, ia),
                         Form("hist_energy_by_gamma_bg_%d_alpha_%d", ig, ia),
                         BINS_ENERGY, XLOW_ENERGY, XUP_ENERGY);
            addHistogramToGrid(histsEnergyByGammaAlphaRc_,
                         ig,
                         Form("hist_energy_by_gamma_rc_%d_alpha_%d", ig, ia),
                         Form("hist_energy_by_gamma_rc_%d_alpha_%d", ig, ia),
                         BINS_ENERGY, XLOW_ENERGY, XUP_ENERGY);
        }
    }

    for (auto ig{0}; ig < gammaNumber_; ig++) {
        addHistogramToRow(histsAmpByGamma_,
                          Form("hist_amp_by_gamma_%d", ig),
                          Form("hist_amp_by_gamma_%d", ig),
                          BINS_CHANNEL, XLOW_CHANNEL, XUP_CHANNEL);
        addHistogramToRow(histsAmpByGammaRc_,
                          Form("hist_amp_by_gamma_rc_%d", ig),
                          Form("hist_amp_by_gamma_rc_%d", ig),
                          BINS_CHANNEL, XLOW_CHANNEL, XUP_CHANNEL);
        addHistogramToRow(histsTimeCorrectedByGamma_,
                          Form("hist_time_corrected_by_gamma_%d", ig),
                          Form("hist_time_corrected_by_gamma_%d", ig),
                          BINS_TIME, XLOW_TIME, XUP_TIME);
        addHistogramToRow(histsEnergyByGamma_,
                          Form("hist_energy_by_gamma_%d", ig),
                          Form("hist_energy_by_gamma_%d", ig),
                          BINS_ENERGY, XLOW_ENERGY, XUP_ENERGY);
        addHistogramToRow(histsEnergyByGammaRc_,
                          Form("hist_energy_by_gamma_rc_%d", ig),
                          Form("hist_energy_by_gamma_rc_%d", ig),
                          BINS_ENERGY, XLOW_ENERGY, XUP_ENERGY);
    }

    for (auto ia{0}; ia < alphaNumber_; ia++) {
        addHistogramToRow(histsAmpByAlpha_,
                          Form("hist_amp_by_alpha_%d", ia),
                          Form("hist_amp_by_alpha_%d", ia),
                          BINS_CHANNEL, XLOW_CHANNEL, XUP_CHANNEL);
        addHistogramToRow(histsTimeCorrectedByAlpha_,
                          Form("hist_time_corrected_by_alpha_%d", ia),
                          Form("hist_time_corrected_by_alpha_%d", ia),
                          BINS_TIME, XLOW_TIME, XUP_TIME);
        addHistogramToRow(histsEnergyByAlpha_,
                          Form("hist_energy_by_alpha_%d", ia),
                          Form("hist_energy_by_alpha_%d", ia),
                          BINS_ENERGY, XLOW_ENERGY, XUP_ENERGY);
    }
}

HistogramManager::~HistogramManager()
{
}

void HistogramManager::resetAll()
{
    for (auto ig{0}; ig < gammaNumber_; ig++) {
        for (auto ia{0}; ia < alphaNumber_; ia++) {
            histsTimeByGammaAlpha_[ig][ia]->Reset();
            histsAmpByGammaAlphaSg_[ig][ia]->Reset();
            histsAmpByGammaAlphaBg_[ig][ia]->Reset();
            histsAmpByGammaAlphaRc_[ig][ia]->Reset();
            histsEnergyByGammaAlphaSg_[ig][ia]->Reset();
            histsEnergyByGammaAlphaBg_[ig][ia]->Reset();
            histsEnergyByGammaAlphaRc_[ig][ia]->Reset();
        }
    }
    for (auto ia{0}; ia < alphaNumber_; ia++) {
            histsAmpByAlpha_[ia]->Reset();
            histsTimeCorrectedByAlpha_[ia]->Reset();
            histsEnergyByAlpha_[ia]->Reset();
    }
    for (auto ig{0}; ig < gammaNumber_; ig++) {
            histsAmpByGamma_[ig]->Reset();
            histsAmpByGammaRc_[ig]->Reset();
            histsEnergyByGamma_[ig]->Reset();
            histsEnergyByGammaRc_[ig]->Reset();
    }
}

int HistogramManager::gammaNumber() const
{
    return gammaNumber_;
}

int HistogramManager::alphaNumber() const
{
    return alphaNumber_;
}

const std::vector<std::vector<std::unique_ptr<TH1D> > > &HistogramManager::histsAmpByGammaAlphaSg() const
{
    return histsAmpByGammaAlphaSg_;
}

const std::vector<std::vector<std::unique_ptr<TH1D> > > &HistogramManager::histsAmpByGammaAlphaBg() const
{
    return histsAmpByGammaAlphaBg_;
}

const std::vector<std::vector<std::unique_ptr<TH1D> > > &HistogramManager::histsAmpByGammaAlphaRc() const
{
    return histsAmpByGammaAlphaRc_;
}

const std::vector<std::vector<std::unique_ptr<TH1D> > > &HistogramManager::histsTimeByGammaAlpha() const
{
    return histsTimeByGammaAlpha_;
}

const std::vector<std::vector<std::unique_ptr<TH1D> > > &HistogramManager::histsTimeCorrectedByGammaAlpha() const
{
    return histsTimeCorrectedByGammaAlpha_;
}

const std::vector<std::unique_ptr<TH1D> > &HistogramManager::histsTimeCorrectedByAlpha() const
{
    return histsTimeCorrectedByAlpha_;
}

const std::vector<std::unique_ptr<TH1D> > &HistogramManager::histsTimeCorrectedByGamma() const
{
    return histsTimeCorrectedByGamma_;
}

const std::vector<std::unique_ptr<TH1D> > &HistogramManager::histsAmpByAlpha() const
{
    return histsAmpByAlpha_;
}

const std::vector<std::unique_ptr<TH1D> > &HistogramManager::histsAmpByGamma() const
{
    return histsAmpByGamma_;
}

const std::vector<std::unique_ptr<TH1D> > &HistogramManager::histsAmpByGammaRc() const
{
    return histsAmpByGammaRc_;
}

const std::vector<std::vector<std::unique_ptr<TH1D> > > &HistogramManager::histsEnergyByGammaAlphaSg() const
{
    return histsEnergyByGammaAlphaSg_;
}

const std::vector<std::vector<std::unique_ptr<TH1D> > > &HistogramManager::histsEnergyByGammaAlphaBg() const
{
    return histsEnergyByGammaAlphaBg_;
}

const std::vector<std::unique_ptr<TH1D> > &HistogramManager::histsEnergyByGamma() const
{
    return histsEnergyByGamma_;
}

const std::vector<std::unique_ptr<TH1D> > &HistogramManager::histsEnergyByAlpha() const
{
    return histsEnergyByAlpha_;
}

TH1D*HistogramManager::histTimeTotal() const
{
    return histTimeTotal_.get();
}

TH1D*HistogramManager::histEnergyTotal() const
{
    return histEnergyTotal_.get();
}

const std::vector<std::unique_ptr<TH1D> > &HistogramManager::histsEnergyByGammaRc() const
{
    return histsEnergyByGammaRc_;
}

const std::vector<std::vector<std::unique_ptr<TH1D> > > &HistogramManager::histsEnergyByGammaAlphaRc() const
{
    return histsEnergyByGammaAlphaRc_;
}
