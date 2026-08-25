#ifndef HISTOGRAMPAINTER_H
#define HISTOGRAMPAINTER_H

#include <TH1D.h>

class HistogramPainter
{
public:
    static void paintHist(TH1D *hist, const std::string &fileName);
    static void paintHists(const std::vector<TH1D *> &hists, const std::string &fileName);
    static void paintHists(const std::vector<std::vector<TH1D *>> &hists, const std::string &fileName);
private:
    HistogramPainter() = delete;
    HistogramPainter(const HistogramPainter&) = delete;
    HistogramPainter& operator=(const HistogramPainter&) = delete;
};

#endif // HISTOGRAMPAINTER_H
