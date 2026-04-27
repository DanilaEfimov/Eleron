#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "environmentalsettings.h"
#include "boardsettings.h"
#include "telemetrysetting.h"
#include "globals.h"
#include "errordialog.h"
#include "commandsettings.h"
#include "area.h"
#include "Logger.h"

#include <QPushButton>
#include <QFile>
#include <QFileDialog>
#include <QStyle>
#include <QScrollBar>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QSignalBlocker>
#include <QToolButton>
#include <QStatusBar>
#include <algorithm>

namespace {

double simulationSpeedStepFor(double factor)
{
    if (factor < 1.0) {
        return 0.25;
    }
    if (factor < 2.0) {
        return 0.5;
    }
    if (factor < 5.0) {
        return 1.0;
    }
    if (factor < 20.0) {
        return 2.0;
    }
    if (factor < 50.0) {
        return 5.0;
    }
    return 10.0;
}

QString compactSpeedText(double value)
{
    QString text = QString::number(value, 'f', 2);
    while (text.contains('.') && text.endsWith('0')) {
        text.chop(1);
    }
    if (text.endsWith('.')) {
        text.chop(1);
    }
    return text;
}

} // namespace

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow),
    mission()
{
    ui->setupUi(this);
    this->binding();

    this->setDarkTheme();
    this->setLeftAlign();

    this->ui->canvas->setMission(this->mission);
    this->ui->zoomSlider->setValue(100);
    this->updatePlayPauseButton(false);
    this->setSimulationSpeedFactor(1.0);

    this->setWindowIcon(QIcon(globals::iconPath));
    this->setIconSize(globals::iconSize);

    Logger::log("Приложение выполняется");
}

MainWindow::~MainWindow()
{
    Logger::log("Приложение закрыто");
    delete ui;
}

void MainWindow::binding()
{
    connect(ui->environmentDialog, &QPushButton::clicked, this, &MainWindow::environmentDialog);
    connect(ui->boardDialog, &QPushButton::clicked, this, &MainWindow::onboardSystemDialog);
    connect(ui->telemetryDialog, &QPushButton::clicked, this, &MainWindow::telemetryDialog);
    connect(ui->areaDialog, &QPushButton::clicked, this, &MainWindow::areaDialog);
    connect(ui->playPauseBtn, &QPushButton::clicked, this, &MainWindow::toggleFlightPlayback);
    connect(ui->commandsEditor, &QPushButton::clicked, this, &MainWindow::commandEditorDialog);
    connect(ui->zoomInButton, &QToolButton::clicked, this, &MainWindow::zoomIn);
    connect(ui->zoomOutButton, &QToolButton::clicked, this, &MainWindow::zoomOut);
    connect(ui->zoomSlider, &QSlider::valueChanged, this, &MainWindow::zoomChanged);
    connect(ui->simSpeedSpin,
            qOverload<double>(&QDoubleSpinBox::valueChanged),
            this,
            &MainWindow::simulationSpeedChanged);
    connect(ui->simSpeedResetButton, &QToolButton::clicked, this, &MainWindow::stepSimulationSpeed);

    if (QLineEdit *editor = ui->simSpeedSpin->findChild<QLineEdit *>()) {
        connect(editor, &QLineEdit::selectionChanged, this, [editor]() {
            if (editor->hasSelectedText()) {
                editor->deselect();
            }
        });
    }

    connect(ui->canvas, &CanvasWidget::pointClicked, this, &MainWindow::commandEditorDialog);
    connect(ui->canvas, &CanvasWidget::flightPlaybackChanged, this, &MainWindow::updatePlayPauseButton);
    connect(ui->canvas, &CanvasWidget::flightFinished, this, [this]() {
        ui->playPauseBtn->setText("Сброс");
        statusBar()->showMessage("Полёт завершён", 3000);
        Logger::log("Миссия завершена");
    });
    connect(ui->canvas, &CanvasWidget::panRequested, this, [this](QPoint delta) {
        ui->canvasScrollArea->horizontalScrollBar()->setValue(
            ui->canvasScrollArea->horizontalScrollBar()->value() + delta.x());
        ui->canvasScrollArea->verticalScrollBar()->setValue(
            ui->canvasScrollArea->verticalScrollBar()->value() + delta.y());
    });

    connect(ui->canvas, &CanvasWidget::zoomStepRequested, this, [this](int steps) {
        ui->zoomSlider->setValue(ui->zoomSlider->value() + steps * ui->zoomSlider->singleStep());
    });

    connect(ui->actionLight_theme, &QAction::triggered, this, &MainWindow::setLightTheme);
    connect(ui->actionDark_theme, &QAction::triggered, this, &MainWindow::setDarkTheme);
    connect(ui->actionLeft_aligned, &QAction::triggered, this, &MainWindow::setLeftAlign);
    connect(ui->actionRight_aligned, &QAction::triggered, this, &MainWindow::setRightAlign);

    connect(this->ui->actionExport_mission, &QAction::triggered, this, &MainWindow::exportMission);
    connect(this->ui->actionImport_mission, &QAction::triggered, this, &MainWindow::importMission);
}

