#include "coordinatessettings.h"
#include "ui_coordinatessettings.h"

CoordinatesSettings::CoordinatesSettings(QWidget *parent)
    : Dialog(parent)
    , ui(new Ui::CoordinatesSettings),
    coords()
{
    ui->setupUi(this);
}

CoordinatesSettings::~CoordinatesSettings()
{
    delete ui;
}

Coordinates CoordinatesSettings::get()
{
    return this->coords;
}

void CoordinatesSettings::closeEvent(QCloseEvent *ev)
{
    this->coords.updateCoordinates(
        this->ui->spinLantitude->value(),
        this->ui->spinLongitude->value(),
        this->ui->spinAltitude->value()
        );

    this->coords.updateCourse(
        this->ui->spinCourse->value()
        );

    emit this->dialogFinished();

    Dialog::closeEvent(ev);
}
