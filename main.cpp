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

void process() {
//    const std::string fileName{"results/sector10.root"};
    const std::string fileName{"results/c12_4cm_3300g_47cm_1.root"};
    std::unique_ptr<TFile> fileIn{std::make_unique<TFile>(fileName.c_str(), "OPEN")};
    if (fileIn.get()->IsOpen()) {
        std::unique_ptr<TH1D> hist{static_cast<TH1D *>(fileIn.get()->Get("hist_amp_by_gamma_0"))};
        hist.get()->SetDirectory(nullptr);
        std::unique_ptr<TH1D> histRc{static_cast<TH1D *>(fileIn.get()->Get("hist_amp_by_gamma_rc_0"))};
        histRc.get()->SetDirectory(nullptr);


        TSpectrum *s = new TSpectrum();
        TH1 *hBg{s->Background(histRc.get(), 20, "SAME")}; // Adjust parameters
        hist.get()->Add(hBg, -1.0); // Subtract background
        Int_t peaks{s->Search(histRc.get(), 3.5, "", 0.01)};
        Double_t *peaksX = s->GetPositionX();
        Double_t *peaksY = s->GetPositionY();
//        for (int i = 0; i < peaks; i++) {
//            // Create a marker at the peak position
//            // Parameters: x, y, marker style (20 = full circle, 24 = cross, etc.)
//            TMarker *m{new TMarker(peaksX[i], peaksY[i], 20)};
//            m->SetMarkerSize(1.5);
//            m->SetMarkerColor(kRed);
//            hist.get()->GetListOfFunctions()->Add(m);
//        }


//        EnergyPeakFinder energyPeakFinder;
//        energyPeakFinder.process(hist.get(), histRc.get());
        const std::string psFileName{"output_tmp_"};
        HistogramPainter::paintHist(histRc.get(), AppConstants::OUTPUT_PATH + psFileName + ".pdf");

    } else {
        std::cout << "Can\'t open file " << fileName << std::endl;
    }

}

int main(int argc, char *argv[])
{
//     process();
//     return 0;
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
        std::cout << "No valid data in file " << filePath << std::endl;
        return 1;
    }

    HistogramManager histogramManager(AppConstants::MAX_GAMMA_NUMBER, AppConstants::MAX_ALPHA_NUMBER);

    Calibration calibration(&histogramManager);
    start = std::chrono::steady_clock::now();
    calibration.setNewData(decoder.events(), decoder.channels(), decoder.time(), decoder.counters());
    calibration.process();
    stop = std::chrono::steady_clock::now();
    HistogramWriter histogramWriter;
    histogramWriter.addHist(histogramManager.histEnergyTotal());
    histogramWriter.addHists(histogramManager.histsEnergyByGamma());
    histogramWriter.addHists(histogramManager.histsEnergyByAlpha());
//    histogramWriter.addHists(histogramManager.histsAmpByGamma());
//    histogramWriter.addHists(histogramManager.histsAmpByGammaRc());
    const std::string rootFileName{AppConstants::OUTPUT_PATH + filePath.filename().string() + ".root"};
    if (histogramWriter.write(rootFileName)) {
        std::cout << "Histograms successfully written to file " << rootFileName << std::endl;
    }
    // TODO !
//    HistogramPainter::paintHist(histogramManager.histEnergyTotal(), AppConstants::OUTPUT_PATH + filePath.filename().string() + ".ps");
//    HistogramPainter::paintHists(histogramManager.histsTimeByGammaAlpha(), AppConstants::OUTPUT_PATH + filePath.filename().string() + "_t" + ".ps");
    HistogramPainter::paintHists(histogramManager.histsAmpByGamma(), AppConstants::OUTPUT_PATH + filePath.filename().string() + "_amp_sg" + ".pdf");
    HistogramPainter::paintHists(histogramManager.histsAmpByGammaRc(), AppConstants::OUTPUT_PATH + filePath.filename().string() + "_amp_rc" + ".pdf");
//    HistogramPainter::paintHists(histogramManager.histsEnergyByGamma(), AppConstants::OUTPUT_PATH + filePath.filename().string() + "_energy" + ".ps");

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


