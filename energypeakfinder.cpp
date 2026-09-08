#include "energypeakfinder.h"

#include <TF1.h>
#include <TGraphErrors.h>
#include <TLine.h>
#include <TVirtualFitter.h>
#include <TList.h>

#include <vector>
#include <map>
#include <functional>

EnergyPeakFinder::EnergyPeakFinder() : calib_{1.0}, offset_{0.0}
{
    fCalib_ = new TF1("fCalib_", "pol2", 0.0, 8.0e3);
    fCalib_->SetParameter(0, 0.0);
    fCalib_->SetParameter(1, 1.0);
    fCalib_->SetParameter(2, 0.0);

    findPeakPosFunctions_ = {
        {EnergyPeak::Id::FE847, [&](TH1 *, TH1 *histRc) { return findPeakPosFerrum847(histRc, 1.2); }},
        {EnergyPeak::Id::FE1238, [&](TH1 *, TH1 *histRc) { return findPeakPosFerrum1238(histRc, 1.2); }},
        {EnergyPeak::Id::HYDROGEN, [&](TH1 *, TH1 *histRc) { return findPeakPosHydrogen(histRc, 1.2); }},
        {EnergyPeak::Id::SILICON, [&](TH1 *hist, TH1 *) { return findPeakPosSilicon(hist, 1.3); }},
        {EnergyPeak::Id::CARBON, [&](TH1 *hist, TH1 *) { return findPeakPosCarbon(hist, 1.35); }},
        {EnergyPeak::Id::OXYGEN, [&](TH1 *hist, TH1 *) { return findPeakPosOxygen(hist, 1.35); }},
        {EnergyPeak::Id::FE7631, [&](TH1 *, TH1 *histRc) { return findPeakPosFerrum7631(histRc, 1.5); }},
    };
}

EnergyPeakFinder::~EnergyPeakFinder()
{
    if (fCalib_) {
        delete fCalib_;
        fCalib_ = nullptr;
    }
}

void EnergyPeakFinder::process(TH1D *hist, TH1D *histRc)
{
    energyPeaks_.clear();
    fCalib_->SetParameters(0.0, 1.0, 0.0);
    TVirtualFitter::SetDefaultFitter("Minuit");

    std::vector<EnergyPeak> peaks{
                EnergyPeak{EnergyPeak::Id::FE847, 0.0},
                EnergyPeak{EnergyPeak::Id::FE1238, 0.0},
                 EnergyPeak{EnergyPeak::Id::HYDROGEN, 0.0},
//                EnergyPeak{EnergyPeak::Id::CARBON, 0.0},
                EnergyPeak{EnergyPeak::Id::SILICON, 0.0},
                EnergyPeak{EnergyPeak::Id::OXYGEN, 0.0},
//                EnergyPeak{EnergyPeak::Id::FE7631, 0.0},
    };

    auto fe847PosApprox{getFerrum847PosApprox(histRc)};

    offset_ = 0.0;
    calib_ = EnergyPeak::energyById(EnergyPeak::Id::FE847) / fe847PosApprox;
    fCalib_->SetParameter(1, EnergyPeak::energyById(EnergyPeak::Id::FE847) / fe847PosApprox);

    TGraphErrors graphPolN;

    auto fitGraph = [&](){
        auto n{graphPolN.GetN()};
        if (n == 0 || n == 1) {
            return;
        }
        if (n == 2) {
            fCalib_->ReleaseParameter(0);
            fCalib_->ReleaseParameter(1);
            fCalib_->FixParameter(2, 0.0);
        } else {
            fCalib_->ReleaseParameter(0);
            fCalib_->ReleaseParameter(1);
            fCalib_->ReleaseParameter(2);
        }
        graphPolN.Fit(fCalib_, "RQN0");
    };

    for (size_t i{0}; i < peaks.size(); i++) {
        auto peakPos{findPeakPosFunctions_.at(peaks.at(i).id())(hist, histRc)};
        peaks.at(i).setChannel(peakPos);
        graphPolN.SetPoint(i, peakPos, peaks.at(i).energy());
        fitGraph();
    }
    // for (size_t i{0}; i < peaks.size(); i++) {
    //     if (hist) {
    //         auto listOfFunctions{hist->GetListOfFunctions()};
    //         for (auto *item : *listOfFunctions) {
    //             // std::cout << item->GetName() << std::endl;
    //             // item->Delete();
    //         }
    //     }
    //     if (histRc) {
    //         auto listOfFunctionsRc{histRc->GetListOfFunctions()};
    //         for (auto *item : *listOfFunctionsRc) {
    //             // std::cout << item->GetName() << std::endl;
    //             // item->Delete();
    //         }
    //     }
    //     auto peakPos{findPeakPosFunctions_.at(peaks.at(i).id())(hist, histRc)};
    //     peaks.at(i).setChannel(peakPos);
    // }

    std::sort(peaks.begin(), peaks.end());

    energyPeaks_ = std::move(peaks);
}

