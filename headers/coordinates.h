#ifndef COORDINATES_H
#define COORDINATES_H

#include "data.h"

class Coordinates : Data {
public:
    double latitude;
    double longitude;
    double altitude;
    double course;

    Coordinates(double latitude = 0.0,
                double longitude = 0.0,
                double altitude = 0.0,
                double course = 0.0);

    Coordinates(const Coordinates &other);

    Coordinates& operator=(const Coordinates &other);

    bool updateCoordinates(double latitude,
                           double longitude,
                           double altitude);
    bool updateCourse(double course);
    void move(double distance);

    QString toString() const override;
    QJsonObject toJson() const override;
    void fromJson(const QJsonObject &json) override;

private:
    bool isLatitudeValid(double latitude);
    bool isLongitudeValid(double longitude);
    bool isCourseValid(double course);
};

#endif // COORDINATES_H
