#ifndef TIMEPEAKSFINDER_H
#define TIMEPEAKSFINDER_H

#include <TH1.h>

class TimePeaksFinder
{
public:
    TimePeaksFinder();
    static double calculatePeakPos(TH1 *hist);
    static std::pair<double, double> calculateResolution(TH1 *hist);
};

#endif // TIMEPEAKSFINDER_H
