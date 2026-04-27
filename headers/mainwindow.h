#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "mission.h"

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    Mission mission;
    double simulationSpeedFactor = 1.0;

    void binding();
    void setZoomPercent(int zoomPercent);
    void setSimulationSpeedFactor(double factor);
    void updateSimulationSpeedControls();

private slots:
    void environmentDialog();
    void telemetryDialog();
    void onboardSystemDialog();
    void areaDialog();
    void toggleFlightPlayback();
    void zoomIn();
    void zoomOut();
    void zoomChanged(int value);
    void simulationSpeedChanged(double value);
    void stepSimulationSpeed();

    void commandEditorDialog(int idx = 0);
    void updatePlayPauseButton(bool isPlaying);

    void setLightTheme();
    void setDarkTheme();
    void setRightAlign();
    void setLeftAlign();

    void exportMission();
    void importMission();
};

#endif // MAINWINDOW_H
