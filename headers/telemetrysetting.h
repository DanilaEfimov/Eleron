#ifndef TELEMETRYSETTING_H
#define TELEMETRYSETTING_H

#include "dialog.h"
#include "telemetry.h"

#include <QWidget>

namespace Ui {
class TelemetrySettingWindow;
}

class TelemetrySettingWindow : public Dialog
{
    Q_OBJECT

public:
    explicit TelemetrySettingWindow(QWidget *parent = nullptr);
    ~TelemetrySettingWindow();

    Telemetry get() const;

private:
    Ui::TelemetrySettingWindow *ui;
    Telemetry telemetry;

protected:
    void closeEvent(QCloseEvent* ev) override;
};

#endif // TELEMETRYSETTING_H
