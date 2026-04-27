#ifndef AREA_H
#define AREA_H

#include <QString>

class QWidget;

class Area
{
public:
    static QString selectFile(QWidget *parent);

private:
    static QString resolveSourcesDir();
};

#endif // AREA_H
