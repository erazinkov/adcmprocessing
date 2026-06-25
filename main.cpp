#include <QCoreApplication>

#include <iostream>
#include <filesystem>

#include "decoder.h"
#include "histogrammanager.h"
#include "calibration.h"

#include "constants.h"
#include "consoletable.h"

bool createOutputDirectory(const std::string& path);

int main(int argc, char *argv[])
{
//    QCoreApplication a(argc, argv);
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <filepath>" << std::endl;
        return 1;
    }
    std::filesystem::path filePath = argv[1];
    if (!createOutputDirectory(AppConstants::OUTPUT_PATH)) {
        return 1;
    }
    auto start = std::chrono::steady_clock::now();
    Decoder decoder;
    std::cout << "Processing file: " << filePath << std::endl;
    decoder.process(filePath);
    auto stop = std::chrono::steady_clock::now();
    auto dT{std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count()};

    if (decoder.events_o().empty() || qFuzzyCompare(decoder.time(), 0.0) || decoder.counters().empty()) {
        return 1;
    }


    HistogramManager histogramManager(AppConstants::MAX_GAMMA_NUMBER, AppConstants::MAX_ALPHA_NUMBER);
    Calibration calibration(filePath.filename().string(), &histogramManager);
    start = std::chrono::steady_clock::now();
//    calibration.setNewData(decoder.events(), decoder.channels(), decoder.time(), decoder.counters());
    calibration.setNewData_o(decoder.events_o(), decoder.channels(), decoder.time(), decoder.counters());
    calibration.process();
    stop = std::chrono::steady_clock::now();
    auto dP{std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count()};

    auto eventsNumber{0};
    for (const auto& pair : decoder.events_o()) {
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
    return 0;
//    return a.exec();
}

bool createOutputDirectory(const std::string& path) {
    try {
        if (!std::filesystem::exists(path)) {
            if (std::filesystem::create_directories(path)) {
                std::cout << "Output directory created: " << path << std::endl;
                return true;
            } else {
                std::cerr << "Failed to create output directory: " << path << std::endl;
                return false;
            }
        }
        std::cout << "Output directory already exists: " << path << std::endl;
        return true;
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
        return false;
    }
}
