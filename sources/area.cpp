#include "area.h"
#include "globals.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileDialog>
#include <QObject>
#include <QWidget>

QString Area::selectFile(QWidget *parent)
{
    return QFileDialog::getOpenFileName(
        parent,
        QObject::tr("Выберите зону"),
        resolveSourcesDir(),
        QObject::tr("Изображения (*.png *.jpg *.jpeg *.bmp)")
        );
}

QString Area::resolveSourcesDir()
{
    const QString relativeAreaDir = globals::areaSourcesDir;
    const QString appDir = QCoreApplication::applicationDirPath();
    const QStringList candidates = {
        QDir::current().absoluteFilePath(relativeAreaDir),
        QDir(appDir).absoluteFilePath(relativeAreaDir),
        QDir(appDir).absoluteFilePath("../" + relativeAreaDir),
        QDir(appDir).absoluteFilePath("../../" + relativeAreaDir),
        QDir(appDir).absoluteFilePath("../../../" + relativeAreaDir)
    };

    for (const QString &candidate : candidates) {
        if (QDir(candidate).exists()) {
            return QDir(candidate).absolutePath();
        }
    }

    return QDir::currentPath();
}