void EnergyPeakFinder::check(TH1D *hist, TH1D *histRc)
{
    fCalib_->SetParameters(0.0, 1.0, 0.0);
    TVirtualFitter::SetDefaultFitter("Minuit");

    std::vector<EnergyPeak> peaks{
                                  // EnergyPeak{EnergyPeak::Id::FE847, 0.0},
                                  // EnergyPeak{EnergyPeak::Id::FE1238, 0.0},
                                  // EnergyPeak{EnergyPeak::Id::HYDROGEN, 0.0},
                                  EnergyPeak{EnergyPeak::Id::CARBON, 0.0},
                                  EnergyPeak{EnergyPeak::Id::OXYGEN, 0.0},
                                  };



    for (size_t i{0}; i < peaks.size(); i++) {
        auto peakPos{findPeakPosFunctions_.at(peaks.at(i).id())(hist, histRc)};
        std::cout << peakPos << std::endl;
    }

}

void EnergyPeakFinder::processRaw(TH1D *hist)
{
    auto binMax{hist->GetMaximumBin()};
    auto xMax{hist->GetBinCenter(hist->GetBin(binMax))};
    energyPeak_ = EnergyPeak(EnergyPeak::Id::FE847, xMax);
}

double EnergyPeakFinder::getFerrum847PosApprox(TH1 *h)
{
    double pos{h->GetXaxis()->GetBinCenter(h->GetMaximumBin())};
    return pos;

}

double EnergyPeakFinder::findPeakPosFerrum847(TH1 *h, const double A)
{
    double peakEnergy{EnergyPeak::energyById(EnergyPeak::Id::FE847)};
    double pos{fCalib_->GetX(peakEnergy)};
    double sigma{fCalib_->GetX(TMath::Sqrt(peakEnergy) * A)};

    double xL{pos - 3.0 * sigma};
    double xR{pos + 3.0 * sigma};

    auto y = [](const TH1 *h, const double &x){
        return h->GetBinContent(h->GetXaxis()->FindBin(x));
    };

    double yL{y(h, xL)};
    double yR{y(h, xR)};

    TF1 f("f", "gaus(0) + pol1(3)", xL, xR);
    // y = p0 + p1 * x
    double p1{(yR - yL) / (xR - xL)};
    double p0{yR - p1 * xR};

    f.SetParameter(0, y(h, pos) - yR);
    f.SetParameter(1, pos);
    f.SetParameter(2, sigma);
    f.SetParameter(3, p0);
    f.SetParameter(4, p1);

    f.SetParLimits(0, 0.0, h->GetMaximum());
    f.SetParLimits(1, xL, xR);
    f.SetParLimits(2, 0.5 * sigma, 1.5 * sigma);
    f.SetParLimits(4, -1.0e7, 0.0);
    h->Fit("f", "RQN0");

    pos = f.GetParameter(1);
    xL = pos - 3.0 * sigma;
    xR = pos + 3.0 * sigma;
    f.SetRange(xL, xR);
    h->Fit("f", "RQN0");

    TF1 *fP{new TF1("fP", "gaus(0) + pol1(3)", xL, xR)};
    fP->SetLineColor(kOrange);
    fP->SetParameters(f.GetParameters());
    h->GetListOfFunctions()->Add(fP);
    TF1 *fBg{new TF1("fBg", "pol1(0)", xL, xR)};
    fBg->SetLineColor(kGreen);
    fBg->SetParameters(f.GetParameters() + 3);
    h->GetListOfFunctions()->Add(fP);
    h->GetListOfFunctions()->Add(fBg);

    TLine *l{new TLine(pos, 0.0, pos, h->GetMaximum())};
    h->GetListOfFunctions()->Add(l);

    TLine *lPrev{new TLine(xL, yL, xR, yR)};
    lPrev->SetLineColor(kMagenta);
    h->GetListOfFunctions()->Add(lPrev);

    return f.GetParameter(1);
}

