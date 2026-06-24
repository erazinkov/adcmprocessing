QT -= gui

CONFIG += c++17 console
CONFIG -= app_bundle

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        calibration.cpp \
#        channelmap.cpp \
    consoletable.cpp \
        decoder.cpp \
        energypeak.cpp \
    energypeakfinder.cpp \
        histogrammanager.cpp \
        main.cpp \
        piecewiselinearfunction.cpp \
    resolutionprocessing.cpp \
    timepeaksfinder.cpp



INCLUDEPATH += $$system(root-config --incdir)
LIBS += $$system(root-config --libs) -lMinuit -lSpectrum

HEADERS += \
        adcm_df.h \
        calibration.h \
#        channelmap.h \
    consoletable.h \
        constants.h \
        decoder.h \
        energypeak.h \
    energypeakfinder.h \
        histogrammanager.h \
        piecewiselinearfunction.h \
        progressbar.h \
    resolutionprocessing.h \
        spinner.h \
    timepeaksfinder.h \
        utils.h

# Default rules for deployment.
        qnx: target.path = /tmp/$${TARGET}/bin
        else: unix:!android: target.path = /opt/$${TARGET}/bin
        !isEmpty(target.path): INSTALLS += target
