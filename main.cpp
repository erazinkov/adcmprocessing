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

    const std::string internalEnergyPeaksFileName{AppConstants::OUTPUT_PATH + filePath.filename().string() + "_ep" + ".root"};
//    const std::string externalEnergyPeaksFileName{"results/proba_c_7_1_ep.root"};
    const std::string externalEnergyPeaksFileName{"results/pulp_rot_sop1_3_ep.root"};
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
    return 0;
//    return a.exec();
}

//int main(int argc, char *argv[])
//{
////     process();
////     return 0;
////    QCoreApplication a(argc, argv);
////    if (argc != 2) {
////        std::cerr << "Usage: " << argv[0] << " <filepath>" << std::endl;
////        return 1;
////    }
////    std::filesystem::path filePath = argv[1];

//    struct File {
//        std::string filePath;
//        std::string externalEnergyPeaksFileName;
//    };

//    const std::string path{"/home/egor/shares/coal_stroy/B7-2"};

//    std::vector<File> files {
////        { path + "/sep02-11.44.12", "results/sep02-11.44.12_ep.root" },
////        { path + "/sep03-11.32.04", "results/sep03-11.32.04_ep.root" },
////        { path + "/sep04-11.51.00", "results/sep04-11.51.00_ep.root" },
////        { path + "/sep05-12.08.15", "results/sep05-12.08.15_ep.root" },
////        { path + "/sep06-12.05.44", "results/sep06-12.05.44_ep.root" },
////        { path + "/sep07-11.46.00", "results/sep07-11.46.00_ep.root" },
////        { path + "/sep08-11.42.16", "results/sep08-11.42.16_ep.root" },
////        { path + "/sep09-11.42.19", "results/sep09-11.42.19_ep.root" },
////        { path + "/sep10-12.00.57", "results/sep10-12.00.57_ep.root" },
////        { path + "/sep11-11.50.45", "results/sep11-11.50.45_ep.root" },
////        { path + "/sep13-11.58.12", "results/sep13-11.58.12_ep.root" },
////        { path + "/sep14-11.50.03", "results/sep14-11.50.03_ep.root" },
////        { path + "/sep15-13.18.48", "results/sep15-13.18.48_ep.root" },
////        { path + "/sep16-11.54.11", "results/sep16-11.54.11_ep.root" },
////        { path + "/sep17-11.49.26", "results/sep17-11.49.26_ep.root" },
////        { path + "/sep18-11.46.17", "results/sep18-11.46.17_ep.root" },
////        { path + "/sep19-12.15.42", "results/sep19-12.15.42_ep.root" },
////        { path + "/sep20-12.03.52", "results/sep20-12.03.52_ep.root" },
////        { path + "/sep21-11.51.00", "results/sep21-11.51.00_ep.root" },
////        { path + "/sep22-11.52.04", "results/sep22-11.52.04_ep.root" },
////        { path + "/sep23-11.39.16", "results/sep23-11.39.16_ep.root" },
////        { path + "/sep24-12.08.49", "results/sep24-12.08.49_ep.root" },
////        { path + "/sep25-11.42.45", "results/sep25-11.42.45_ep.root" },
////        { path + "/oct01-08.02.08", "results/oct01-08.02.08_ep.root" },
////        { path + "/oct02-07.58.41", "results/oct02-07.58.41_ep.root" },
////        { path + "/oct04-20.08.05", "results/oct04-20.08.05_ep.root" },
////        { path + "/oct05-08.04.42", "results/oct05-08.04.42_ep.root" },
////        { path + "/oct06-09.19.40", "results/oct06-09.19.40_ep.root" },
////        { path + "/oct09-08.09.13", "results/oct09-08.09.13_ep.root" },
////        { path + "/oct10-20.05.11", "results/oct10-20.05.11_ep.root" },
////        { path + "/oct13-01.22.59", "results/oct13-01.22.59_ep.root" },

//        { path + "/nov23-17.18.56", "results/nov23-17.18.56_ep.root" },
//        { path + "/nov24-09.21.31", "results/nov24-09.21.31_ep.root" },
//        { path + "/nov26-14.15.39", "results/nov26-14.15.39_ep.root" },
//        { path + "/nov27-12.27.22", "results/nov27-12.27.22_ep.root" },

//    };

//    if (!createOutputDirectory(AppConstants::OUTPUT_PATH)) {
//        return 1;
//    }

//    for (const auto &file : files) {
//        std::filesystem::path filePath = file.filePath;

