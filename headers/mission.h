#ifndef MISSION_H
#define MISSION_H

#include "way.h"
#include "onboardsystem.h"
#include "environmentparameters.h"
#include "telemetry.h"
#include "data.h"

class Mission : Data
{
public:
    Mission();
    Mission(const Way &path,
            const OnboardSystem &board,
            const EnvironmentalParameters &env,
            const Telemetry &telemetry);

    Way path;
    OnboardSystem board;
    EnvironmentalParameters env;
    Telemetry telemetry;

    QString toString() const override;
    QJsonObject toJson() const override;
    void fromJson(const QJsonObject &json) override;

    bool saveToFile(const QString &filepath) const;
    bool loadFromFile(const QString &filepath);
};

#endif // MISSION_H