double EnergyPeakFinder::findPeakPosFerrum1238(TH1 *h, const double A)
{
    double peakEnergy{EnergyPeak::energyById(EnergyPeak::Id::FE1238)};
    double pos{fCalib_->GetX(peakEnergy)};
    double sigma{fCalib_->GetX(TMath::Sqrt(peakEnergy) * A)};

    double xL{pos - 3.0 * sigma};
    double xR{pos + 3.5 * sigma};

    auto y = [](const TH1 *h, const double &x){
        return h->GetBinContent(h->GetXaxis()->FindBin(x));
    };

    double yL{y(h, xL)};
    double yR{y(h, xR)};

    TF1 f("f", "gaus(0) + pol1(3)", xL, xR);
    // y = p0 + p1 * x
    double p1{(yR - yL) / (xR - xL)};
    double p0{yR - p1 * xR};

    f.SetParameter(0, y(h, pos) - yR);
    f.SetParameter(1, pos);
    f.SetParameter(2, sigma);
    f.SetParameter(3, p0);
    f.SetParameter(4, p1);

    f.SetParLimits(0, 0.0, h->GetMaximum());
    f.SetParLimits(1, xL, xR);
    f.SetParLimits(2, 0.5 * sigma, 1.5 * sigma);
    f.SetParLimits(4, -1.0e7, 0.0);
    h->Fit("f", "RQN0");

    pos = f.GetParameter(1);
    xL = pos - 3.0 * sigma;
    xR = pos + 3.0 * sigma;
    f.SetRange(xL, xR);
    h->Fit("f", "RQN0");

    TF1 *fP{new TF1("fP", "gaus(0) + pol1(3)", xL, xR)};
    fP->SetLineColor(kOrange);
    fP->SetParameters(f.GetParameters());
    h->GetListOfFunctions()->Add(fP);
    TF1 *fBg{new TF1("fBg", "pol1(0)", xL, xR)};
    fBg->SetLineColor(kGreen);
    fBg->SetParameters(f.GetParameters() + 3);
    h->GetListOfFunctions()->Add(fP);
    h->GetListOfFunctions()->Add(fBg);

    TLine *l{new TLine(pos, 0.0, pos, h->GetMaximum())};
    h->GetListOfFunctions()->Add(l);

    TLine *lPrev{new TLine(xL, yL, xR, yR)};
    lPrev->SetLineColor(kMagenta);
    h->GetListOfFunctions()->Add(lPrev);

    return f.GetParameter(1);
}

