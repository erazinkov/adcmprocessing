//#include <QCoreApplication>

#include <iostream>
#include <limits>

#include "decoder.h"
#include "histogrammanager.h"
#include "calibration.h"
#include "histogramwriter.h"

#include "constants.h"
#include "consoletable.h"
#include "utils.h"

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

    if (decoder.events().empty() || decoder.time() < std::numeric_limits<double>::epsilon() || decoder.counters().empty()) {
        return 1;
    }

    HistogramManager histogramManager(AppConstants::MAX_GAMMA_NUMBER, AppConstants::MAX_ALPHA_NUMBER);

    Calibration calibration(filePath.filename().string(), &histogramManager);
    start = std::chrono::steady_clock::now();
    calibration.setNewData(decoder.events(), decoder.channels(), decoder.time(), decoder.counters());
    calibration.process();
    stop = std::chrono::steady_clock::now();
    HistogramWriter histogramWriter;
    histogramWriter.addHist(histogramManager.histEnergyTotal());
    const auto rootFileName{AppConstants::OUTPUT_PATH + filePath.filename().string() + ".root"};
    if (histogramWriter.write(rootFileName)) {
        std::cout << "Histograms successfully written to file " << rootFileName << std::endl;
    }
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
    return 0;
//    return a.exec();
}


