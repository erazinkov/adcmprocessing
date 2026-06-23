#include "resolutionprocessing.h"

#include <TF1.h>
#include <TH1.h>
#include <TList.h>
#include <TCanvas.h>
#include <TFile.h>
#include <TGraph.h>
#include <TStyle.h>

#include <algorithm>
#include <fstream>

ResolutionProcessing::ResolutionProcessing()
{

}

void ResolutionProcessing::processingEnergy(const std::string &filePath, const std::vector<TH1D *> &hists, const size_t &gammaNumber)
{
    energyResolutionsByGamma_.clear();
    TH1D *hS{nullptr};
    const std::string psName{filePath + "_energy_res.ps"};
    TCanvas *c = new TCanvas("c", "c", 1024, 960);
    c->Print((psName + '[').c_str());
    gPad->SetGrid(1, 1);
    for (size_t i{0}; i < std::min(hists.size(), gammaNumber); ++i) {
        if (i == 0) {
            hS = static_cast<TH1D *>(hists.at(i)->Clone("hS"));
            hS->SetTitle(hS->GetName());
        } else {
            hS->Add(hists.at(i));
        }
        TH1D *h = static_cast<TH1D *>(hists.at(i)->Clone());
        energyResolutionsByGamma_.emplace_back(energyResolution(h));
        h->Draw();
        auto listOfFunctions{h->GetListOfFunctions()};
        for (auto *item : *listOfFunctions) {
            item->Draw("SAME");
        }
        c->Print(psName.c_str());
        delete h;
        h = nullptr;
    }
    energyResolutionsByGamma_.emplace_back(energyResolution(hS));
    hS->Draw();
    auto listOfFunctions{hS->GetListOfFunctions()};
    for (auto *item : *listOfFunctions) {
        item->Draw("SAME");
    }
    c->Print(psName.c_str());
    c->Print((psName + ']').c_str());
    TFile f{"tmp.root", "RECREATE"};
    hS->Write();
    f.Close();
    delete hS;
    delete c;
}

void ResolutionProcessing::processingTime(const std::string &filePath, const std::vector<TH1D *> &hists, const size_t &gammaNumber)
{
    timeResolutionsByGamma_.clear();
    TH1D *hS{nullptr};
    const std::string psName{filePath + "_time_res.ps"};
    TCanvas *c = new TCanvas("c", "c", 1024, 960);
    c->Print((psName + '[').c_str());
    gPad->SetGrid(1, 1);
    for (size_t i{0}; i < std::min(hists.size(), gammaNumber); ++i) {
        if (i == 0) {
            hS = static_cast<TH1D *>(hists.at(i)->Clone("hS"));
            hS->SetTitle(hS->GetName());
        } else {
            hS->Add(hists.at(i));
        }
        TH1D *h = static_cast<TH1D *>(hists.at(i)->Clone());
        timeResolutionsByGamma_.emplace_back(timeResolution(h));
        h->Draw();
        auto listOfFunctions{h->GetListOfFunctions()};
        for (auto *item : *listOfFunctions) {
            item->Draw("SAME");

        }
        c->Print(psName.c_str());
        delete h;
        h = nullptr;
    }
    timeResolutionsByGamma_.emplace_back(timeResolution(hS));
    hS->Draw();
    auto listOfFunctions{hS->GetListOfFunctions()};
    for (auto *item : *listOfFunctions) {
        item->Draw("SAME");
    }
    c->Print(psName.c_str());
    c->Print((psName + ']').c_str());
    delete hS;
    delete c;
}

void ResolutionProcessing::exportToCSV(const std::string &filePath)
{
    std::ofstream file(filePath + "_energy_time_table.csv");

    if (!file.is_open()) {
        std::clog << "Can't open file" << filePath + "_energy_time_table.csv" << std::endl;
        return;
    }
    for (size_t i{0}; i < energyResolutionsByGamma_.size(); i++) {
        auto idx = (i == energyResolutionsByGamma_.size() - 1) ? 111 : i;
        file << idx << "," << energyResolutionsByGamma_.at(i).first << ","<< energyResolutionsByGamma_.at(i).second << "\n";
        std::cout << idx << "," << energyResolutionsByGamma_.at(i).first << ","<< energyResolutionsByGamma_.at(i).second << "\n";
    }
    for (size_t i{0}; i < timeResolutionsByGamma_.size(); i++) {
        auto idx = (i == timeResolutionsByGamma_.size() - 1) ? 111 : i;
        file << idx << "," << timeResolutionsByGamma_.at(i).first << ","<< timeResolutionsByGamma_.at(i).second << "\n";
        std::cout << idx << "," << timeResolutionsByGamma_.at(i).first << ","<< timeResolutionsByGamma_.at(i).second << "\n";
    }
    file.close();
}