double EnergyPeakFinder::findPeakPosHydrogen(TH1 *h, const double A)
{
    double peakEnergyAdd{EnergyPeak::energyById(EnergyPeak::Id::HYDROGENADD)};
    double posAdd{fCalib_->GetX(peakEnergyAdd)};
    double sigmaAdd{fCalib_->GetX(TMath::Sqrt(peakEnergyAdd) * A)};

    double peakEnergy{EnergyPeak::energyById(EnergyPeak::Id::HYDROGEN)};
    double pos{fCalib_->GetX(peakEnergy)};
    double sigma{fCalib_->GetX(TMath::Sqrt(peakEnergy) * A)};

    double xL{posAdd - 3.0 * sigmaAdd};
    double xR{pos + 3.0 * sigma};

    auto y = [](const TH1 *h, const double &x){
        return h->GetBinContent(h->GetXaxis()->FindBin(x));
    };

    double yL{y(h, xL)};
    double yR{y(h, xR)};

    auto ff = [=](double *x, double *par){
        double arg_1{0.0}, arg_2{0.0};
        if (par[2] != 0.0)
        {
            arg_1 = ( x[0] - par[1] ) / par[2];
            arg_2 = ( x[0] - ( par[1] - (pos - posAdd) ) ) / par[2];
        }
        double fitval{
            par[0] * TMath::Exp(-0.5 * arg_1 * arg_1)+
            par[0] * par[3] * TMath::Exp(-0.5 * arg_2 * arg_2)+
            par[4] + par[5] * x[0]
        };

        return fitval;
    };

    TF1 f("f", ff, xL, xR, 6);
    // y = p0 + p1 * x
    double p1{(yR - yL) / (xR - xL)};
    double p0{yR - p1 * xR};

//    f.SetParameter(0, y(h, pos) - yR);
    f.SetParameter(0, 1.0e1);
    f.SetParameter(1, pos);
    f.SetParameter(2, sigma);
    f.SetParameter(3, 0.1);
    f.SetParameter(4, p0);
    f.SetParameter(5, p1);

     f.SetParLimits(0, 0.0, h->GetMaximum());
    f.SetParLimits(1, xL, xR);
    f.SetParLimits(2, 0.5 * sigma, 1.5 * sigma);
    f.SetParLimits(3, 0.05, 0.15);
    f.SetParLimits(5, -1.0e7, 0.0);
    h->Fit("f","RQN0");

    posAdd = f.GetParameter(1) - (pos - posAdd);
    pos = f.GetParameter(1);
    xL = posAdd - 3.0 * sigmaAdd;
    xR = pos + 3.0 * sigma;
    f.SetRange(xL, xR);
    h->Fit("f", "RQN0");

    TF1 *fP{new TF1("fP", ff, xL, xR, 6)};
    fP->SetLineColor(kOrange);
    fP->SetParameters(f.GetParameters());
    h->GetListOfFunctions()->Add(fP);
    TF1 *fBg{new TF1("fBg", "pol1(0)", xL, xR)};
    fBg->SetLineColor(kGreen);
    fBg->SetParameters(f.GetParameters() + 4);
    h->GetListOfFunctions()->Add(fP);
    h->GetListOfFunctions()->Add(fBg);

    TLine *l{new TLine(pos, 0.0, pos, h->GetMaximum())};
    TLine *lAdd{new TLine(posAdd, 0.0, posAdd, h->GetMaximum())};
    h->GetListOfFunctions()->Add(l);
    h->GetListOfFunctions()->Add(lAdd);

    TF1 *fP_primary{new TF1("fP_primary", "gaus(0) + pol1(3)", xL, xR)};
    fP_primary->SetParameter(0, f.GetParameter(0));
    fP_primary->SetParameter(1, f.GetParameter(1));
    fP_primary->SetParameter(2, f.GetParameter(2));
    fP_primary->SetParameter(3, f.GetParameter(4));
    fP_primary->SetParameter(4, f.GetParameter(5));
    fP_primary->SetLineColor(kBlue);
    TF1 *fP_add{new TF1("fP_add", "gaus(0) + pol1(3)", xL, xR)};
    fP_add->SetParameter(0, f.GetParameter(0) * f.GetParameter(3));
    fP_add->SetParameter(1, f.GetParameter(1) - (pos - posAdd));
    fP_add->SetParameter(2, f.GetParameter(2));
    fP_add->SetParameter(3, f.GetParameter(4));
    fP_add->SetParameter(4, f.GetParameter(5));
    fP_add->SetLineColor(kBlue + 1);


    h->GetListOfFunctions()->Add(fP_primary);
    h->GetListOfFunctions()->Add(fP_add);

    TLine *lPrev{new TLine(xL, yL, xR, yR)};
    lPrev->SetLineColor(kMagenta);
    h->GetListOfFunctions()->Add(lPrev);

    return f.GetParameter(1);
}

double EnergyPeakFinder::findPeakPosSilicon(TH1 *h, const double A)
{
    double peakEnergy{EnergyPeak::energyById(EnergyPeak::Id::SILICON)};
    double pos{fCalib_->GetX(peakEnergy)};
    double sigma{2 * fCalib_->GetX(TMath::Sqrt(peakEnergy) * A)};

    double xL{pos - 3.0 * sigma};
    double xR{pos + 3.0 * sigma};

    auto y = [](const TH1 *h, const double &x){
        return h->GetBinContent(h->GetXaxis()->FindBin(x));
    };

    double yL{y(h, xL)};
    double yR{y(h, xR)};

    TF1 f("f", "gaus(0) + pol1(3)", xL, xR);
    // y = p0 + p1 * x
    double p1{(yR - yL) / (xR - xL)};
    double p0{yR - p1 * xR};

//    f.SetParameter(0, y(h, pos) - yR);
    f.SetParameter(1, pos);
    f.SetParameter(2, sigma);
    f.SetParameter(3, p0);
    f.SetParameter(4, p1);

//    f.SetParLimits(0, 0.0, h->GetMaximum());
//    f.SetParLimits(1, xL, xR);
//    f.SetParLimits(2, 0.25 * sigma, 1.75 * sigma);
//    h->Fit("f","RQN0");

    pos = f.GetParameter(1);
    xL = pos - 3.0 * sigma;
    xR = pos + 3.0 * sigma;
    f.SetRange(xL, xR);
    h->Fit("f", "RQN0");

    TF1 *fP{new TF1("fP", "gaus(0) + pol1(3)", xL, xR)};
    fP->SetLineColor(kOrange);
    fP->SetParameters(f.GetParameters());
    h->GetListOfFunctions()->Add(fP);
    TF1 *fBg{new TF1("fBg", "pol1(0)", xL, xR)};
    fBg->SetLineColor(kGreen);
    fBg->SetParameters(f.GetParameters() + 3);
    h->GetListOfFunctions()->Add(fP);
    h->GetListOfFunctions()->Add(fBg);

    TLine *l{new TLine(pos, 0.0, pos, h->GetMaximum())};
    h->GetListOfFunctions()->Add(l);

    TLine *lPrev{new TLine(xL, yL, xR, yR)};
    lPrev->SetLineColor(kMagenta);
    h->GetListOfFunctions()->Add(lPrev);

    return f.GetParameter(1);
}

