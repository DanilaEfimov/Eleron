#include "boardsettings.h"
#include "ui_boardsettings.h"

BoardSettings::BoardSettings(QWidget *parent)
    : Dialog(parent)
    , ui(new Ui::BoardSettings),
    board()
{
    ui->setupUi(this);

    connect(this->ui->coordsMenu, &CoordinatesSettings::dialogFinished, this, [this](){
        this->board.coordinates = this->ui->coordsMenu->get();
    });
}

BoardSettings::~BoardSettings()
{
    delete ui;
}

OnboardSystem BoardSettings::get() const
{
    return board;
}

void BoardSettings::closeEvent(QCloseEvent *ev)
{
    this->ui->coordsMenu->close();

    this->board.updateBatteryLevel(this->ui->spinBattery->value());
    this->board.updateSpeed(this->ui->spinSpeed->value());
    this->board.updateTemperature(this->ui->spinTemperature->value());

    emit this->dialogFinished();

    Dialog::closeEvent(ev);
}