void MainWindow::commandEditorDialog(int idx)
{
    auto* win = new CommandSettings(this->mission);

    win->setStyleSheet(this->styleSheet());

    connect(win, &CommandSettings::checkPointChanged,
            this, [this](const CheckPoint& cp)
            {
                ui->canvas->update();

                Logger::log("Настройки контрольной точки изменены: " + cp.toString());
            });

    win->show();

    win->updatePointIndex(idx);
}

void MainWindow::environmentDialog()
{
    auto* win = new EnvironmentalSettings(this->mission.env);

    QString style = this->styleSheet();
    win->setStyleSheet(style);

    connect(win, &EnvironmentalSettings::dialogFinished, this, [this, win](){
        this->mission.env = win->get();

        win->deleteLater();

        Logger::log("Параметры окружения изменены");
        Logger::log(this->mission.env.toString());
    });

    win->show();
}

void MainWindow::telemetryDialog()
{
    auto* win = new TelemetrySettingWindow();

    QString style = this->styleSheet();
    win->setStyleSheet(style);

    connect(win, &TelemetrySettingWindow::dialogFinished, this, [this, win](){
        this->mission.telemetry = win->get();

        win->deleteLater();

        Logger::log("Параметры телеметрии изменены");
        Logger::log(this->mission.telemetry.toString());
    });

    win->show();
}

void MainWindow::onboardSystemDialog()
{
    BoardSettings* win = new BoardSettings();

    QString style = this->styleSheet();
    win->setStyleSheet(style);

    connect(win, &BoardSettings::dialogFinished, this, [this, win](){
        this->mission.board = win->get();

        win->deleteLater();

        Logger::log("Параметры дрона изменены");
        Logger::log(this->mission.board.toString());
    });

    win->show();
}

void MainWindow::areaDialog()
{
    const QString selectedFile = Area::selectFile(this);

    if (selectedFile.isEmpty()) {
        return;
    }

    if (!ui->canvas->setAreaImage(selectedFile)) {
        ErrorDialog::showError("Не удалось загрузить выбранный файл зоны.");
        Logger::log("Ошибка: не удалось загрузить выбранный файл зоны");
        return;
    }

    Logger::log("Местность полета выбрана: " + selectedFile);
}

void MainWindow::toggleFlightPlayback()
{
    static bool missionStarted = false;

    if (!missionStarted) {
        Logger::log("=== МИССИЯ НАЧАТА ===");
        missionStarted = true;
    }

    if (!ui->canvas->toggleFlightPlayback()) {
        statusBar()->showMessage("Добавьте как минимум две точки маршрута для начала полёта", 3000);
        Logger::log("Ошибка: недостаточно точек маршрута для начала полёта");
    } else {
        Logger::log(">>> ПОЛЁТ ЗАПУЩЕН <<<");
    }
}

void MainWindow::zoomIn()
{
    ui->zoomSlider->setValue(ui->zoomSlider->value() + ui->zoomSlider->singleStep());
}

void MainWindow::zoomOut()
{
    ui->zoomSlider->setValue(ui->zoomSlider->value() - ui->zoomSlider->singleStep());
}

void MainWindow::zoomChanged(int value)
{
    setZoomPercent(value);
}

void MainWindow::simulationSpeedChanged(double value)
{
    setSimulationSpeedFactor(value);
}

void MainWindow::stepSimulationSpeed()
{
    setSimulationSpeedFactor(simulationSpeedFactor + simulationSpeedStepFor(simulationSpeedFactor));
}

