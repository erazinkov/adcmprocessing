#include "energypeak.h"

EnergyPeak::EnergyPeak(EnergyPeak::Id id, double channel, double channelErr) :
    id_(id), channel_(channel), channelErr_(channelErr)
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
    case EnergyPeak::Id::FE7631ADD:
        return 0.5 * (7631.1 + 7645.6) - 511.0;
    case EnergyPeak::Id::FE7631:
        return 0.5 * (7631.1 + 7645.6);
    }
    return 0.0;
}

double EnergyPeak::channelErr() const
{
    return channelErr_;
}

void EnergyPeak::setChannelErr(double newChannelErr)
{
    channelErr_ = newChannelErr;
}

std::string EnergyPeak::idToString(EnergyPeak::Id id)
{
    switch (id) {
        case EnergyPeak::Id::FE847:        return "FE847";
        case EnergyPeak::Id::FE1238:       return "FE1238";
        case EnergyPeak::Id::HYDROGENADD:  return "HYDROGENADD";
        case EnergyPeak::Id::HYDROGEN:     return "HYDROGEN";
        case EnergyPeak::Id::CARBON:       return "CARBON";
        case EnergyPeak::Id::SILICON:      return "SILICON";
        case EnergyPeak::Id::OXYGEN:       return "OXYGEN";
        case EnergyPeak::Id::OXYGENADD:    return "OXYGENADD";
        case EnergyPeak::Id::FE7631:       return "FE7631";
        case EnergyPeak::Id::FE7631ADD:    return "FE7631ADD";
    }
    return "UNKNOWN";
}

EnergyPeak::Id EnergyPeak::idFromString(const std::string& s)
{
    static const std::map<std::string, EnergyPeak::Id> table = {
        {"FE847",       EnergyPeak::Id::FE847},
        {"FE1238",      EnergyPeak::Id::FE1238},
        {"HYDROGENADD", EnergyPeak::Id::HYDROGENADD},
        {"HYDROGEN",    EnergyPeak::Id::HYDROGEN},
        {"CARBON",      EnergyPeak::Id::CARBON},
        {"SILICON",     EnergyPeak::Id::SILICON},
        {"OXYGEN",      EnergyPeak::Id::OXYGEN},
        {"OXYGENADD",   EnergyPeak::Id::OXYGENADD},
        {"FE7631",      EnergyPeak::Id::FE7631},
        {"FE7631ADD",   EnergyPeak::Id::FE7631ADD},
    };
    auto it = table.find(s);
    if (it == table.end())
        throw std::runtime_error("Unknown EnergyPeak::Id: '" + s + "'");
    return it->second;
}
