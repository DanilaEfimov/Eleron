#ifndef DATA_H
#define DATA_H

#include <QString>
#include <QJsonObject>

class Data
{
public:
    ~Data() = default;

    virtual QString toString() const = 0;
    virtual QJsonObject toJson() const = 0;
    virtual void fromJson(const QJsonObject& json) = 0;
};

#endif // DATA_H