double EnergyPeakFinder::findPeakPosCarbon(TH1 *h, const double A)
{
    double peakEnergy{EnergyPeak::energyById(EnergyPeak::Id::CARBON)};
    double pos{fCalib_->GetX(peakEnergy)};
    double sigma{2 * fCalib_->GetX(TMath::Sqrt(peakEnergy) * A)};

    double xL{pos - 3.0 * sigma};
    double xR{pos + 3.0 * sigma};

    auto y = [](const TH1 *h, const double &x){
        return h->GetBinContent(h->GetXaxis()->FindBin(x));
    };

    double yL{y(h, xL)};
    double yR{y(h, xR)};

    TF1 f("f", "gaus(0) + pol1(3)", xL, xR);
    // y = p0 + p1 * x
    double p1{(yR - yL) / (xR - xL)};
    double p0{yR - p1 * xR};

    f.SetParameter(0, y(h, pos) - yR);
    f.SetParameter(1, pos);
    f.SetParameter(2, sigma);
    f.SetParameter(3, p0);
    f.SetParameter(4, p1);

    f.SetParLimits(0, 0.0, h->GetMaximum());
    f.SetParLimits(1, xL, xR);
    f.SetParLimits(2, 0.25 * sigma, 1.75 * sigma);
    h->Fit("f","RQN0");

//    pos = f.GetParameter(1);
//    xL = pos - 3.0 * sigma;
//    xR = pos + 3.0 * sigma;
//    f.SetRange(xL, xR);
//    h->Fit("f", "RQN0");

    TF1 *fP{new TF1("fP", "gaus(0) + pol1(3)", xL, xR)};
    fP->SetLineColor(kOrange);
    fP->SetParameters(f.GetParameters());
    h->GetListOfFunctions()->Add(fP);
    TF1 *fBg{new TF1("fBg", "pol1(0)", xL, xR)};
    fBg->SetLineColor(kGreen);
    fBg->SetParameters(f.GetParameters() + 3);
    h->GetListOfFunctions()->Add(fP);
    h->GetListOfFunctions()->Add(fBg);

    TLine *l{new TLine(pos, 0.0, pos, h->GetMaximum())};
    h->GetListOfFunctions()->Add(l);

    TLine *lPrev{new TLine(xL, yL, xR, yR)};
    lPrev->SetLineColor(kMagenta);
    h->GetListOfFunctions()->Add(lPrev);

    return f.GetParameter(1);
}

