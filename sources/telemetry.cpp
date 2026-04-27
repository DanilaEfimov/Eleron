#include "telemetry.h"
#include "Logger.h"
#include "globals.h"

Telemetry::Telemetry(int numSatellites,
                     double communicationFrequency,
                     const QString& modulation)
    : Data(),
    numSatellites(numSatellites),
    communicationFrequency(communicationFrequency),
    modulation(modulation)
{}

bool Telemetry::updateTelemetry(int numSatellites,
                                double communicationFrequency,
                                const QString& modulation)
{
    if (!isNumSatellitesValid(numSatellites)) {
        Logger::log("Invalid number of satellites: " + QString::number(numSatellites));
        return false;
    }
    if (!isCommunicationFrequencyValid(communicationFrequency)) {
        Logger::log("Invalid communication frequency: " + QString::number(communicationFrequency));
        return false;
    }
    if (!isModulationValid(modulation)) {
        Logger::log("Invalid modulation: " + modulation);
        return false;
    }

    this->numSatellites = numSatellites;
    this->communicationFrequency = communicationFrequency;
    this->modulation = modulation;
    return true;
}

QString Telemetry::toString() const
{
    return QString("Telemetry{numSatellites=%1, communicationFrequency=%2, modulation=%3}")
        .arg(this->numSatellites)
        .arg(this->communicationFrequency)
        .arg(this->modulation);
}

QJsonObject Telemetry::toJson() const
{
    QJsonObject obj;
    obj["numSatellites"] = numSatellites;
    obj["communicationFrequency"] = communicationFrequency;
    obj["modulation"] = modulation;
    return obj;
}

void Telemetry::fromJson(const QJsonObject &json)
{
    numSatellites = json["numSatellites"].toInt();
    communicationFrequency = json["communicationFrequency"].toDouble();
    modulation = json["modulation"].toString();
}

bool Telemetry::isNumSatellitesValid(int numSatellites) {
    return numSatellites >= 1 && numSatellites <= 50;
}

bool Telemetry::isCommunicationFrequencyValid(double communicationFrequency) {
    return communicationFrequency >= 1.0 && communicationFrequency <= 10.0;
}

bool Telemetry::isModulationValid(const QString& modulation) {
    return globals::isValidModulation(modulation);
}
