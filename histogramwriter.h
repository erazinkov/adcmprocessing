#ifndef HISTOGRAMWRITER_H
#define HISTOGRAMWRITER_H

#include <TH1D.h>
#include <vector>

class HistogramWriter
{
public:
    HistogramWriter();

    HistogramWriter& addHist(TH1D *hist);
    HistogramWriter& addHists(const std::vector<TH1D *> &hists);

    bool write(const std::string &fileName, const std::string &option = "RECREATE");

private:
    std::vector<TH1D *> hists_;
};

#endif // HISTOGRAMWRITER_H
