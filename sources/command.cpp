#include "command.h"
#include "globals.h"

Command::Command(CmdEnum action)
    : action(action)
{}

CmdEnum Command::getAction() const
{
    return action;
}

QString Command::toString() const
{
    return globals::CmdNamesRu.value(this->action, "Неизвестная команда");
}

QJsonObject Command::toJson() const
{
    QJsonObject obj;
    obj["action"] = static_cast<int>(action);
    obj["actionName"] = globals::CmdNamesRu.value(action, "Unknown");
    return obj;
}

void Command::fromJson(const QJsonObject &json)
{
    action = static_cast<CmdEnum>(json["action"].toInt());
}
