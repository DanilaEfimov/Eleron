#ifndef ENVIRONMENTALSETTINGS_H
#define ENVIRONMENTALSETTINGS_H

#include "dialog.h"
#include "environmentparameters.h"

#include <QWidget>

namespace Ui {
class EnvironmentalSettingsWindow;
}

class EnvironmentalSettings : public Dialog
{
    Q_OBJECT

public:
    explicit EnvironmentalSettings(QWidget *parent = nullptr);
    explicit EnvironmentalSettings(const EnvironmentalParameters &initial, QWidget *parent = nullptr);
    ~EnvironmentalSettings();

    EnvironmentalParameters get();

private:
    Ui::EnvironmentalSettingsWindow *ui;
    EnvironmentalParameters env;
    bool applyOnClose = true;

    bool applyFromUi(bool showError);
    void syncUiFromEnv();

protected:
    void closeEvent(QCloseEvent* ev) override;
};

#endif // ENVIRONMENTALSETTINGS_H
