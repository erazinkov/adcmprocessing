#include "histogramwriter.h"

#include <limits>
#include <TFile.h>
#include <iostream>

HistogramWriter::HistogramWriter()
{

}

HistogramWriter &HistogramWriter::addHist(TH1D *hist) {
    if (hist) {
        hists_.push_back(hist);
    }
    return *this;
}

HistogramWriter &HistogramWriter::addHists(const std::vector<TH1D *> &hists) {
    for (const auto &hist : hists) {
        if (hist && hist->Integral() > std::numeric_limits<double>::epsilon()) {
            hists_.push_back(hist);
        }
    }
    return *this;
}

bool HistogramWriter::write(const std::string &fileName, const std::string &option) {
    std::unique_ptr<TFile> file{TFile::Open(fileName.c_str(), option.c_str())};
    if (!file.get() || file.get()->IsZombie()) {
        std::cout << "Error: Can\'t open output file " << fileName << std::endl;
        return false;
    }
    if (hists_.empty()) {
        std::cout << "Warning: No hists to write to " << fileName << std::endl;
    }
    for (const auto &hist : hists_) {
        if (hist) {
            file.get()->Write();
        }
    }
    file->Close();
    return true;
}
