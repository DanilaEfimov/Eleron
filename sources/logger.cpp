#include "Logger.h"
#include "globals.h"
#include "errordialog.h"

#include <QDir>
#include <QDateTime>

QFile Logger::logFile;

void Logger::log(const QString& message) {
    if (!logFile.isOpen()) {
        openLogFile();
    }
    QTextStream out(&logFile);
    out << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " - " << message << "\n";
}

void Logger::openLogFile() {
    QDir logDir;
    if (!logDir.exists(globals::logDir)) {
        logDir.mkpath(globals::logDir);
    }
    logFile.setFileName(globals::logDir + QDir::separator() + globals::logFilename);
    if (!logFile.open(QIODevice::Append | QIODevice::Text)) {
        ErrorDialog::showError("Не удалось открыть файл журнала для записи.");
    }
}

void Logger::closeLogFile() {
    if (logFile.isOpen()) {
        logFile.close();
    }
}
