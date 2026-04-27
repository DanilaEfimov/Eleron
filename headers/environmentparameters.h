#ifndef ENVIRONMENTALPARAMETERS_H
#define ENVIRONMENTALPARAMETERS_H

#include "data.h"

class EnvironmentalParameters : Data {
public:
    double windSpeed;
    double windDirection;
    double temperature;
    double humidity;

    EnvironmentalParameters(double windSpeed = 0.0,
                            double windDirection = 0.0,
                            double temperature = 25.0,
                            double humidity = 50.0);

    bool updateWeather(double windSpeed,
                       double windDirection,
                       double temperature,
                       double humidity);

    QString toString() const override;
    QJsonObject toJson() const override;
    void fromJson(const QJsonObject &json) override;

private:
    bool isWindSpeedValid(double windSpeed);
    bool isWindDirectionValid(double windDirection);
    bool isTemperatureValid(double temperature);
    bool isHumidityValid(double humidity);
};

#endif // ENVIRONMENTALPARAMETERS_H
