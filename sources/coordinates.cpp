#include "coordinates.h"
#include "errordialog.h"

#include <cmath>

Coordinates::Coordinates(double latitude,
                         double longitude,
                         double altitude,
                         double course)
    : Data(),
    latitude(latitude),
    longitude(longitude),
    altitude(altitude),
    course(course)
{}

Coordinates::Coordinates(const Coordinates &other)
    : Data(),
    latitude(other.latitude),
    longitude(other.longitude),
    altitude(other.altitude),
    course(other.course)
{}

Coordinates& Coordinates::operator=(const Coordinates &other)
{
    if (this != &other) {
        latitude = other.latitude;
        longitude = other.longitude;
        altitude = other.altitude;
        course = other.course;
    }
    return *this;
}

bool Coordinates::updateCoordinates(double latitude,
                                    double longitude,
                                    double altitude)
{
    if (!isLatitudeValid(latitude)) {
        ErrorDialog::showError("Некорректное значение широты.");
        return false;
    }
    if (!isLongitudeValid(longitude)) {
        ErrorDialog::showError("Некорректное значение долготы.");
        return false;
    }

    this->latitude = latitude;
    this->longitude = longitude;
    this->altitude = altitude;
    return true;
}

bool Coordinates::updateCourse(double course) {
    if (!isCourseValid(course)) {
        ErrorDialog::showError("Некорректное значение курса.");
        return false;
    }
    this->course = course;
    return true;
}

void Coordinates::move(double distance) {
    double deltaLat = distance * cos(course * M_PI / 180.0);
    double deltaLon = distance * sin(course * M_PI / 180.0);

    latitude += deltaLat;
    longitude += deltaLon;
}

QString Coordinates::toString() const
{
    return QString("Coordinates{latitude=%1, longitude=%2, course=%3}")
        .arg(this->latitude)
        .arg(this->longitude)
        .arg(this->course);
}

QJsonObject Coordinates::toJson() const
{
    QJsonObject obj;
    obj["latitude"] = latitude;
    obj["longitude"] = longitude;
    obj["altitude"] = altitude;
    obj["course"] = course;
    return obj;
}

void Coordinates::fromJson(const QJsonObject &json)
{
    latitude = json["latitude"].toDouble();
    longitude = json["longitude"].toDouble();
    altitude = json["altitude"].toDouble();
    course = json["course"].toDouble();
}

bool Coordinates::isLatitudeValid(double latitude) {
    return latitude >= -90.0 && latitude <= 90.0;
}

bool Coordinates::isLongitudeValid(double longitude) {
    return longitude >= -180.0 && longitude <= 180.0;
}

bool Coordinates::isCourseValid(double course) {
    return course >= 0.0 && course < 360.0;
}
