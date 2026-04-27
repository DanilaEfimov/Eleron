#include "telemetrysetting.h"
#include "errordialog.h"
#include "globals.h"
#include "ui_telemetrysetting.h"

TelemetrySettingWindow::TelemetrySettingWindow(QWidget *parent)
    : Dialog(parent)
    , ui(new Ui::TelemetrySettingWindow)
{
    ui->setupUi(this);

    ui->frequencySpin->setMaximum(globals::maxFrequencyMhz);

    ui->modulationComboBox->addItems(globals::validModulations);
}

TelemetrySettingWindow::~TelemetrySettingWindow()
{
    delete ui;
}

Telemetry TelemetrySettingWindow::get() const
{
    return this->telemetry;
}

void TelemetrySettingWindow::closeEvent(QCloseEvent *ev)
{
    bool changed = this->telemetry.updateTelemetry(
        this->ui->satellitesSpin->value(),
        this->ui->frequencySpin->value(),
        this->ui->modulationComboBox->currentText()
        );

    if(!changed){
        ErrorDialog::showError("Введены некорректные значения телеметрии.");
    }

    emit this->dialogFinished();

    Dialog::closeEvent(ev);
}