void MainWindow::setZoomPercent(int zoomPercent)
{
    auto *horizontalBar = ui->canvasScrollArea->horizontalScrollBar();
    auto *verticalBar = ui->canvasScrollArea->verticalScrollBar();
    const QSize viewportSize = ui->canvasScrollArea->viewport()->size();

    const double previousZoom = ui->canvas->zoomFactor();
    const double nextZoom = static_cast<double>(zoomPercent) / 100.0;
    ui->zoomValueLabel->setText(QString("%1%").arg(zoomPercent));

    if (qFuzzyCompare(previousZoom, nextZoom)) {
        return;
    }

    const double centerX = horizontalBar->value() + viewportSize.width() / 2.0;
    const double centerY = verticalBar->value() + viewportSize.height() / 2.0;
    const double zoomRatio = nextZoom / previousZoom;

    ui->canvas->setZoomFactor(nextZoom);

    horizontalBar->setValue(qRound(centerX * zoomRatio - viewportSize.width() / 2.0));
    verticalBar->setValue(qRound(centerY * zoomRatio - viewportSize.height() / 2.0));
}

void MainWindow::setSimulationSpeedFactor(double factor)
{
    simulationSpeedFactor = std::clamp(factor, 0.25, 100.0);
    ui->canvas->setSimulationSpeedFactor(simulationSpeedFactor);
    updateSimulationSpeedControls();
}

void MainWindow::updateSimulationSpeedControls()
{
    const double step = simulationSpeedStepFor(simulationSpeedFactor);
    const QSignalBlocker blocker(ui->simSpeedSpin);
    ui->simSpeedSpin->setSingleStep(step);
    ui->simSpeedSpin->setValue(simulationSpeedFactor);

    if (QLineEdit *editor = ui->simSpeedSpin->findChild<QLineEdit *>()) {
        editor->deselect();
    }

    const QString stepText = compactSpeedText(step);
    ui->simSpeedResetButton->setText(QString("+%1x").arg(stepText));
    ui->simSpeedResetButton->setToolTip(
        QString("Увеличить скорость симуляции на %1x").arg(stepText));
}

void MainWindow::updatePlayPauseButton(bool isPlaying)
{
    ui->playPauseBtn->setText(isPlaying ? "Пауза" : "Старт");
}

void MainWindow::setLightTheme()
{
    QFile stylesheet(globals::lightThemeFilepath);

    if(!stylesheet.open(QIODevice::ReadOnly)){
        ErrorDialog::showError("Не удалось загрузить стили светлой темы.");
        Logger::log("Ошибка: не удалось загрузить стили светлой темы");
        return;
    }

    QString qss = stylesheet.readAll();
    this->setStyleSheet(qss);
}

void MainWindow::setDarkTheme()
{
    QFile stylesheet(globals::darkThemeFilepath);

    if(!stylesheet.open(QIODevice::ReadOnly)){
        ErrorDialog::showError("Не удалось загрузить стили тёмной темы.");
        Logger::log("Ошибка: не удалось загрузить стили тёмной темы");
        return;
    }

    QString qss = stylesheet.readAll();
    this->setStyleSheet(qss);
}

void MainWindow::setRightAlign()
{
    this->ui->horizontalLayout->setDirection(QHBoxLayout::RightToLeft);
    this->ui->horizontalLayout_2->setDirection(QHBoxLayout::RightToLeft);
}

void MainWindow::setLeftAlign()
{
    this->ui->horizontalLayout->setDirection(QHBoxLayout::LeftToRight);
    this->ui->horizontalLayout_2->setDirection(QHBoxLayout::LeftToRight);
}

void MainWindow::exportMission()
{
    QString defaultName = QString("mission_%1.json")
    .arg(QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss"));

    QString filepath = QFileDialog::getSaveFileName(this,
                                                    "Экспорт миссии",
                                                    QDir::homePath() + "/" + defaultName,
                                                    "JSON files (*.json)");

    if (!filepath.isEmpty()) {
        if (mission.saveToFile(filepath)) {
            statusBar()->showMessage("Миссия успешно экспортирована", 3000);
            Logger::log("Миссия экспортирована в: " + filepath);
        } else {
            ErrorDialog::showError("Не удалось сохранить файл миссии");
            Logger::log("Ошибка экспорта миссии в: " + filepath);
        }
    }
}

void MainWindow::importMission()
{
    QString filepath = QFileDialog::getOpenFileName(this,
                                                    "Импорт миссии",
                                                    QDir::homePath(),
                                                    "JSON files (*.json)");

    if (!filepath.isEmpty()) {
        Mission loadedMission;
        if (loadedMission.loadFromFile(filepath)) {
            mission = loadedMission;
            ui->canvas->setMission(mission);
            ui->canvas->update();
            statusBar()->showMessage("Миссия успешно импортирована", 3000);
            Logger::log("Миссия импортирована из: " + filepath);
        } else {
            ErrorDialog::showError("Файл миссии повреждён или имеет неверный формат");
            Logger::log("Ошибка импорта миссии из: " + filepath);
        }
    }
}
