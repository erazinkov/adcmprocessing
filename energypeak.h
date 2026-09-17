#ifndef ENERGYPEAK_H
#define ENERGYPEAK_H

#include <iostream>

class EnergyPeak
{
public:
    enum class Id {
        FE847,
        FE1238,
        HYDROGENADD,
        HYDROGEN,
        CARBON,
        SILICON,
        OXYGEN,
        OXYGENADD,
        FE7631,
        FE7631ADD
    };
    EnergyPeak(EnergyPeak::Id id, double channel, double channelErr = 0.0);
    EnergyPeak::Id id() const;

    double channel() const;

    double energy() const;

    void setChannel(double newChannel);

    static double energyById(EnergyPeak::Id id);

    bool operator<(const EnergyPeak& other) const {
        return channel_ < other.channel_;
    }

    double channelErr() const;

    void setChannelErr(double newChannelErr);

private:
    EnergyPeak::Id id_;
    double channel_;
    double channelErr_;
    double energy_;
};

#endif // ENERGYPEAK_H