double EnergyPeakFinder::findPeakPosOxygen(TH1 *h, const double A)
{
    double peakEnergyAdd{EnergyPeak::energyById(EnergyPeak::Id::OXYGENADD)};
    double posAdd{fCalib_->GetX(peakEnergyAdd)};
    double sigmaAdd{fCalib_->GetX(TMath::Sqrt(peakEnergyAdd) * A)};

    double peakEnergy{EnergyPeak::energyById(EnergyPeak::Id::OXYGEN)};
    double pos{fCalib_->GetX(peakEnergy)};
    double sigma{fCalib_->GetX(TMath::Sqrt(peakEnergy) * A)};

    double xL{posAdd - 3.0 * sigmaAdd};
    double xR{pos + 3.0 * sigma};

    auto y = [](const TH1 *h, const double &x){
        return h->GetBinContent(h->GetXaxis()->FindBin(x));
    };

    double yL{y(h, xL)};
    double yR{y(h, xR)};

    auto ff = [=](double *x, double *par){
        double arg_1{0.0}, arg_2{0.0};
        if (par[2] != 0.0)
        {
            arg_1 = ( x[0] - par[1] ) / par[2];
            arg_2 = ( x[0] - ( par[1] - (pos - posAdd) ) ) / par[2];

        }
        double fitval{
            par[0] * TMath::Exp(-0.5 * arg_1 * arg_1)+
            par[0] * par[3] * TMath::Exp(-0.5 * arg_2 * arg_2)+
            par[4] + par[5] * x[0]
        };

        return fitval;
    };

    TF1 f("f", ff, xL, xR, 6);
    // y = p0 + p1 * x
    double p1{(yR - yL) / (xR - xL)};
    double p0{yR - p1 * xR};

    // f.SetParameter(0, y(h, pos) - yR);
    f.SetParameter(1, pos);
    f.SetParameter(2, sigma);
    f.SetParameter(3, 0.5);
    f.SetParameter(4, p0);
    f.SetParameter(5, p1);

    f.SetParLimits(0, 0.0, h->GetMaximum());
    f.SetParLimits(1, xL, xR);
    f.SetParLimits(2, 0.5 * sigma, 1.5 * sigma);
    f.SetParLimits(3, 0.25, 0.75);
//    f.SetParLimits(5, -1.0e7, 0.0);
    h->Fit("f","RQN0");

    posAdd = f.GetParameter(1) - (pos - posAdd);
    pos = f.GetParameter(1);
    xL = posAdd - 3.0 * sigmaAdd;
    xR = pos + 3.0 * sigma;
    f.SetRange(xL, xR);
    h->Fit("f", "RQN0");

    TF1 *fP{new TF1("fP", ff, xL, xR, 6)};
    fP->SetLineColor(kOrange);
    fP->SetParameters(f.GetParameters());
    h->GetListOfFunctions()->Add(fP);
    TF1 *fBg{new TF1("fBg", "pol1(0)", xL, xR)};
    fBg->SetLineColor(kGreen);
    fBg->SetParameters(f.GetParameters() + 4);
    h->GetListOfFunctions()->Add(fP);
    h->GetListOfFunctions()->Add(fBg);

    TLine *l{new TLine(pos, 0.0, pos, h->GetMaximum())};
    TLine *lAdd{new TLine(posAdd, 0.0, posAdd, h->GetMaximum())};
    lAdd->SetLineColor(kMagenta);
    h->GetListOfFunctions()->Add(l);
    h->GetListOfFunctions()->Add(lAdd);

    TLine *lPrev{new TLine(xL, yL, xR, yR)};
    lPrev->SetLineColor(kMagenta);
    h->GetListOfFunctions()->Add(lPrev);

    return f.GetParameter(1);
}

