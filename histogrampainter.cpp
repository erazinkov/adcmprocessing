#include "histogrampainter.h"

#include <TCanvas.h>
#include <TError.h>

#include <iostream>

void HistogramPainter::paintHist(TH1D *hist, const std::string &fileName)
{
    gErrorIgnoreLevel = 3'000;
    std::unique_ptr<TCanvas> c{new TCanvas("c", "c", 1024, 960)};
    c.get()->Print((fileName + '[').c_str());
    if (hist) {
        hist->Draw();
        auto listOfFunctions{hist->GetListOfFunctions()};
        for (auto *item : *listOfFunctions) {
            item->Draw("SAME");
        }
    } else {
        std::cout << "Can\'t paint " << hist->GetName() << " to file " << fileName << std::endl;
    }
    c.get()->Print(fileName.c_str());
    c.get()->Print((fileName + ']').c_str());
    c.get()->Delete();
    gErrorIgnoreLevel = 0;
}

void HistogramPainter::paintHists(const std::vector<TH1D *> &hists, const std::string &fileName)
{
    gErrorIgnoreLevel = 3'000;
    std::unique_ptr<TCanvas> c{new TCanvas("c", "c", 1024, 960)};
    c.get()->Print((fileName + '[').c_str());
    for (size_t ig{0}; ig < hists.size(); ++ig) {
        if (hists.at(ig)) {
            gPad->SetGrid();
             hists.at(ig)->GetXaxis()->SetRangeUser(1'000.0, 2'000.0);
            hists.at(ig)->Draw();
            auto listOfFunctions{hists.at(ig)->GetListOfFunctions()};
            for (auto *item : *listOfFunctions) {
                item->Draw("SAME");
            }
        } else {
            std::cout << "Can\'t paint " << hists.at(ig)->GetName() << " to file " << fileName << std::endl;
        }
        c.get()->Print(fileName.c_str());
    }
    c.get()->Print((fileName + ']').c_str());
    c.get()->Delete();
    gErrorIgnoreLevel = 0;
}

void HistogramPainter::paintHists(const std::vector<std::vector<TH1D *> > &hists, const std::string &fileName)
{
    gErrorIgnoreLevel = 3'000;
    std::unique_ptr<TCanvas> c{new TCanvas("c", "c", 1024, 960)};
    c.get()->Print((fileName + '[').c_str());
    for (size_t ig{0}; ig < hists.size(); ++ig) {
        auto cd{static_cast<int>(std::ceil(std::sqrt(hists.at(ig).size())))};
        c.get()->Divide(cd, cd);
        for (size_t ia{0}; ia <  hists.at(ig).size(); ++ia) {
            c.get()->cd(static_cast<int>(ia) + 1);
            if (hists.at(ig).at(ia)) {
                hists.at(ig).at(ia)->Draw();
                auto listOfFunctions{hists.at(ig).at(ia)->GetListOfFunctions()};
                for (auto *item : *listOfFunctions) {
                    item->Draw("SAME");
                }
            } else {
                std::cout << "Can\'t paint " << hists.at(ig).at(ia)->GetName() << " to file " << fileName << std::endl;
            }
        }
        c.get()->Print(fileName.c_str());
        c.get()->Clear();
    }
    c.get()->Print((fileName + ']').c_str());
    c.get()->Delete();
    gErrorIgnoreLevel = 0;
}
