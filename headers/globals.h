#ifndef GLOBALS_H
#define GLOBALS_H

#include "cmdenum.h"
#include "modulations.h"

#include <QStringList>
#include <QString>
#include <QMap>
#include <QColor>
#include <QSize>

namespace globals {

    static constexpr double EPS = std::numeric_limits<double>::epsilon() * 1e6;

    const QColor canvasBackground = QColor(200, 200, 200);
    constexpr int radius = 6;   // in px
    constexpr int checkPointNumberShift = 6;    // in px

    const QSize iconSize = {64, 64};    // in px

    const QString iconPath = ":/eleron64.ico";
    const QString lightThemeFilepath = ":/styles/light.qss";
    const QString darkThemeFilepath = ":/styles/dark.qss";
    const QString areaSourcesDir = "spaces/sources";

    const QString unresolvedPath = "*no:path:selected*";
    const QString logDir = "logs";
    const QString logFilename = "simulation.log";

    constexpr int defaultHeight = 50;

    constexpr double maxFrequencyMhz = 5.8;
    constexpr double minStableFrequencyMhz = 2.4;   // Wi-fi
    constexpr double dropChanceFactor = 1.5;
    // Topographic maps in spaces/sources share one scale:
    // the kilometer grid period on source images is ~35 px.
    // Boosted by 2.5x to match desired mission pacing.
    constexpr double mapMetersPerPixel = 1000.0 / (35.0 * 2.5);
    const QStringList validModulations = {
        QPSK, BPSK, OQPSK
    };

    inline const QMap<CmdEnum, QString> CmdNamesRu = {
        { CmdEnum::None,               "Нет команды" },
        { CmdEnum::TakePhoto,          "Сделать фото" },
        { CmdEnum::StartVideo,         "Начать видео" },
        { CmdEnum::EndVideo,           "Остановить видео" },
        { CmdEnum::ChangeAltitudeUp,   "Увеличить высоту" },
        { CmdEnum::ChangeAltitudeDown, "Уменьшить высоту" },
        { CmdEnum::Drop,               "Сброс полезной нагрузки" }
    };

    inline const QMap<CmdEnum, QColor> CmdColors = {
        { CmdEnum::TakePhoto,         QColor(255, 200,   0) },
        { CmdEnum::StartVideo,        QColor(  0, 200,  60) },
        { CmdEnum::EndVideo,          QColor(180,   0, 200) },
        { CmdEnum::ChangeAltitudeUp,  QColor(  0, 160, 255) },
        { CmdEnum::ChangeAltitudeDown,QColor(255, 120,   0) },
        { CmdEnum::Drop,              QColor(120,  30,  30) },
    };

    inline bool isValidModulation(const QString& modulation) {
        return validModulations.contains(modulation);
    }

}

#endif // GLOBALS_H
