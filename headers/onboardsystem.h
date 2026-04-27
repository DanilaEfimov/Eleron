#ifndef ONBOARDSYSTEM_H
#define ONBOARDSYSTEM_H

#include "coordinates.h"
#include "data.h"

class OnboardSystem : Data {
public:
    Coordinates coordinates;
    double speed;
    double batteryLevel;
    double temperature;

    OnboardSystem(const Coordinates& coords = Coordinates(),
                  double speed = 0.0,
                  double batteryLevel = 100.0,
                  double temperature = 25.0);

    void updateSpeed(double speed);
    void updateCourse(double course);
    void updateBatteryLevel(double batteryLevel);
    void updateTemperature(double temperature);

    QString toString() const override;
    QJsonObject toJson() const override;
    void fromJson(const QJsonObject &json) override;
};

#endif // ONBOARDSYSTEM_H
