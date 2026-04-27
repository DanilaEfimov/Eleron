#include "checkpoint.h"

CheckPoint::CheckPoint(const CheckPoint &other)
    : Data(),
    pos(other.pos),
    cmd(other.cmd)
{}

CheckPoint::CheckPoint(const Coordinates &pos, const Command &cmd)
    : Data(),
    pos(pos),
    cmd(cmd)
{}

CheckPoint& CheckPoint::operator=(const CheckPoint &other)
{
    if (this != &other) {
        pos = other.pos;
        cmd = other.cmd;
    }

    return *this;
}

Coordinates CheckPoint::getPos() const
{
    return pos;
}

Command CheckPoint::getCmd() const
{
    return cmd;
}

QString CheckPoint::toString() const
{
    return QString("CheckPoint={pos=%1, cmd=%2}")
        .arg(this->pos.toString())
        .arg(this->cmd.toString());
}

QJsonObject CheckPoint::toJson() const
{
    QJsonObject obj;
    obj["position"] = pos.toJson();
    obj["command"] = cmd.toJson();
    return obj;
}

void CheckPoint::fromJson(const QJsonObject &json)
{
    pos.fromJson(json["position"].toObject());
    cmd.fromJson(json["command"].toObject());
}
