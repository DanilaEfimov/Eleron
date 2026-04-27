#ifndef CHECKPOINT_H
#define CHECKPOINT_H

#include "coordinates.h"
#include "command.h"
#include "cmdenum.h"
#include "data.h"

#include <QDebug>

class CheckPoint : Data
{
    Coordinates pos;
    Command cmd;

public:
    CheckPoint() : cmd(CmdEnum::None) {
        qDebug() << "CheckPoint<>::fake constructor called";
    }

    CheckPoint(const Coordinates &pos, const Command &cmd);

    CheckPoint(const CheckPoint &other);

    CheckPoint& operator=(const CheckPoint &other);

    Coordinates getPos() const;
    Command getCmd() const;

    QString toString() const override;
    QJsonObject toJson() const override;
    void fromJson(const QJsonObject &json) override;
};

#endif // CHECKPOINT_H
