#ifndef COMMANDSETTINGS_H
#define COMMANDSETTINGS_H

#include <QWidget>
#include "mission.h"
#include "checkpoint.h"

namespace Ui {
class CommandSettings;
}

class CommandSettings : public QWidget
{
    Q_OBJECT

public:
    explicit CommandSettings(Mission& mission, QWidget *parent = nullptr);
    ~CommandSettings();

    void updatePointIndex(int idx);

signals:
    void checkPointChanged(const CheckPoint& point);
    void requestApply();
    void requestReset();
    void requestCancel();

private slots:
    void onWaypointIndexChanged(int index);
    void onCommandIndexChanged(int index);
    void onApplyClicked();
    void onResetClicked();
    void onCancelClicked();

private:
    Ui::CommandSettings *ui;

    Mission& m_mission;
    int      m_currentIndex;
    CmdEnum  m_savedCmd      = CmdEnum::None;
    double   m_savedAltitude = 50.0;

    void rebuildWaypointList();
    void updateUI();
    void loadSavedIntoUI();
    void buildLegend();
    void connectDialogOptions();
};

#endif // COMMANDSETTINGS_H
