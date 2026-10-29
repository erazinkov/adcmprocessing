//#include <QCoreApplication>

#include <iostream>
#include <limits>

#include "decoder.h"
#include "histogrammanager.h"
#include "calibration.h"
#include "histogramwriter.h"
#include "histogrampainter.h"

#include "constants.h"
#include "consoletable.h"
#include "utils.h"

#include <TFile.h>
#include <TSpectrum.h>
#include <TMarker.h>

struct ProcessingData {
    int rowIdx;
    int colIdx;
    std::vector<long long int> ts;
};

std::vector<ProcessingData> readProcessingData(const std::string& fileName) {
    std::vector<ProcessingData> processingData;

    std::ifstream ifs(fileName, std::ios::in);
    if (!ifs.is_open()) {
        std::cout << "Can\'t read " << fileName << std::endl;
        return processingData;
    }

    std::string line;

    while (std::getline(ifs, line)) {
        if (line.empty()) {
            continue;
        }

        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        std::istringstream ss(line);
        std::string str;

        ProcessingData pD;

        // rowIdx
        if (!std::getline(ss, str, ',')) {
            continue;
        }
        pD.rowIdx = std::stoi(str);

        // colIdx
        if (!std::getline(ss, str, ',')) {
            continue;
        }
        pD.colIdx = std::stoi(str);

        std::string tsField;
        std::getline(ss, tsField);

        if (!tsField.empty()) {
            std::istringstream tsStream(tsField);
            std::string ts;
            while (std::getline(tsStream, ts, ';')) {
                if (!ts.empty()) {
                    std::istringstream iss(ts);
                    long long int ts;
                    if (iss >> ts) {
                        pD.ts.push_back(ts * 1'000);
                    }
                }

            }
        }
        std::sort(pD.ts.begin(), pD.ts.end());
        processingData.push_back(pD);
    }

    ifs.close();
    return processingData;
}

int process(const std::string &file, const std::pair<std::string, long long int> &daqDataItem) {
//    if (argc != 2) {
//        std::cerr << "Usage: " << argv[0] << " <filepath>" << std::endl;
//        return 1;
//    }
//    std::filesystem::path filePath = argv[1];
//    if (!createOutputDirectory(AppConstants::OUTPUT_PATH)) {
//        return 1;
//    }

    if (!createOutputDirectory(AppConstants::OUTPUT_PATH)) {
        return 1;
    }
    auto start = std::chrono::steady_clock::now();
    std::filesystem::path filePath = daqDataItem.first;
    auto time = daqDataItem.second;
    Decoder decoder;
    std::cout << "Processing file: " << filePath << std::endl;
    decoder.process(filePath, time);
    auto stop = std::chrono::steady_clock::now();
    auto dT{std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count()};

    if (decoder.events().empty() || decoder.time() < std::numeric_limits<double>::epsilon() || decoder.counters().empty()) {
        std::cout << "No valid data in file " << filePath << std::endl;
        return 1;
    }

//    std::pair<uint8_t, uint8_t> p{decoder.channels().g.at(0), decoder.channels().a.at(0)};
//    std::cout << decoder.events().at(p).front().ts << " " <<  decoder.events().at(p).back().ts << std::endl;
    return 0;

    HistogramManager histogramManager(AppConstants::MAX_GAMMA_NUMBER, AppConstants::MAX_ALPHA_NUMBER);

    Calibration calibration(&histogramManager);
    start = std::chrono::steady_clock::now();
    calibration.setNewData(decoder.events(), decoder.channels(), decoder.time(), decoder.counters());

    const std::string internalEnergyPeaksFileName{AppConstants::OUTPUT_PATH + filePath.filename().string() + "_ep" + ".root"};
//    const std::string externalEnergyPeaksFileName{"results/proba_c_7_1_ep.root"};
    const std::string externalEnergyPeaksFileName{"results/proba_c_0_1_ep.root"};
    calibration.process(internalEnergyPeaksFileName);
//    calibration.process(internalEnergyPeaksFileName);
    stop = std::chrono::steady_clock::now();
    HistogramWriter histogramWriter;
    histogramWriter.addHist(histogramManager.histEnergyTotal());
    histogramWriter.addHists(histogramManager.histsEnergyByGamma());
//    histogramWriter.addHists(histogramManager.histsEnergyByAlpha());
//    histogramWriter.addHists(histogramManager.histsAmpByGamma());
//    histogramWriter.addHists(histogramManager.histsAmpByGammaRc());
    const std::string rootFileName{AppConstants::OUTPUT_PATH + filePath.filename().string() + ".root"};
    if (histogramWriter.write(rootFileName)) {
        std::cout << "Histograms successfully written to file " << rootFileName << std::endl;
    }
    // TODO !
//    HistogramPainter::paintHist(histogramManager.histEnergyTotal(), AppConstants::OUTPUT_PATH + filePath.filename().string() + ".pdf");
    HistogramPainter::paintHists(histogramManager.histsTimeByGammaAlpha(), AppConstants::OUTPUT_PATH + filePath.filename().string() + "_t" + ".pdf");
    HistogramPainter::paintHists(histogramManager.histsAmpByGamma(), AppConstants::OUTPUT_PATH + filePath.filename().string() + "_amp_sg" + ".pdf");
    HistogramPainter::paintHists(histogramManager.histsAmpByGammaRc(), AppConstants::OUTPUT_PATH + filePath.filename().string() + "_amp_rc" + ".pdf");
    HistogramPainter::paintHists(histogramManager.histsEnergyByGamma(), AppConstants::OUTPUT_PATH + filePath.filename().string() + "_energy_sg" + ".pdf");
     HistogramPainter::paintHists(histogramManager.histsEnergyByGammaRc(), AppConstants::OUTPUT_PATH + filePath.filename().string() + "_energy_rc" + ".pdf");

    auto dP{std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count()};

    std::ofstream ofs("counters.txt", std::ios::out | std::ios::app);
    if (ofs.is_open()) {
        ofs << filePath;
        for (const auto &[key, value] : decoder.counters()) {
            ofs << " " << value;
        }
        ofs << std::endl;
        ofs.close();
    }

    auto eventsNumber{0};
    for (const auto& pair : decoder.events()) {
        eventsNumber += pair.second.size();
    }

    std::vector<std::vector<std::string>> pData = {
            {std::string(BOLD) + "parameter" + RESET, std::string(BOLD) + "value" + RESET, std::string(BOLD) + "dimension" + RESET},
            {"decodeTime", std::to_string(dT), std::string(YELLOW) + "ms" + RESET},
            {"processTime", std::to_string(dP), std::string(YELLOW) + "ms" + RESET},
            {"eventsNumber", std::to_string(eventsNumber), std::string(YELLOW) + "" + RESET},
            {"measTime", std::to_string(decoder.time()), std::string(YELLOW) + "s" + RESET},
            {"gammaNumber", std::to_string(decoder.channels().g.size()), std::string(YELLOW) + "" + RESET},
            {"alphaNumber", std::to_string(decoder.channels().a.size()), std::string(YELLOW) + "" + RESET},
        };
    ConsoleTable pTable(pData);
    pTable.show();


}

int main(int argc, char *argv[])
{

    const std::vector<std::pair<std::string, long long int>> daqData{
        { "sector1", stringToMcsFromEpochUTC("2025-06-17 10:27:54") },
//        { "sector2", stringToMsFromEpochUTC("2025-06-17 10:56:08") },
//        { "sector3", stringToMsFromEpochUTC("2025-06-17 13:34:41") },
//        { "sector4", stringToMsFromEpochUTC("2025-06-18 15:33:10") },
//        { "sector5", stringToMsFromEpochUTC("2025-06-18 15:49:44") },
//        { "sector6", stringToMsFromEpochUTC("2025-06-18 16:06:24") },
//        { "sector7", stringToMsFromEpochUTC("2025-06-18 16:38:34") },
//        { "sector8", stringToMsFromEpochUTC("2025-06-18 17:03:21") },
//        { "sector9", stringToMsFromEpochUTC("2025-06-18 17:19:42") },
//        { "sector10", stringToMsFromEpochUTC("2025-06-18 19:03:37") }
    };

    const std::string processingFile{
        "processing_data.csv"
    };

    std::vector<ProcessingData> sectorsData{readProcessingData(processingFile)};
//    for (const auto &sectorsDataItem : sectorsData) {
//        std::cout << sectorsDataItem.rowIdx << " " << sectorsDataItem.colIdx << " " << sectorsDataItem.ts.size() << std::endl;
//    }
//    return 0;

    for (const auto &sectorsDataItem : sectorsData) {
        std::unordered_map<std::pair<uint8_t, uint8_t>,
                               std::vector<dec_ev_t>, PairHash> sectorEvents;
        std::unordered_map<std::pair<uint8_t, uint8_t>,
                               std::vector<dec_ev_t>, PairHash> sectorEventsFiltered;
        std::map<uint8_t, uint32_t> sectorCounters;
        double sectorTime{0.0};
        dec_ch_t sectorChannels;

        auto start = std::chrono::steady_clock::now();
        for (const auto &daqDataItem : daqData) {
            std::filesystem::path filePath = daqDataItem.first;
            auto time = daqDataItem.second;
            std::cout << "Processing file: " << filePath << std::endl;
            std::cout << "Daq start time/Sector min data time/Sector max data time " << time << "/" << sectorsDataItem.ts.front()<< "/" << sectorsDataItem.ts.back() << " " << "ms"<< std::endl;
            if (time > sectorsDataItem.ts.back()) {
                std::cout << "Skip daq file " << daqDataItem.first << ".\n"
                          << "Daq start time: " <<  time << ".\n"
                          << "Sector data time: " << sectorsDataItem.ts.back() << std::endl;
                continue;
            }
            Decoder decoder;
            decoder.process(filePath, time);
            if (decoder.events().empty() || decoder.time() < std::numeric_limits<double>::epsilon() || decoder.counters().empty()) {
                std::cout << "No valid data in file " << filePath << std::endl;
                continue;
            }

            for (const auto& [key, vec] : decoder.events()) {
                auto [it, inserted] = sectorEvents.try_emplace(key, vec);
                if (!inserted) {
                    it->second.insert(it->second.end(), vec.begin(), vec.end());
                }
            }
            long long int e1{0};
            for (const auto &[key, value] : decoder.events()) {
    //            std::cout << key.first << "_" << key.second << " " << value.size() << std::endl;
                e1 += value.size();
            }
            std::cout << "Sector unfiltered decoder.events()" << " " << sectorsDataItem.rowIdx << "_" << sectorsDataItem.colIdx
                      << " " << "number" << " " << e1 << std::endl;
            for (const auto& [key, value] : decoder.counters()) {
                auto [it, inserted] = sectorCounters.try_emplace(key, value);
                if (!inserted) {
                    it->second = value;
                }
            }
            sectorChannels = decoder.channels();
            sectorTime += decoder.time();
            long long int e{0};
            for (const auto &[key, value] : sectorEvents) {
    //            std::cout << key.first << "_" << key.second << " " << value.size() << std::endl;
                e += value.size();
            }
            std::cout << "Sector unfiltered events" << " " << sectorsDataItem.rowIdx << "_" << sectorsDataItem.colIdx
                      << " " << "number" << " " << e << std::endl;
            std::cout << "Sector" << " " << sectorsDataItem.rowIdx << "_" << sectorsDataItem.colIdx
                      << " " << "time" << " " << sectorTime << std::endl;
            for (const auto &sectorsDataItemTsItem : sectorsDataItem.ts) {
                long long int mcs{500'000};
                long long int tsMin{sectorsDataItemTsItem - mcs};
                long long int tsMax{sectorsDataItemTsItem + mcs};
                for (auto it = sectorEvents.begin(); it != sectorEvents.end(); ) {
                    auto& vec = it->second;
                    // Assumes vec is sorted ascending by ts. // 22380506 // 22117186 // 263320

                    // First element with ts > tsMin   (open lower bound)
                    auto first = std::upper_bound(
                        vec.begin(), vec.end(), tsMin,
                        [](long long v, const dec_ev_t& e) { return v < e.ts; });

                    // First element with ts >= tsMax  (open upper bound)
                    auto last = std::lower_bound(
                        first, vec.end(), tsMax,
                        [](const dec_ev_t& e, long long v) { return e.ts < v; });

                    if (first != last) {
                        auto& dst = sectorEventsFiltered[it->first];
                        dst.insert(dst.end(),
                                   std::make_move_iterator(first),
                                   std::make_move_iterator(last));

                        // Erase the [first, last) range from the source
                        vec.erase(first, last);
                    }

                    if (vec.empty()) {
                        it = sectorEvents.erase(it);
                    } else {
                        ++it;
                    }
                }
            }
        }
        long long int eF{0};
        for (const auto &[key, value] : sectorEventsFiltered) {
//            std::cout << key.first << "_" << key.second << " " << value.size() << std::endl;
            eF += value.size();
        }
        std::cout << "Sector" << " " << std::to_string(sectorsDataItem.rowIdx) << "_" << std::to_string(sectorsDataItem.colIdx)
                  << " " << "events number" << " " << eF << std::endl;
//        if (e < 250'000) {
//            continue;
//        }
        if (sectorChannels.g.empty() || sectorChannels.a.empty()) {
            std::cout << "Sector" << " " << std::to_string(sectorsDataItem.rowIdx) << "_" << std::to_string(sectorsDataItem.colIdx) << " " << "skipped"  << std::endl;
            continue;
        }
//        return 0; // !!!
//        std::pair<uint8_t, uint8_t> p{sectorChannels.g.at(0), sectorChannels.a.at(0)}; // 540497
//        if (sectorEventsFiltered.at(p).size() < 250'000) {
//            std::cout << "Sector" << " " << sectorEventsFiltered.at(p).size() << " < 250\'000"  << std::endl;
//            continue;
//        }
//        std::cout << "Sector" << " " << sectorEventsFiltered.at(p).size() << std::endl;
        auto stop = std::chrono::steady_clock::now();
        auto dT{std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count()};

        std::cout << "decodeTime" << " " <<  std::to_string(dT) << " " << "ms"
                  << " " << "sectorTime" << " " << sectorTime << " " << "s"
                  << std::endl;
        HistogramManager histogramManager(AppConstants::MAX_GAMMA_NUMBER, AppConstants::MAX_ALPHA_NUMBER);

        Calibration calibration(&histogramManager);
        start = std::chrono::steady_clock::now();
        calibration.setNewData(sectorEventsFiltered, sectorChannels, sectorTime, sectorCounters);
        const std::string fileName = std::to_string(sectorsDataItem.rowIdx) + "_" + std::to_string(sectorsDataItem.colIdx);
        const std::string internalEnergyPeaksFileName{AppConstants::OUTPUT_PATH + fileName + "_ep" + ".root"};
    //    const std::string externalEnergyPeaksFileName{"results/proba_c_7_1_ep.root"};
        const std::string externalEnergyPeaksFileName{"results/proba_c_0_1_ep.root"};
        calibration.process(internalEnergyPeaksFileName);
    //    calibration.process(internalEnergyPeaksFileName);
        stop = std::chrono::steady_clock::now();
        HistogramWriter histogramWriter;
        histogramWriter.addHist(histogramManager.histEnergyTotal());
        histogramWriter.addHists(histogramManager.histsEnergyByGamma());
    //    histogramWriter.addHists(histogramManager.histsEnergyByAlpha());
    //    histogramWriter.addHists(histogramManager.histsAmpByGamma());
    //    histogramWriter.addHists(histogramManager.histsAmpByGammaRc());

        const std::string rootFileName{AppConstants::OUTPUT_PATH + fileName + ".root"};
        if (histogramWriter.write(rootFileName)) {
            std::cout << "Histograms successfully written to file " << rootFileName << std::endl;
        }
        // TODO !
    //    HistogramPainter::paintHist(histogramManager.histEnergyTotal(), AppConstants::OUTPUT_PATH + filePath.filename().string() + ".pdf");
        HistogramPainter::paintHists(histogramManager.histsTimeByGammaAlpha(), AppConstants::OUTPUT_PATH + fileName + "_t" + ".pdf");
        HistogramPainter::paintHists(histogramManager.histsAmpByGamma(), AppConstants::OUTPUT_PATH + fileName + "_amp_sg" + ".pdf");
        HistogramPainter::paintHists(histogramManager.histsAmpByGammaRc(), AppConstants::OUTPUT_PATH + fileName + "_amp_rc" + ".pdf");
        HistogramPainter::paintHists(histogramManager.histsEnergyByGamma(), AppConstants::OUTPUT_PATH + fileName + "_energy_sg" + ".pdf");
        HistogramPainter::paintHists(histogramManager.histsEnergyByGammaRc(), AppConstants::OUTPUT_PATH + fileName + "_energy_rc" + ".pdf");

        auto dP{std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count()};
    }


    return 0;
//    QCoreApplication a(argc, argv);
//    return a.exec();
}


