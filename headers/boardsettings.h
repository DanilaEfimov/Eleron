#ifndef BOARDSETTINGS_H
#define BOARDSETTINGS_H

#include "dialog.h"
#include "onboardsystem.h"

#include <QWidget>

namespace Ui {
class BoardSettings;
}

class BoardSettings : public Dialog
{
    Q_OBJECT

public:
    explicit BoardSettings(QWidget *parent = nullptr);
    ~BoardSettings();

    OnboardSystem get() const;

private:
    Ui::BoardSettings *ui;
    OnboardSystem board;

protected:
    void closeEvent(QCloseEvent* ev) override;
};

#endif // BOARDSETTINGS_H