double EnergyPeakFinder::findPeakPosFerrum7631(TH1 *h, const double A)
{

    double peakEnergyAdd{EnergyPeak::energyById(EnergyPeak::Id::FE7631ADD)};
    peakEnergyAdd -= 125.0;
    double posAdd{fCalib_->GetX(peakEnergyAdd)};
    double sigmaAdd{fCalib_->GetX(TMath::Sqrt(peakEnergyAdd) * A)};

    double peakEnergy{EnergyPeak::energyById(EnergyPeak::Id::FE7631)};
    double pos{fCalib_->GetX(peakEnergy)};
    double sigma{fCalib_->GetX(TMath::Sqrt(peakEnergy) * A)};

    double xL{posAdd - 3.0 * sigmaAdd};
    double xR{pos + 3.0 * sigma};

    auto y = [](const TH1 *h, const double &x){
        return h->GetBinContent(h->GetXaxis()->FindBin(x));
    };

    double yL{y(h, xL)};
    double yR{y(h, xR)};

    auto ff = [=](double *x, double *par){
        double arg_1{0.0}, arg_2{0.0};
        if (par[2] != 0.0)
        {
            arg_1 = ( x[0] - par[1] ) / par[2];
            arg_2 = ( x[0] - ( par[1] - (pos - posAdd) ) ) / par[2];

        }
        double fitval{
            par[0] * TMath::Exp(-0.5 * arg_1 * arg_1)+
            par[0] * par[3] * TMath::Exp(-0.5 * arg_2 * arg_2)+
            par[4] + par[5] * x[0]
        };

        return fitval;
    };

    TF1 f("f", ff, xL, xR, 6);
    // y = p0 + p1 * x
    double p1{(yR - yL) / (xR - xL)};
    double p0{yR - p1 * xR};

    // f.SetParameter(0, y(h, pos) - yR);
    f.SetParameter(1, pos);
    f.SetParameter(2, sigma);
    f.SetParameter(3, 1.0);
    f.SetParameter(4, p0);
    f.SetParameter(5, p1);

    f.SetParameter(6, pos - posAdd);

    f.SetParLimits(0, 0.0, h->GetMaximum());
    f.SetParLimits(1, xL, xR);
    f.SetParLimits(2, 0.25 * sigma, 1.75 * sigma);
//    f.SetParLimits(3, 0.25, 1.75);
//    f.SetParLimits(5, -1.0e7, 0.0);
//    f.SetParLimits(6, 0.75 * (pos - posAdd), 1.25 * (pos - posAdd));
    h->Fit("f","RQN0");

    posAdd = f.GetParameter(1) - (pos - posAdd);
    pos = f.GetParameter(1);
    xL = posAdd - 3.0 * sigmaAdd;
    xR = pos + 3.0 * sigma;
    f.SetRange(xL, xR);
    h->Fit("f", "RQN0");

    TF1 *fP{new TF1("fP", ff, xL, xR, 6)};
    fP->SetLineColor(kOrange);
    fP->SetParameters(f.GetParameters());
    h->GetListOfFunctions()->Add(fP);
    TF1 *fBg{new TF1("fBg", "pol1(0)", xL, xR)};
    fBg->SetLineColor(kGreen);
    fBg->SetParameter(0, f.GetParameter(4));
    fBg->SetParameter(1, f.GetParameter(5));
    h->GetListOfFunctions()->Add(fP);
    h->GetListOfFunctions()->Add(fBg);

    TLine *l{new TLine(pos, 0.0, pos, h->GetMaximum())};
    TLine *lAdd{new TLine(posAdd, 0.0, posAdd, h->GetMaximum())};
    lAdd->SetLineColor(kMagenta);
    h->GetListOfFunctions()->Add(l);
    h->GetListOfFunctions()->Add(lAdd);

    TLine *lPrev{new TLine(xL, yL, xR, yR)};
    lPrev->SetLineColor(kMagenta);
    h->GetListOfFunctions()->Add(lPrev);

    return f.GetParameter(1);
}


const EnergyPeak &EnergyPeakFinder::energyPeak() const
{
    return energyPeak_;
}

std::pair<double, double> EnergyPeakFinder::calculateResolution(TH1 *hist)
{
    double pos{4438.0};
    double dL{250.0};
    TF1 f("f", "gaus(0) + pol2(3)", pos - dL, pos + dL);
    double amp{hist->GetBinContent(hist->GetXaxis()->FindBin(pos)) - hist->GetBinContent(hist->GetXaxis()->FindBin(pos + dL))};

    f.SetParameters(amp, pos, dL * 0.25, 0.0, 0.0, 0.0);
    f.SetParLimits(0, 0.0, 1.0e5);
    f.SetParLimits(1, pos - dL * 0.5, pos + dL * 0.5);
    f.SetParLimits(2, dL * 0.05, dL);
    f.SetParLimits(3, 0.0, hist->GetBinContent(hist->GetXaxis()->FindBin(pos)));
    f.SetParLimits(4, 0.0, -1.0 * DBL_MAX);
    f.FixParameter(5, 0.0);
    hist->Fit(&f, "RQN0");

    TF1 *fP{new TF1("fP", "gaus(0) + pol2(3)", pos - dL, pos + dL)};
    fP->SetLineColor(kRed);
    fP->SetParameters(f.GetParameters());
    hist->GetListOfFunctions()->Add(fP);
    TF1 *fBg{new TF1("fBg", "pol2(0)", pos - dL, pos + dL)};
    fBg->SetLineColor(kGreen);
    fBg->SetParameters(f.GetParameters() + 3);
    hist->GetListOfFunctions()->Add(fP);
    hist->GetListOfFunctions()->Add(fBg);

    std::pair<double, double> res{100.0 * 2.35 * f.GetParameter(2) / 4438.0, 100.0 * 2.35 * f.GetParError(2) / 4438.0};

    return res;
}

const std::vector<EnergyPeak> &EnergyPeakFinder::energyPeaks() const
{
    return energyPeaks_;
}

