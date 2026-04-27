#ifndef COORDINATESSETTINGS_H
#define COORDINATESSETTINGS_H

#include "coordinates.h"
#include "dialog.h"

#include <QWidget>

namespace Ui {
class CoordinatesSettings;
}

class CoordinatesSettings : public Dialog
{
    Q_OBJECT

public:
    explicit CoordinatesSettings(QWidget *parent = nullptr);
    ~CoordinatesSettings();

    Coordinates get();

private:
    Ui::CoordinatesSettings *ui;
    Coordinates coords;

protected:
    void closeEvent(QCloseEvent* ev) override;
};

#endif // COORDINATESSETTINGS_H
