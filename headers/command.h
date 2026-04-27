#ifndef COMMAND_H
#define COMMAND_H

#include "data.h"
#include "cmdenum.h"

class Command : Data
{
    CmdEnum action;

public:
    Command(CmdEnum action);

    CmdEnum getAction() const;
    QString toString() const override;
    QJsonObject toJson() const override;
    void fromJson(const QJsonObject &json) override;
};

#endif // COMMAND_H
