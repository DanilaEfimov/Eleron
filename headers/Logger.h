#ifndef LOGGER_H
#define LOGGER_H

#include <QString>
#include <QFile>
#include <QTextStream>

class Logger {
public:
    static void log(const QString& message);

private:
    static QFile logFile;
    static void openLogFile();
    static void closeLogFile();
};

#endif // LOGGER_H
