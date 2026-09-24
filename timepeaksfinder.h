#ifndef TIMEPEAKSFINDER_H
#define TIMEPEAKSFINDER_H

#include <TH1.h>

class TimePeaksFinder
{
public:
    TimePeaksFinder();
    static double calculateObjPeakPos(TH1 *hist);
    static double calculateBoxPeakPos(TH1 *hist);
    static std::pair<double, double> calculateResolution(TH1 *hist);
};

#endif // TIMEPEAKSFINDER_H
