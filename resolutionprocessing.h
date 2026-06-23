#ifndef RESOLUTIONPROCESSING_H
#define RESOLUTIONPROCESSING_H

#include <iostream>
#include <TH1.h>

class ResolutionProcessing
{
public:
    ResolutionProcessing();
    void processingEnergy(const std::string &filePath, const std::vector<TH1D *> &hists, const size_t &gammaNumber);
    void processingTime(const std::string &filePath, const std::vector<TH1D *> &hists, const size_t &gammaNumber);
    void exportToCSV(const std::string &filePath);

private:
    std::vector<std::pair<double, double>> energyResolutionsByGamma_;
    std::vector<std::pair<double, double>> timeResolutionsByGamma_;

    std::pair<double, double> energyResolution(TH1 *hist);
    std::pair<double, double> timeResolution(TH1 *hist);
};

#endif // RESOLUTIONPROCESSING_H
