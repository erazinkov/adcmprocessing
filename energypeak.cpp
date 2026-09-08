#include "energypeak.h"

EnergyPeak::EnergyPeak(EnergyPeak::Id id, double channel) :
    id_(id), channel_(channel)
{
    energy_ = energyById(id);
}

EnergyPeak::Id EnergyPeak::id() const
{
    return id_;
}

double EnergyPeak::channel() const
{
    return channel_;
}

double EnergyPeak::energy() const
{
    return energy_;
}

void EnergyPeak::setChannel(double newChannel)
{
    channel_ = newChannel;
}

double EnergyPeak::energyById(Id id)
{
    switch (id) {
    case EnergyPeak::Id::FE847:
        return 847.0;
    case EnergyPeak::Id::FE1238:
        return 1238.0;
    case EnergyPeak::Id::HYDROGENADD:
        return 2100.0;
    case EnergyPeak::Id::HYDROGEN:
        return 2223.0;
    case EnergyPeak::Id::CARBON:
        return 4438.0;
    case EnergyPeak::Id::SILICON:
        return 4496.0;
    case EnergyPeak::Id::OXYGENADD:
        return 6129.0 - 511.0;
    case EnergyPeak::Id::OXYGEN:
        return 6129.0;
//    case EnergyPeak::Id::FE7631ADD:
//        return 7631.0 - 511.0;
//    case EnergyPeak::Id::FE7631:
//        return 7631.0;
    case EnergyPeak::Id::FE7631ADD:
        return 0.5 * (7631.1 + 7645.6) - 511.0;
    case EnergyPeak::Id::FE7631:
        return 0.5 * (7631.1 + 7645.6);
    }
    return 0.0;
}
