#ifndef DECODER_H
#define DECODER_H

#include "adcm_df.h"
#include <map>

class Decoder
{
public:
    Decoder();

    void process(const std::string &filePath);

    std::vector<dec_ev_t> convert_file_mmap(const std::string& filename);

    const dec_ch_t &channels() const;

    const std::map<std::pair<uint8_t, uint8_t>, std::vector<dec_ev_m_t>> &events() const;

    double time() const;

    const std::map<uint8_t, uint32_t> &counters() const;

private:
    std::ifstream ifs_;
    std::map<std::pair<uint8_t, uint8_t>, std::vector<dec_ev_m_t>> events_;
    std::map<uint8_t, uint32_t> counters_;
    double time_;

    dec_ch_t channels_;
};





#endif // DECODER_H
