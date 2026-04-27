#include "mission.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>


Mission::Mission() :
    path(),
    board(),
    env(),
    telemetry()
{}

Mission::Mission(const Way &path,
                 const OnboardSystem &board,
                 const EnvironmentalParameters &env,
                 const Telemetry &telemetry)
    : path(path),
    board(board),
    env(env),
    telemetry(telemetry)
{}


QString Mission::toString() const
{
    return QString("Mission={path=%1, board=%2, env=%3, telemetry=%4}")
        .arg(this->path.toString())
        .arg(this->board.toString())
        .arg(this->env.toString())
        .arg(this->telemetry.toString());
}

QJsonObject Mission::toJson() const
{
    QJsonObject obj;
    obj["path"] = path.toJson();
    obj["board"] = board.toJson();
    obj["environment"] = env.toJson();
    obj["telemetry"] = telemetry.toJson();
    obj["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    obj["version"] = "1.0";
    return obj;
}

void Mission::fromJson(const QJsonObject &json)
{
    path.fromJson(json["path"].toObject());
    board.fromJson(json["board"].toObject());
    env.fromJson(json["environment"].toObject());
    telemetry.fromJson(json["telemetry"].toObject());
}

bool Mission::saveToFile(const QString &filepath) const
{
    QJsonObject obj = this->toJson();
    QJsonDocument doc(obj);

    QFile file(filepath);
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "Failed to open file for writing:" << filepath;
        return false;
    }

    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

bool Mission::loadFromFile(const QString &filepath)
{
    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Failed to open file for reading:" << filepath;
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
        qDebug() << "Failed to parse JSON from file:" << filepath;
        return false;
    }

    this->fromJson(doc.object());
    return true;
}
