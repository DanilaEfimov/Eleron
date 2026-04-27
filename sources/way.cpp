#include "way.h"

#include <stdexcept>
#include <QJsonArray>

Way::Way(const QVector<CheckPoint> &points)
    : Data(),
    checkPoints(points)
{}

QVector<CheckPoint> Way::getCheckPoints() const {
    return checkPoints;
}

CheckPoint Way::getCheckPoint(int index) const {
    if (index < 0 || index >= checkPoints.size()) {
        throw std::out_of_range("Index out of range");
    }
    return checkPoints.at(index);
}

CheckPoint& Way::getCheckPointRef(int index)
{
    if (index < 0 || index >= checkPoints.size()) {
        throw std::out_of_range("Index out of range");
    }
    return checkPoints[index];
}

void Way::addCheckPoint(const CheckPoint &point) {
    checkPoints.append(point);
}

void Way::updateCheckPoint(int index, const CheckPoint &point) {
    if (index < 0 || index >= checkPoints.size()) {
        throw std::out_of_range("Index out of range");
    }
    checkPoints[index] = point;
}

void Way::removeCheckPoint(int index) {
    if (index < 0 || index >= checkPoints.size()) {
        throw std::out_of_range("Index out of range");
    }
    checkPoints.removeAt(index);
}

void Way::removeLast()
{
    this->checkPoints.removeLast();
}

void Way::clear()
{
    this->checkPoints.clear();
}

int Way::size() const {
    return checkPoints.size();
}

bool Way::empty() const
{
    return this->checkPoints.isEmpty();
}

QString Way::toString() const
{
    QString res = "Way=";
    res += "{";
    res += "checkPoints=";

    res += "[";
    for(const auto& point : this->checkPoints){
        res += QString("%1").arg(point.toString());
    }
    res += "]";

    res += "}";

    return res;
}

QJsonObject Way::toJson() const
{
    QJsonObject obj;
    QJsonArray pointsArray;
    for (const auto& point : checkPoints) {
        pointsArray.append(point.toJson());
    }
    obj["checkPoints"] = pointsArray;
    obj["size"] = size();
    obj["empty"] = empty();
    return obj;
}

void Way::fromJson(const QJsonObject &json)
{
    checkPoints.clear();
    QJsonArray pointsArray = json["checkPoints"].toArray();
    for (const auto& pointValue : pointsArray) {
        CheckPoint point;
        point.fromJson(pointValue.toObject());
        checkPoints.append(point);
    }
}
