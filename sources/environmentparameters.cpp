#include "environmentparameters.h"
#include "Logger.h"

EnvironmentalParameters::EnvironmentalParameters(double windSpeed,
                                                 double windDirection,
                                                 double temperature,
                                                 double humidity)
    : Data(),
    windSpeed(windSpeed),
    windDirection(windDirection),
    temperature(temperature),
    humidity(humidity)
{}

bool EnvironmentalParameters::updateWeather(double windSpeed,
                                            double windDirection,
                                            double temperature,
                                            double humidity)
{
    if (!isWindSpeedValid(windSpeed)) {
        Logger::log("Invalid wind speed: " + QString::number(windSpeed));
        return false;
    }
    if (!isWindDirectionValid(windDirection)) {
        Logger::log("Invalid wind direction: " + QString::number(windDirection));
        return false;
    }
    if (!isTemperatureValid(temperature)) {
        Logger::log("Invalid temperature: " + QString::number(temperature));
        return false;
    }
    if (!isHumidityValid(humidity)) {
        Logger::log("Invalid humidity: " + QString::number(humidity));
        return false;
    }

    this->windSpeed = windSpeed;
    this->windDirection = windDirection;
    this->temperature = temperature;
    this->humidity = humidity;
    return true;
}

QString EnvironmentalParameters::toString() const
{
    return QString("EnvironmentalParameters={windSpeed=%1, windDirection=%2, temperature=%3, humidity=%4}")
        .arg(this->windSpeed)
        .arg(this->windDirection)
        .arg(this->temperature)
        .arg(this->humidity);
}

QJsonObject EnvironmentalParameters::toJson() const
{
    QJsonObject obj;
    obj["windSpeed"] = windSpeed;
    obj["windDirection"] = windDirection;
    obj["temperature"] = temperature;
    obj["humidity"] = humidity;
    return obj;
}

void EnvironmentalParameters::fromJson(const QJsonObject &json)
{
    windSpeed = json["windSpeed"].toDouble();
    windDirection = json["windDirection"].toDouble();
    temperature = json["temperature"].toDouble();
    humidity = json["humidity"].toDouble();
}

bool EnvironmentalParameters::isWindSpeedValid(double windSpeed)
{
    return windSpeed >= 0.0;
}

bool EnvironmentalParameters::isWindDirectionValid(double windDirection)
{
    return windDirection >= 0.0 && windDirection < 360.0;
}

bool EnvironmentalParameters::isTemperatureValid(double temperature)
{
    return temperature >= -100.0 && temperature <= 100.0;
}

bool EnvironmentalParameters::isHumidityValid(double humidity)
{
    return humidity >= 0.0 && humidity <= 100.0;
}
