#ifndef TELEMETRY_H
#define TELEMETRY_H

#include "data.h"

#include <QString>

class Telemetry : Data {
public:
    int numSatellites;
    double communicationFrequency;
    QString modulation;

    Telemetry(int numSatellites = 12,
              double communicationFrequency = 2.4,
              const QString& modulation = "QPSK");

    bool updateTelemetry(int numSatellites,
                         double communicationFrequency,
                         const QString& modulation);

    QString toString() const override;
    QJsonObject toJson() const override;
    void fromJson(const QJsonObject &json) override;

private:
    bool isNumSatellitesValid(int numSatellites);
    bool isCommunicationFrequencyValid(double communicationFrequency);
    bool isModulationValid(const QString& modulation);
};

#endif // TELEMETRY_H