std::pair<double, double> ResolutionProcessing::energyResolution(TH1 *hist)
{
    gErrorIgnoreLevel = 3'000;
    gStyle->SetOptFit(1111);
    double posE{4438.0 - 511.0};
    double posP{4438.0};
    double dL{250.0};

    auto ff = [&](double *x, double *par){
        double arg_1{0.0}, arg_2{0.0};
        if (par[2] != 0.0)
        {
            arg_1 = ( x[0] - par[1] ) / par[2];
            arg_2 = ( x[0] - par[4] ) / par[5];

        }
        double fitval{
            par[0] * TMath::Exp(-0.5 * arg_1 * arg_1)+
            par[3] * TMath::Exp(-0.5 * arg_2 * arg_2)+
            par[6] + par[7] * x[0] + par[8] * x[0] * x[0]
        };

        return fitval;
    };

    TGraph gr2;
    gr2.SetPoint(0, posE - dL, hist->GetBinContent(hist->GetXaxis()->FindBin(posE - dL)));
    gr2.SetPoint(1, posP - dL, hist->GetBinContent(hist->GetXaxis()->FindBin(posP - dL)));
    gr2.SetPoint(2, posP + dL, hist->GetBinContent(hist->GetXaxis()->FindBin(posP + dL)));

    TF1 f2("f2", "pol2", posE - dL, posP + dL);
    gr2.Fit(&f2, "RQN0");


    TF1 f("f", ff, posE - dL, posP + dL, 9);
    double ampP{hist->GetBinContent(hist->GetXaxis()->FindBin(posP)) - hist->GetBinContent(hist->GetXaxis()->FindBin(posP + dL))};
    double ampE{hist->GetBinContent(hist->GetXaxis()->FindBin(posE)) - hist->GetBinContent(hist->GetXaxis()->FindBin(posP + dL))};

    f.SetParameters(ampE, posE, dL * 0.25, ampP, posP, dL * 0.25, f2.GetParameter(0), f2.GetParameter(1), f2.GetParameter(2));
    f.SetParLimits(0, 0.0, 1.0e5);
    f.SetParLimits(1, posE - dL * 0.5, posE + dL * 0.5);
    f.SetParLimits(2, dL * 0.05, dL);
    f.SetParLimits(3, 0.0, 1.0e5);
    f.SetParLimits(4, posP - dL * 0.5, posP + dL * 0.5);
    f.SetParLimits(5, dL * 0.05, dL);
    hist->Fit(&f, "RQ0");

    TF1 *fP{new TF1("fP", ff, posE - dL, posP + dL, 9)};
    fP->SetLineColor(kRed);
    fP->SetParameters(f.GetParameters());
    TF1 *fBg{new TF1("fBg", "pol2", posE - dL, posP + dL)};
    fBg->SetLineColor(kGreen);
    fBg->SetParameters(f.GetParameters() + 6);
    hist->GetListOfFunctions()->Add(fP);
    hist->GetListOfFunctions()->Add(fBg);

    gErrorIgnoreLevel = 0;

    return std::pair<double, double>{100.0 * 2.35 * f.GetParameter(5) / 4438.0, 100.0 * 2.35 * f.GetParError(5) / 4438.0};
}

std::pair<double, double> ResolutionProcessing::timeResolution(TH1 *hist)
{
    gErrorIgnoreLevel = 3'000;
    gStyle->SetOptFit(1111);
    auto binMax{hist->GetMaximumBin()};
    auto xMax{hist->GetBinCenter(hist->GetBin(binMax))};
    auto rcAmp{hist->GetBinContent(hist->GetXaxis()->FindBin(xMax - 25.0))};
    auto obPeakAmp{hist->GetBinContent(binMax) - rcAmp};

    auto fff = [](double *x, double *par){
        double arg{0};
        if (par[2] != 0.0)
        {
            arg = ( x[0] - par[1] ) / par[2];
        }
        double fitval{par[0] * TMath::Exp(-0.5 * arg * arg) + par[3] + par[4] * x[0]};
        return fitval;
    };

    TF1 f{"f", fff, xMax - 50.0, xMax + 50.0, 5};

    f.SetParameter(0, obPeakAmp);
    f.SetParameter(1, xMax);
    f.SetParameter(2, 0.5 * ( 1.5 + 3.0 ));
    f.SetParameter(3, rcAmp);
    f.FixParameter(4, 0.0);

    hist->Fit(&f, "RQ0");

    TF1 *fOb{new TF1("fOb", fff, xMax - 50.0, xMax + 50.0, 5)};
    fOb->SetParameters(f.GetParameters());
    hist->GetListOfFunctions()->Add(fOb);

    gErrorIgnoreLevel = 0;
    return std::pair<double, double>{2.35 * f.GetParameter(2), 2.35 * f.GetParError(2)};
}

