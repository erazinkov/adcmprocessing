#ifndef TIMEPEAKSFINDER_H
#define TIMEPEAKSFINDER_H

#include <TH1.h>

class TimePeaksFinder
{
public:
    TimePeaksFinder();
    static double calculatePeakPos(TH1 *hist);
    static std::pair<double, double> calculateResolution(TH1 *hist);
    static void saveParameters(TH1 *hist, const std::string &fileName);
};

#endif // TIMEPEAKSFINDER_H
