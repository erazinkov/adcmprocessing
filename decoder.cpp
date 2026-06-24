#include "decoder.h"

#include <iostream>
#include <bits/stdc++.h>

#include "progressbar.h"
#include "decoder.h"

#include <iostream>
#include <bits/stdc++.h>


Decoder::Decoder()
{
}


void Decoder::process(const std::string &filePath)
{
    ifs_.open(filePath, std::ios::in | std::ios::binary);
    if (!ifs_.is_open())
    {
        std::clog << "Can't open file" << std::endl;
        return;
    }

    ifs_.seekg(0, std::ios::end);
    u_int64_t size{static_cast<u_int32_t>(ifs_.tellg())};
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

    while (ifs_)
    {
        ifs_ >> hdr;

        if (hdr.id == STOR_ID_CMAP && hdr.size > sizeof(stor_packet_hdr_t))
        {
            currentPosition = static_cast<u_int64_t>(ifs_.tellg());
            ifs_ >> cmap;
            // currentPosition -= sizeof(stor_packet_hdr_t);
            ProgressBar<u_int64_t>::show(currentPosition, size);
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
//                    if (events_.find({g->ch, a->ch}) == events_.end()) {
//                        channels_.g.insert(g->ch);
//                        channels_.a.insert(a->ch);
//                    }
                    events_[{g->ch, a->ch}].emplace_back(dec_ev_m_t{g->t - a->t, g->a});
//                    g = nullptr;
//                    a = nullptr;


//                    std::unique_ptr<stor_puls_t> g{new stor_puls_t()};
//                    std::unique_ptr<stor_puls_t> a{new stor_puls_t()};
//                    ifs_ >> *g.get() >> *a.get();
//                    events_[{g.get()->ch, a.get()->ch}].emplace_back(dec_ev_m_t{g.get()->t - a.get()->t, g.get()->a});
//                    channels_.g.insert(g.get()->ch);
//                    channels_.a.insert(a.get()->ch);
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
            for (size_t i{0}; i < counters.n; ++i) {
                counters_[i] = counters.rawhits.at(i);
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

const std::map<std::pair<uint8_t, uint8_t>, std::vector<dec_ev_m_t>> &Decoder::events() const
{
    return events_;
}

double Decoder::time() const
{
    return time_;
}

const std::map<uint8_t, uint32_t> &Decoder::counters() const
{
    return counters_;
}

