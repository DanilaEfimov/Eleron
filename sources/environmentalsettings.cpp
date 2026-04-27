#include "environmentalsettings.h"
#include "errordialog.h"
#include "ui_environmentalsettings.h"

#include <QPushButton>

EnvironmentalSettings::EnvironmentalSettings(QWidget *parent)
    : EnvironmentalSettings(EnvironmentalParameters(), parent)
{
}

EnvironmentalSettings::EnvironmentalSettings(const EnvironmentalParameters &initial, QWidget *parent)
    : Dialog(parent)
    , ui(new Ui::EnvironmentalSettingsWindow)
    , env(initial)
{
    ui->setupUi(this);
    setWindowTitle("Настройки среды");
    syncUiFromEnv();

    auto applyBtn = ui->widget->findChild<QPushButton*>("pushButton");
    auto resetBtn = ui->widget->findChild<QPushButton*>("pushButton_2");
    auto cancelBtn = ui->widget->findChild<QPushButton*>("pushButton_3");

    if (applyBtn != nullptr) {
        connect(applyBtn, &QPushButton::clicked, this, [this]() {
            if (applyFromUi(true)) {
                this->close();
            }
        });
    }

    if (resetBtn != nullptr) {
        connect(resetBtn, &QPushButton::clicked, this, [this]() {
            syncUiFromEnv();
        });
    }

    if (cancelBtn != nullptr) {
        connect(cancelBtn, &QPushButton::clicked, this, [this]() {
            applyOnClose = false;
            this->close();
        });
    }
}

EnvironmentalSettings::~EnvironmentalSettings()
{
    delete ui;
}

EnvironmentalParameters EnvironmentalSettings::get()
{
    return this->env;
}

bool EnvironmentalSettings::applyFromUi(bool showError)
{
    const bool changed = this->env.updateWeather(
        this->ui->windSpin->value(),
        this->ui->courseSpin->value(),
        this->ui->temperatureSpin->value(),
        this->ui->humiditySpin->value()
    );

    if (!changed && showError) {
        ErrorDialog::showError("Введены некорректные значения параметров среды.");
    }

    return changed;
}

void EnvironmentalSettings::syncUiFromEnv()
{
    this->ui->windSpin->setValue(this->env.windSpeed);
    this->ui->courseSpin->setValue(this->env.windDirection);
    this->ui->temperatureSpin->setValue(this->env.temperature);
    this->ui->humiditySpin->setValue(static_cast<int>(this->env.humidity));
}

void EnvironmentalSettings::closeEvent(QCloseEvent *ev)
{
    if (applyOnClose) {
        applyFromUi(true);
    }

    emit this->dialogFinished();

    Dialog::closeEvent(ev);
}