//        auto start = std::chrono::steady_clock::now();
//        Decoder decoder;
//        std::cout << "Processing file: " << filePath << std::endl;
//        decoder.process(filePath);
//        auto stop = std::chrono::steady_clock::now();
//        auto dT{std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count()};

//        if (decoder.events().empty() || decoder.time() < std::numeric_limits<double>::epsilon() || decoder.counters().empty()) {
//            std::cout << "No valid data in file " << filePath << std::endl;
//            return 1;
//        }

//        HistogramManager histogramManager(AppConstants::MAX_GAMMA_NUMBER, AppConstants::MAX_ALPHA_NUMBER);

//        Calibration calibration(&histogramManager);
//        start = std::chrono::steady_clock::now();
//        calibration.setNewData(decoder.events(), decoder.channels(), decoder.time(), decoder.counters());

//        const std::string internalEnergyPeaksFileName{AppConstants::OUTPUT_PATH + filePath.filename().string() + "_ep" + ".root"};
//    //    const std::string externalEnergyPeaksFileName{"results/proba_c_7_1_ep.root"};
//        const std::string externalEnergyPeaksFileName{file.externalEnergyPeaksFileName};
//        calibration.process(internalEnergyPeaksFileName, externalEnergyPeaksFileName);
////        calibration.process(internalEnergyPeaksFileName);
//        stop = std::chrono::steady_clock::now();
//        HistogramWriter histogramWriter;
//        histogramWriter.addHist(histogramManager.histEnergyTotal());
//        histogramWriter.addHists(histogramManager.histsEnergyByGamma());
//    //    histogramWriter.addHists(histogramManager.histsEnergyByAlpha());
//    //    histogramWriter.addHists(histogramManager.histsAmpByGamma());
//    //    histogramWriter.addHists(histogramManager.histsAmpByGammaRc());
//        const std::string rootFileName{AppConstants::OUTPUT_PATH + filePath.filename().string() + ".root"};
//        if (histogramWriter.write(rootFileName)) {
//            std::cout << "Histograms successfully written to file " << rootFileName << std::endl;
//        }
//        // TODO !
//    //    HistogramPainter::paintHist(histogramManager.histEnergyTotal(), AppConstants::OUTPUT_PATH + filePath.filename().string() + ".pdf");
//        HistogramPainter::paintHists(histogramManager.histsTimeByGammaAlpha(), AppConstants::OUTPUT_PATH + filePath.filename().string() + "_t" + ".pdf");
//        HistogramPainter::paintHists(histogramManager.histsAmpByGamma(), AppConstants::OUTPUT_PATH + filePath.filename().string() + "_amp_sg" + ".pdf");
//        HistogramPainter::paintHists(histogramManager.histsAmpByGammaRc(), AppConstants::OUTPUT_PATH + filePath.filename().string() + "_amp_rc" + ".pdf");
//        HistogramPainter::paintHists(histogramManager.histsEnergyByGamma(), AppConstants::OUTPUT_PATH + filePath.filename().string() + "_energy_sg" + ".pdf");
//         HistogramPainter::paintHists(histogramManager.histsEnergyByGammaRc(), AppConstants::OUTPUT_PATH + filePath.filename().string() + "_energy_rc" + ".pdf");

//        auto dP{std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count()};

//        std::ofstream ofs("counters.txt", std::ios::out | std::ios::app);
//        if (ofs.is_open()) {
//            ofs << filePath;
//            for (const auto &[key, value] : decoder.counters()) {
//                ofs << " " << value;
//            }
//            ofs << std::endl;
//            ofs.close();
//        }

//        auto eventsNumber{0};
//        for (const auto& pair : decoder.events()) {
//            eventsNumber += pair.second.size();
//        }

//        std::vector<std::vector<std::string>> pData = {
//                {std::string(BOLD) + "parameter" + RESET, std::string(BOLD) + "value" + RESET, std::string(BOLD) + "dimension" + RESET},
//                {"decodeTime", std::to_string(dT), std::string(YELLOW) + "ms" + RESET},
//                {"processTime", std::to_string(dP), std::string(YELLOW) + "ms" + RESET},
//                {"eventsNumber", std::to_string(eventsNumber), std::string(YELLOW) + "" + RESET},
//                {"measTime", std::to_string(decoder.time()), std::string(YELLOW) + "s" + RESET},
//                {"gammaNumber", std::to_string(decoder.channels().g.size()), std::string(YELLOW) + "" + RESET},
//                {"alphaNumber", std::to_string(decoder.channels().a.size()), std::string(YELLOW) + "" + RESET},
//            };
//        ConsoleTable pTable(pData);
//        pTable.show();
//    }
//    return 0;
////    return a.exec();
//}

