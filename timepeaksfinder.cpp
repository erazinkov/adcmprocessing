#include "timepeaksfinder.h"

#include <TF1.h>
#include <TList.h>


TimePeaksFinder::TimePeaksFinder()
{
}

double TimePeaksFinder::calculatePeakPos(TH1 *hist)
{
    gErrorIgnoreLevel = 3'000;
    auto timePeakPos{0.0};

    auto binMax{hist->GetMaximumBin()};
    auto xMax{hist->GetBinCenter(hist->GetBin(binMax))};
    auto rcAmp{hist->GetBinContent(hist->GetXaxis()->FindBin(xMax - 25.0))};
    auto obPeakAmp{hist->GetBinContent(binMax) - rcAmp};
    auto snPeakAmp{0.5 * obPeakAmp};

    auto ff = [] (double *x, double *par) {
       double arg_1{0.0}, arg_2{0.0}, arg_3{0.0};
       if (par[2] != 0.0 && par[5] != 0.0 && par[8] != 0.0)
       {
           arg_1 = ( x[0] - par[1] ) / par[2];
           arg_2 = ( x[0] - ( par[1] + par[4] ) ) / par[5];
           arg_3 = x[0];
       }

       double fitval{
           par[0] * TMath::Exp( -0.5 * arg_1 * arg_1 ) +
           par[3] * TMath::Exp( -0.5 * arg_2 * arg_2 ) +
           par[6] + par[7] * arg_3
       };

       return fitval;
   };

    TF1 *f{new TF1("f", ff, xMax - 15.0, xMax + 25.0, 11)};

    f->SetParameter(0, obPeakAmp);
    f->SetParameter(1, xMax);
    f->SetParameter(2, 0.5 * ( 1.5 + 3.0 ));
    f->SetParameter(3, 0.05 * obPeakAmp);
    // 4 ?
    f->SetParameter(5, 2.5);
    f->SetParameter(6, rcAmp);
    f->FixParameter(7, 0.0);

    f->SetParLimits(1, 0.9 * xMax, 1.1 * xMax);
    f->SetParLimits(2, 1.5, 3.0);
    f->SetParLimits(3, 0.0, snPeakAmp);
    f->SetParLimits(4, 5.0, 20.0);
    f->SetParLimits(5, 2.0, 7.0);

    // hist->GetXaxis()->SetRangeUser(f->GetParameter(1) - 40.0, f->GetParameter(1) + 25.0);

    hist->Fit(f, "RQ0N");

    auto fff = [](double *x, double *par){
        double arg{0};
        if (par[2] != 0.0)
        {
            arg = ( x[0] - par[1] ) / par[2];
        }
        double fitval{par[0] * TMath::Exp(-0.5 * arg * arg) + par[3] + par[4] * x[0]};
        return fitval;
    };

    TF1 *fOb{new TF1("fOb", fff, xMax - 15.0, xMax + 25.0, 5)};
    fOb->SetParameters(f->GetParameter(0),
                       f->GetParameter(1),
                       f->GetParameter(2),
                       f->GetParameter(6),
                       f->GetParameter(7));
    fOb->SetLineColor(kGreen);
    TF1 *fSn{new TF1("fSn", fff, xMax - 15.0, xMax + 25.0, 5)};
    fSn->SetParameters(f->GetParameter(3),
                       f->GetParameter(1) + f->GetParameter(4),
                       f->GetParameter(5),
                       f->GetParameter(6),
                       f->GetParameter(7));
    fSn->SetLineColor(kBlue);

    hist->GetListOfFunctions()->Add(fOb);
    hist->GetListOfFunctions()->Add(fSn);

    timePeakPos = f->GetParameter(1);
//    timePeakPos = xMax;

    delete f;
    f = nullptr;

    gErrorIgnoreLevel = 0;

    return timePeakPos;
}

std::pair<double, double> TimePeaksFinder::calculateResolution(TH1 *hist)
{
    gErrorIgnoreLevel = 3'000;

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

    TF1 *f{new TF1("f", fff, xMax - 50.0, xMax + 50.0, 5)};

    f->SetParameter(0, obPeakAmp);
    f->SetParameter(1, xMax);
    f->SetParameter(2, 0.5 * ( 1.5 + 3.0 ));
    f->SetParameter(3, rcAmp);
    f->FixParameter(4, 0.0);

    hist->Fit(f, "RQ0N");

    TF1 *fOb{new TF1("fOb", fff, xMax - 50.0, xMax + 50.0, 5)};
    fOb->SetParameters(f->GetParameters());
    hist->GetListOfFunctions()->Add(fOb);

    std::pair<double, double> res{2.35 * f->GetParameter(2), 2.35 * f->GetParError(2)};

    delete f;
    f = nullptr;

    gErrorIgnoreLevel = 0;
    return res;
}
