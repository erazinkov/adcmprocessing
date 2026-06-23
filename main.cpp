#include <QCoreApplication>

#include <iostream>
#include <filesystem>

#include "decoder.h"
#include "histogrammanager.h"
#include "calibration.h"

#include "constants.h"

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
        std::cout << filePath << " " << "Time:" << " " << decoder.time();
        auto stop = std::chrono::steady_clock::now();
        std::cout << " " << "Decoding:" << " " << std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count() << std::endl;
        if (decoder.events().empty() || qFuzzyCompare(decoder.time(), 0.0) || decoder.counters().empty()) {
            return 0;
        }
        HistogramManager histogramManager(AppConstants::MAX_GAMMA_NUMBER, AppConstants::MAX_ALPHA_NUMBER);
        Calibration calibration(filePath.filename().string(), &histogramManager);
        start = std::chrono::steady_clock::now();
        calibration.setNewData(decoder.events(), decoder.channels(), decoder.time(), decoder.counters());
        calibration.process();
        stop = std::chrono::steady_clock::now();
        std::cout << "Processing: " << std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count() << std::endl;

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
