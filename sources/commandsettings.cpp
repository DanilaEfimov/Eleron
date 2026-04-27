#include "commandsettings.h"
#include "globals.h"
#include "ui_commandsettings.h"

#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QGridLayout>

static bool isAltitudeCommand(CmdEnum cmd)
{
    return cmd == CmdEnum::ChangeAltitudeUp || cmd == CmdEnum::ChangeAltitudeDown;
}

CommandSettings::CommandSettings(Mission& mission, QWidget *parent)
    : QWidget(parent),
    ui(new Ui::CommandSettings),
    m_mission(mission),
    m_currentIndex(-1)
{
    ui->setupUi(this);

    connect(ui->comboCommand,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &CommandSettings::onCommandIndexChanged);

    ui->comboCommand->clear();

    for (auto it = globals::CmdNamesRu.begin();
         it != globals::CmdNamesRu.end();
         ++it)
    {
        ui->comboCommand->addItem(it.value(), static_cast<int>(it.key()));
    }

    connect(ui->comboWaypoint,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &CommandSettings::onWaypointIndexChanged);

    ui->groupAltitude->setVisible(false);

    buildLegend();

    rebuildWaypointList();

    connectDialogOptions();
}

CommandSettings::~CommandSettings()
{
    delete ui;
}

void CommandSettings::updatePointIndex(int idx)
{
    ui->comboWaypoint->setCurrentIndex(idx);
    emit this->onWaypointIndexChanged(idx);
}

void CommandSettings::rebuildWaypointList()
{
    ui->comboWaypoint->clear();

    for (int i = 0; i < m_mission.path.size(); ++i)
    {
        const Coordinates& c =
            m_mission.path.getCheckPoint(i).getPos();

        QString text = QString("ТМ %1 | шир:%2 долг:%3 выс:%4")
                           .arg(i + 1)
                           .arg(c.latitude, 0, 'f', 2)
                           .arg(c.longitude, 0, 'f', 2)
                           .arg(c.altitude, 0, 'f', 1);

        ui->comboWaypoint->addItem(text);
    }

    if (m_mission.path.size() > 0)
    {
        m_currentIndex = 0;
        ui->comboWaypoint->setCurrentIndex(0);
        updateUI();
    }
}

void CommandSettings::onWaypointIndexChanged(int index)
{
    if (index < 0 || index >= m_mission.path.size())
        return;

    m_currentIndex = index;
    updateUI();
}

void CommandSettings::updateUI()
{
    if (m_currentIndex < 0 ||
        m_currentIndex >= m_mission.path.size())
        return;

    const CheckPoint& cp = m_mission.path.getCheckPoint(m_currentIndex);
    const Coordinates& c = cp.getPos();

    ui->labelLat->setText(QString::number(c.latitude, 'f', 6));
    ui->labelLon->setText(QString::number(c.longitude, 'f', 6));
    ui->labelAlt->setText(QString::number(c.altitude, 'f', 1));
    ui->labelCourse->setText(QString::number(c.course, 'f', 1));

    m_savedCmd      = cp.getCmd().getAction();
    m_savedAltitude = c.altitude;

    loadSavedIntoUI();
}

void CommandSettings::loadSavedIntoUI()
{
    for (int i = 0; i < ui->comboCommand->count(); ++i) {
        if (ui->comboCommand->itemData(i).toInt() == static_cast<int>(m_savedCmd)) {
            QSignalBlocker blocker(ui->comboCommand);
            ui->comboCommand->setCurrentIndex(i);
            break;
        }
    }

    const bool altCmd = isAltitudeCommand(m_savedCmd);
    ui->groupAltitude->setVisible(altCmd);

    if (altCmd) {
        QSignalBlocker blocker(ui->spinTargetAltitude);
        ui->spinTargetAltitude->setValue(m_savedAltitude);
    }
}

void CommandSettings::onCommandIndexChanged(int index)
{
    const CmdEnum cmdEnum = static_cast<CmdEnum>(
        ui->comboCommand->itemData(index).toInt());

    const bool altCmd = isAltitudeCommand(cmdEnum);
    ui->groupAltitude->setVisible(altCmd);

    if (altCmd) {
        QSignalBlocker blocker(ui->spinTargetAltitude);
        ui->spinTargetAltitude->setValue(m_savedAltitude);
    }
}

void CommandSettings::onApplyClicked()
{
    if (m_currentIndex < 0 ||
        m_currentIndex >= m_mission.path.size())
        return;

    const CmdEnum cmdEnum = static_cast<CmdEnum>(
        ui->comboCommand->currentData().toInt());

    auto& cp = m_mission.path.getCheckPointRef(m_currentIndex);
    Coordinates newPos = cp.getPos();

    if (isAltitudeCommand(cmdEnum))
        newPos.altitude = ui->spinTargetAltitude->value();

    cp = CheckPoint(newPos, Command(cmdEnum));

    m_savedCmd      = cmdEnum;
    m_savedAltitude = newPos.altitude;

    emit checkPointChanged(cp);
    emit requestApply();
}

void CommandSettings::onResetClicked()
{
    if (m_currentIndex < 0 ||
        m_currentIndex >= m_mission.path.size())
        return;

    auto& cp = m_mission.path.getCheckPointRef(m_currentIndex);
    Coordinates newPos = cp.getPos();
    cp = CheckPoint(newPos, Command(CmdEnum::None));

    m_savedCmd      = CmdEnum::None;
    m_savedAltitude = newPos.altitude;

    loadSavedIntoUI();
    emit checkPointChanged(cp);
    emit requestReset();
}

void CommandSettings::onCancelClicked()
{
    loadSavedIntoUI();
    emit requestCancel();
    close();
}

void CommandSettings::buildLegend()
{
    auto* layout = qobject_cast<QGridLayout*>(ui->groupLegend->layout());
    if (!layout)
        return;

    const QList<CmdEnum> order = {
        CmdEnum::TakePhoto,
        CmdEnum::StartVideo,
        CmdEnum::EndVideo,
        CmdEnum::ChangeAltitudeUp,
        CmdEnum::ChangeAltitudeDown,
        CmdEnum::Drop,
    };

    for (int row = 0; row < order.size(); ++row) {
        const CmdEnum cmd = order[row];
        const QColor  color = globals::CmdColors.value(cmd);
        const QString name  = globals::CmdNamesRu.value(cmd);

        auto* swatch = new QLabel(this);
        swatch->setFixedSize(14, 14);
        swatch->setStyleSheet(
            QString("background-color: %1;"
                    "border: 1px solid rgba(0,0,0,0.4);"
                    "border-radius: 2px;")
                .arg(color.name()));

        auto* label = new QLabel(name, this);

        layout->addWidget(swatch, row, 0, Qt::AlignVCenter);
        layout->addWidget(label,  row, 1, Qt::AlignVCenter);
    }

    layout->setColumnStretch(1, 1);
}

void CommandSettings::connectDialogOptions()
{
    auto options = ui->dialogOptions;
    if (!options)
        return;

    auto applyBtn = options->findChild<QPushButton*>("pushButton");
    auto resetBtn = options->findChild<QPushButton*>("pushButton_2");
    auto cancelBtn = options->findChild<QPushButton*>("pushButton_3");

    if (applyBtn)
        connect(applyBtn, &QPushButton::clicked,
                this, &CommandSettings::onApplyClicked);

    if (resetBtn)
        connect(resetBtn, &QPushButton::clicked,
                this, &CommandSettings::onResetClicked);

    if (cancelBtn)
        connect(cancelBtn, &QPushButton::clicked,
                this, &CommandSettings::onCancelClicked);
}
