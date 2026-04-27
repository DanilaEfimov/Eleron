#include "onboardsystem.h"
#include "Logger.h"

OnboardSystem::OnboardSystem(const Coordinates& coords,
                             double speed,
                             double batteryLevel,
                             double temperature)
    : Data(),
    coordinates(coords),
    speed(speed),
    batteryLevel(batteryLevel),
    temperature(temperature)
{}

void OnboardSystem::updateSpeed(double speed) {
    if (speed >= 0.0) {
        this->speed = speed;
    } else {
        Logger::log("Invalid speed: " + QString::number(speed));
    }
}

void OnboardSystem::updateCourse(double course) {
    if (course >= 0.0 && course < 360.0) {
        this->coordinates.course = course;
    } else {
        Logger::log("Invalid course: " + QString::number(course));
    }
}

void OnboardSystem::updateBatteryLevel(double batteryLevel) {
    if (batteryLevel >= 0.0 && batteryLevel <= 100.0) {
        this->batteryLevel = batteryLevel;
    } else {
        Logger::log("Invalid battery level: " + QString::number(batteryLevel));
    }
}

void OnboardSystem::updateTemperature(double temperature) {
    this->temperature = temperature;
}

QString OnboardSystem::toString() const
{
    return QString("onboardSystem{coordinates=%1, speed=%2, butteryLevel=%3, temperature=%4}")
        .arg(this->coordinates.toString())
        .arg(this->speed)
        .arg(this->batteryLevel)
        .arg(this->temperature);
}

QJsonObject OnboardSystem::toJson() const
{
    QJsonObject obj;
    obj["coordinates"] = coordinates.toJson();
    obj["speed"] = speed;
    obj["batteryLevel"] = batteryLevel;
    obj["temperature"] = temperature;
    return obj;
}

void OnboardSystem::fromJson(const QJsonObject &json)
{
    coordinates.fromJson(json["coordinates"].toObject());
    speed = json["speed"].toDouble();
    batteryLevel = json["batteryLevel"].toDouble();
    temperature = json["temperature"].toDouble();
}

