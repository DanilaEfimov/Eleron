#ifndef WAY_H
#define WAY_H

#include "checkpoint.h"
#include "data.h"

#include <QVector>

class Way : Data
{
private:
    QVector<CheckPoint> checkPoints;

public:
    Way() : checkPoints() {};
    Way(const QVector<CheckPoint> &points);

    QVector<CheckPoint> getCheckPoints() const;
    CheckPoint getCheckPoint(int index) const;
    CheckPoint& getCheckPointRef(int index);

    void addCheckPoint(const CheckPoint &point);
    void updateCheckPoint(int index, const CheckPoint &point);
    void removeCheckPoint(int index);
    void removeLast();
    void clear();

    int size() const;
    bool empty() const;

    QString toString() const override;
    QJsonObject toJson() const override;
    void fromJson(const QJsonObject &json) override;
};

#endif // WAY_H
