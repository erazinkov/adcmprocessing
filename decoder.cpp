#include "decoder.h"

#include <iostream>
#include <bits/stdc++.h>

#include "progressbar.h"
#include "decoder.h"

#include <iostream>
#include <bits/stdc++.h>
#include <algorithm>

Decoder::Decoder()
{
}

void Decoder::process(const std::string &filePath, const long long &time)
{
    ifs_.open(filePath, std::ios::in | std::ios::binary);
    if (!ifs_.is_open())
    {
        std::clog << "Can't open file" << std::endl;
        return;
    }

    ifs_.seekg(0, std::ios::end);
    u_int64_t size{static_cast<u_int64_t>(ifs_.tellg())};
    ifs_.seekg(0);

    events_.clear();

    counters_.clear();
    time_ = 0.0;

    stor_packet_hdr_t hdr;
    stor_ev_hdr_t ev;
    adcm_cmap_t cmap;
    adcm_counters_t counters;
    u_int64_t currentPosition{0};

    stor_puls_t *g{new stor_puls_t()};
    stor_puls_t *a{new stor_puls_t()};

    auto isIntegerOverflow = [](long long int prevTs, long long int nextTs,  long long int limit = 3'000'000'000){
        return std::abs(nextTs - prevTs) > limit;
    };

    struct Detector {
        uint8_t index;  // detector index
        float amp;      // detector amplitude
        float t;          // time in ns
    };

    struct Event {
        long long int ts;   // timestamp, 10 ns step
        Detector g;
        Detector a;
    };

    std::vector<Event> spillEvents;

    while (ifs_)
    {
        ifs_ >> hdr;

        if (hdr.id == STOR_ID_CMAP && hdr.size > sizeof(stor_packet_hdr_t))
        {
            currentPosition = static_cast<u_int64_t>(ifs_.tellg());
            ifs_ >> cmap;
            ProgressBar<u_int64_t>::show(currentPosition, size);
            spillEvents.clear();
            continue;
        }
        if (hdr.id == STOR_ID_EVNT && hdr.size > sizeof(stor_packet_hdr_t))
        {
            ifs_ >> ev;
            switch (ev.np)
            {
                case 2:
                {
                    ifs_ >> *g >> *a;
//                    events_[{g->ch, a->ch}].emplace_back(dec_ev_t{g->t - a->t, g->a});
                    spillEvents.push_back({
                                              ev.ts,
                                              {g->ch, g->a, g->t},
                                              {a->ch, a->a, a->t}
                                          });
                    break;
                }
                default:
                {
                    hdr.size -= sizeof(stor_packet_hdr_t);
                    hdr.size -= sizeof(stor_ev_hdr_t);
                    ifs_.ignore(hdr.size);
                    break;
                }
            }
//            hdr.size -= sizeof(stor_packet_hdr_t);
//            hdr.size -= sizeof(stor_ev_hdr_t);
//            ifs_.ignore(hdr.size);
            continue;
        }
        if (hdr.id == STOR_ID_CNTR && hdr.size > sizeof(stor_packet_hdr_t))
        {
            ifs_ >> counters;
            if (!counters.n) {
                continue;
            }
            if (spillEvents.size()) {
                for (size_t i{1}; i < spillEvents.size(); ++i) {
                    if (isIntegerOverflow(spillEvents[0].ts, spillEvents[i].ts) && spillEvents.size()) {
                        spillEvents[i].ts += UINT32_MAX;
                    }
                }
                auto tsOffset{spillEvents[0].ts};
                for (size_t i{0}; i < spillEvents.size(); ++i) {
                    spillEvents[i].ts -= tsOffset;
                }
                for (size_t i{0}; i < spillEvents.size(); ++i) {
//                    auto dateInNanoSec{time + time_ + spillEvents[i].ts * 10 - spillEvents[spillEvents.size() - 1].ts * 10};
                    auto msFromEpochUTC =
                        time +
                        time_ * 1'000 +
                        (spillEvents[i].ts * 10 - spillEvents[spillEvents.size() - 1].ts * 10) / 1'000'000
                    ;
                    spillEvents[i].ts = static_cast<long long>(msFromEpochUTC);
                }
//                std::cout << spillEvents.front().ts << " " << spillEvents.back().ts << std::endl;
                std::cout << "Time, s:" << " " << time_ << " " << "Events:" << " " << spillEvents.size() << std::endl;
                for (const auto &spillEventsItem : spillEvents) {
                    events_[{spillEventsItem.g.index, spillEventsItem.a.index}].emplace_back(
                                dec_ev_t{spillEventsItem.ts,
                                spillEventsItem.g.t - spillEventsItem.a.t,
                                spillEventsItem.g.amp
                            });
                }
//                events_.insert(events_.cend(), spillEvents.cbegin(), spillEvents.cend());
            }
            for (size_t i{0}; i < counters.n; ++i) {
                counters_[i] += counters.rawhits.at(i);
            }
            time_ += counters.time;

//            hdr.size -= sizeof(stor_packet_hdr_t);
//            ifs_.ignore(hdr.size);
            continue;
        }
        ifs_.seekg(1 - static_cast<long long>(sizeof(stor_packet_hdr_t)), std::ios_base::cur);
    }
    delete g;
    g = nullptr;
    delete a;
    a = nullptr;
    ifs_.close();
    std::cout << std::endl;
    std::set<uint8_t> first_set, second_set;

    for (const auto& [key, value] : events_) {
        first_set.insert(key.first);
        second_set.insert(key.second);
    }

    channels_.g.assign(first_set.begin(), first_set.end());
    channels_.a.assign(second_set.begin(), second_set.end());
}

const dec_ch_t &Decoder::channels() const
{
    return channels_;
}

double Decoder::time() const
{
    return time_;
}

const std::map<uint8_t, uint32_t> &Decoder::counters() const
{
    return counters_;
}

const std::unordered_map<std::pair<uint8_t, uint8_t>, std::vector<dec_ev_t>, PairHash> &Decoder::events() const
{
    return events_;
}

