#include "canvaswidget.h"
#include "globals.h"

#include <QPainter>
#include <QColor>
#include <QRect>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QDebug>
#include <cmath>
#include <algorithm>
#include <QRandomGenerator>

constexpr int spriteSize = 50;
constexpr int lineWidth = 2;
constexpr int kFlightTimerIntervalMs = 16;
constexpr double kFallbackCruiseSpeed = 90.0;
constexpr double kMinCruiseSpeed = 20.0;
constexpr double kKmPerHourToMetersPerSec = 1000.0 / 3600.0;
constexpr double kHeadwindPenalty = 3.5;
constexpr double kTailwindBonus = 1.5;
constexpr double kCrosswindInfluence = 0.12;
constexpr double kCrosswindLimit = 24.0;
constexpr double kSpriteForwardOffsetDeg = 90.0;
constexpr double kMaxTurnRateDegPerSec = 60.0;
constexpr double kClimbRateMps = 5.0;

double calculateCourse(const CheckPoint& first, const CheckPoint& second) {
    double dx = second.getPos().longitude - first.getPos().longitude;
    double dy = second.getPos().latitude - first.getPos().latitude;

    double angle = std::atan2(dy, dx);
    double degrees = angle * 180.0 / M_PI;

    if (degrees < 0)
        degrees += 360;

    return degrees;
}

double pointCourse(const QPointF &vector)
{
    double degrees = std::atan2(vector.y(), vector.x()) * 180.0 / M_PI;
    if (degrees < 0.0) {
        degrees += 360.0;
    }

    return degrees;
}

double clampMagnitude(double value, double limit)
{
    return std::clamp(value, -limit, limit);
}

double shortestAngleDelta(double fromDeg, double toDeg)
{
    double delta = std::fmod(toDeg - fromDeg + 540.0, 360.0) - 180.0;
    return delta;
}

double normalizeAngle(double deg)
{
    double result = std::fmod(deg, 360.0);
    if (result < 0.0) {
        result += 360.0;
    }
    return result;
}

double advanceHeading(double currentDeg, double targetDeg, double maxStepDeg)
{
    const double delta = shortestAngleDelta(currentDeg, targetDeg);
    if (std::abs(delta) <= maxStepDeg) {
        return normalizeAngle(targetDeg);
    }
    return normalizeAngle(currentDeg + std::copysign(maxStepDeg, delta));
}

CanvasWidget::CanvasWidget(QWidget *parent)
    : QWidget(parent),
    mission(nullptr)
{
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setMouseTracking(true);
    setCursor(Qt::CrossCursor);

    sprite = QPixmap(":/sprites/drone_transparent.png");
    if (!sprite.isNull()) {
        sprite = sprite.scaled(spriteSize,
                               spriteSize,
                               Qt::KeepAspectRatio,
                               Qt::SmoothTransformation);
    }

    const QMap<CmdEnum, QString> spritePaths = {
        { CmdEnum::TakePhoto,          ":/sprites/photo_action48.png"         },
        { CmdEnum::StartVideo,         ":/sprites/video_play_action48.png"    },
        { CmdEnum::EndVideo,           ":/sprites/video_pause_action48.png"   },
        { CmdEnum::ChangeAltitudeUp,   ":/sprites/altitude_up_action48.png"   },
        { CmdEnum::ChangeAltitudeDown, ":/sprites/altitude_down_action48.png" },
        { CmdEnum::Drop,               ":/sprites/drop_action48.png"          },
    };
    for (auto it = spritePaths.begin(); it != spritePaths.end(); ++it) {
        QPixmap px(it.value());
        if (!px.isNull())
            actionSprites[it.key()] = px;
    }

    flightTimer.setInterval(kFlightTimerIntervalMs);
    connect(&flightTimer, &QTimer::timeout, this, [this]() { advanceFlight(); });

    updateCanvasSize();
}

void CanvasWidget::clearWaypoints()
{
    if (mission == nullptr) {
        return;
    }

    resetFlight();
    mission->path.clear();
    update();
}

void CanvasWidget::setMission(Mission &mission)
{
    resetFlight();
    this->mission = &mission;
    update();
}

bool CanvasWidget::setAreaImage(const QString &filePath)
{
    QPixmap pixmap;
    if (!pixmap.load(filePath)) {
        return false;
    }

    this->pixmapPath = filePath;
    areaPixmap = pixmap;
    updateCanvasSize();
    update();
    return true;
}

void CanvasWidget::setZoomFactor(double factor)
{
    const double normalizedFactor = std::max(0.1, factor);
    if (qFuzzyCompare(currentZoomFactor, normalizedFactor)) {
        return;
    }

    currentZoomFactor = normalizedFactor;
    updateCanvasSize();
    update();
}

double CanvasWidget::zoomFactor() const
{
    return currentZoomFactor;
}

void CanvasWidget::setSimulationSpeedFactor(double factor)
{
    currentSimulationSpeedFactor = std::clamp(factor, 0.1, 100.0);
}

double CanvasWidget::simulationSpeedFactor() const
{
    return currentSimulationSpeedFactor;
}

bool CanvasWidget::toggleFlightPlayback()
{
    if (!hasRoute()) {
        return false;
    }

    if (flightCompleted) {
        startFlight();
        return true;
    }

    if (!flightInProgress) {
        startFlight();
        return true;
    }

    if (flightPaused) {
        resumeFlight();
    } else {
        pauseFlight();
    }

    return true;
}

bool CanvasWidget::isFlightPlaying() const
{
    return flightInProgress && !flightPaused;
}

bool CanvasWidget::hasRoute() const
{
    return mission != nullptr && mission->path.size() >= 2;
}

QSize CanvasWidget::sizeHint() const
{
    return size();
}

void CanvasWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.scale(currentZoomFactor, currentZoomFactor);

    if (areaPixmap.isNull()) {
        painter.fillRect(QRect(QPoint(0, 0), logicalCanvasSize()), globals::canvasBackground);
    } else {
        painter.drawPixmap(0, 0, areaPixmap);
    }

    this->drawPath(painter);
    this->drawWaypoints(painter);
    this->drawDrone(painter);
    this->drawAltitudeLabel(painter);
    this->drawTelemetryNoise(painter);
}

void CanvasWidget::mousePressEvent(QMouseEvent *event)
{
    if (mission == nullptr) {
        return;
    }

    if (event->button() == Qt::LeftButton) {
        if (flightInProgress) {
            resetFlight();
        }

        Way& path = mission->path;
        const QPointF mousePos = event->position() / currentZoomFactor;

        qDebug() << "Mouse clicked at:" << mousePos;

        int existingPoint = this->findWaypointNear(mousePos, globals::radius);
        if(existingPoint != -1) {
            emit this->pointClicked(existingPoint);
            return;
        }

        Coordinates coords;
        coords.longitude = mousePos.x();
        coords.latitude = mousePos.y();
        coords.altitude = globals::defaultHeight;
        coords.course = 0.0;

        qDebug() << "Created coordinates - longitude:" << coords.longitude << "latitude:" << coords.latitude;

        if (!path.empty()) {
            CheckPoint temp(coords, Command(CmdEnum::None));
            coords.course = calculateCourse(
                path.getCheckPoint(path.size() - 1),
                temp
                );
            qDebug() << "Calculated course:" << coords.course;
        }

        CheckPoint point(coords, Command(CmdEnum::None));
        path.addCheckPoint(point);
        emit waypointAdded(mousePos);

        qDebug() << "Total waypoints:" << path.size();

        update();
        return;
    }

    if (event->button() == Qt::RightButton) {
        rightDragging = true;
        rightDragMoved = false;
        rightDragLastPos = event->globalPosition().toPoint();
        setCursor(Qt::ClosedHandCursor);
    }
}

void CanvasWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (rightDragging && (event->buttons() & Qt::RightButton)) {
        const QPoint globalPos = event->globalPosition().toPoint();
        const QPoint delta = globalPos - rightDragLastPos;
        if (!delta.isNull()) {
            rightDragMoved = true;
            rightDragLastPos = globalPos;
            emit panRequested(-delta);
        }
    }
}

void CanvasWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        rightDragging = false;
        setCursor(Qt::CrossCursor);

        if (!rightDragMoved && mission != nullptr && !mission->path.empty()) {
            const int removedIndex = mission->path.size() - 1;
            mission->path.removeLast();
            emit waypointRemoved(removedIndex);
            qDebug() << "Removed last waypoint. Remaining:" << mission->path.size();
            update();
        }
    }
}

void CanvasWidget::wheelEvent(QWheelEvent *event)
{
    const int steps = event->angleDelta().y() / 120;
    if (steps != 0) {
        emit zoomStepRequested(steps);
        event->accept();
    }
}

void CanvasWidget::drawPath(QPainter &painter)
{
    if (mission == nullptr) {
        return;
    }

    const auto& path = mission->path;
    if (path.size() < 2)
        return;

    QPen pen(QColor(0, 102, 255));
    pen.setWidthF(overlaySize(lineWidth, 3.0));
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    painter.setPen(pen);

    for (int i = 0; i < path.size() - 1; ++i) {
        const auto& p1 = path.getCheckPoint(i).getPos();
        const auto& p2 = path.getCheckPoint(i + 1).getPos();

        QPointF from(p1.longitude, p1.latitude);
        QPointF to(p2.longitude, p2.latitude);

        painter.drawLine(from, to);

        qDebug() << "Drawing line from" << from << "to" << to;
    }
}

void CanvasWidget::drawWaypoints(QPainter &painter)
{
    if (mission == nullptr) {
        return;
    }

    QPen outlinePen(Qt::black);
    outlinePen.setWidthF(overlaySize(1.5));
    painter.setPen(outlinePen);

    const auto& path = mission->path;

    const double waypointRadius = overlaySize(globals::radius);
    const double labelShift = overlaySize(globals::checkPointNumberShift);

    for (int i = 0; i < path.size(); ++i) {
        const auto& cp = path.getCheckPoint(i);
        const auto& pos = cp.getPos();
        QPointF point(pos.longitude, pos.latitude);

        const CmdEnum cmd = cp.getCmd().getAction();
        painter.setBrush(globals::CmdColors.value(cmd, Qt::red));

        painter.drawEllipse(point, waypointRadius, waypointRadius);

        painter.drawText(point + QPointF(
                             labelShift,
                             -labelShift
                             ),
                         QString::number(i + 1)
                         );

        bool visited = (i < waypointVisited.size()) && waypointVisited[i];
        if (visited && cmd != CmdEnum::None && actionSprites.contains(cmd)) {
            const QPixmap& px = actionSprites[cmd];
            const double iconSize = overlaySize(24);
            const QPointF iconTopLeft(point.x() - iconSize / 2.0,
                                      point.y() - waypointRadius - iconSize - overlaySize(2));
            painter.drawPixmap(QRectF(iconTopLeft, QSizeF(iconSize, iconSize)), px, px.rect());
        }

        qDebug() << "Drawing waypoint" << i + 1 << "at" << point;
    }
}

void CanvasWidget::drawDrone(QPainter &painter)
{
    if (!droneVisible || sprite.isNull())
        return;

    const double targetSize = overlaySize(spriteSize);
    const double spriteScale = targetSize / static_cast<double>(spriteSize);

    painter.save();

    QPointF noisyPos = applyTelemetryNoise(dronePosition);
    painter.translate(noisyPos);
    painter.rotate(droneCourse + kSpriteForwardOffsetDeg);
    painter.scale(spriteScale, spriteScale);

    painter.drawPixmap(
        -sprite.width() / 2,
        -sprite.height() / 2,
        sprite
        );

    painter.restore();

    qDebug() << "Drone at:" << dronePosition << "course:" << droneCourse;
}

QSize CanvasWidget::logicalCanvasSize() const
{
    if (!areaPixmap.isNull()) {
        return areaPixmap.size();
    }

    return QSize(kDefaultCanvasSize, kDefaultCanvasSize);
}

void CanvasWidget::updateCanvasSize()
{
    const QSize logicalSize = logicalCanvasSize();
    const QSize scaledSize(
        std::max(1, qRound(logicalSize.width() * currentZoomFactor)),
        std::max(1, qRound(logicalSize.height() * currentZoomFactor))
        );

    setFixedSize(scaledSize);
    updateGeometry();
}

double CanvasWidget::overlaySize(double pixels, double maxBoost) const
{
    const double safeZoom = std::max(0.1, currentZoomFactor);
    const double zoomOutBoost = std::max(1.0, std::min(maxBoost, 1.0 / std::sqrt(safeZoom)));
    return (pixels * zoomOutBoost) / safeZoom;
}

void CanvasWidget::advanceFlight()
{
    if (shouldDropFrame()) {
        return;
    }

    if (!flightInProgress || flightPaused || mission == nullptr) {
        return;
    }

    if (!hasRoute()) {
        resetFlight();
        return;
    }

    const qint64 elapsedMs = flightElapsedTimer.elapsed();
    const double deltaSeconds =
        (lastFlightTickMs == 0 ? kFlightTimerIntervalMs : std::max<qint64>(1, elapsedMs - lastFlightTickMs)) / 1000.0;
    lastFlightTickMs = elapsedMs;

    const double scaledDeltaSeconds = deltaSeconds * currentSimulationSpeedFactor;
    double remainingTickTime = scaledDeltaSeconds;

    while (remainingTickTime > 0.0) {
        if (activeSegmentIndex >= mission->path.size() - 1) {
            finishFlight();
            return;
        }

        const CheckPoint startPoint = mission->path.getCheckPoint(activeSegmentIndex);
        const CheckPoint endPoint = mission->path.getCheckPoint(activeSegmentIndex + 1);
        const QPointF start(startPoint.getPos().longitude, startPoint.getPos().latitude);
        const QPointF end(endPoint.getPos().longitude, endPoint.getPos().latitude);
        const QPointF segmentVector = end - start;
        const double segmentLength = std::hypot(segmentVector.x(), segmentVector.y());

        if (segmentLength <= globals::EPS) {
            ++activeSegmentIndex;
            segmentProgress = 0.0;
            segmentCrossTrackOffset = 0.0;
            dronePosition = end;
            if (activeSegmentIndex < waypointVisited.size())
                waypointVisited[activeSegmentIndex] = true;
            applyWaypointCommand(activeSegmentIndex);
            continue;
        }

        const QPointF direction = segmentVector / segmentLength;
        const QPointF normal(-direction.y(), direction.x());
        const double targetCourse = calculateCourse(startPoint, endPoint);
        const double windDeltaRadians =
            (mission->env.windDirection - targetCourse) * M_PI / 180.0;
        const double headwindComponent = mission->env.windSpeed * std::cos(windDeltaRadians);
        const double crosswindComponent = mission->env.windSpeed * std::sin(windDeltaRadians);

        double alongTrackSpeedKmh = resolveCruiseSpeed();
        alongTrackSpeedKmh -= std::max(0.0, headwindComponent) * kHeadwindPenalty;
        alongTrackSpeedKmh += std::max(0.0, -headwindComponent) * kTailwindBonus;
        alongTrackSpeedKmh = std::max(kMinCruiseSpeed, alongTrackSpeedKmh);
        droneCurrentSpeed = alongTrackSpeedKmh;

        const double alongTrackSpeedPxPerSec =
            (alongTrackSpeedKmh * kKmPerHourToMetersPerSec) / globals::mapMetersPerPixel;
        const double crosswindPxPerSec =
            (crosswindComponent * kKmPerHourToMetersPerSec) / globals::mapMetersPerPixel;

        const double remainingDistance = segmentLength * (1.0 - segmentProgress);
        const double availableTravelDistance = alongTrackSpeedPxPerSec * remainingTickTime;

        if (availableTravelDistance + globals::EPS >= remainingDistance) {
            const double consumedSeconds = remainingDistance / alongTrackSpeedPxPerSec;
            remainingTickTime -= consumedSeconds;
            dronePosition = end;
            droneCourse = advanceHeading(droneCourse, targetCourse, kMaxTurnRateDegPerSec * consumedSeconds);
            ++activeSegmentIndex;
            segmentProgress = 0.0;
            segmentCrossTrackOffset =
                clampMagnitude(segmentCrossTrackOffset + crosswindPxPerSec * kCrosswindInfluence * consumedSeconds,
                               kCrosswindLimit) * 0.35;
            if (activeSegmentIndex < waypointVisited.size())
                waypointVisited[activeSegmentIndex] = true;
            applyWaypointCommand(activeSegmentIndex);
            continue;
        }

        const double consumedSeconds = remainingTickTime;
        remainingTickTime = 0.0;
        segmentProgress += availableTravelDistance / segmentLength;
        segmentCrossTrackOffset = clampMagnitude(
            segmentCrossTrackOffset + crosswindPxPerSec * kCrosswindInfluence * consumedSeconds,
            kCrosswindLimit);

        const QPointF basePosition = start + segmentVector * segmentProgress;
        const QPointF windDrift = normal * segmentCrossTrackOffset;
        const QPointF visibleVelocity = direction * alongTrackSpeedPxPerSec + normal * (crosswindPxPerSec * 3.0);

        dronePosition = basePosition + windDrift;
        droneCourse = advanceHeading(droneCourse, pointCourse(visibleVelocity), kMaxTurnRateDegPerSec * consumedSeconds);
    }

    if (!qFuzzyCompare(droneCurrentHeight, droneTargetHeight)) {
        const double maxStep = kClimbRateMps * scaledDeltaSeconds;
        const double diff = droneTargetHeight - droneCurrentHeight;
        droneCurrentHeight += std::copysign(std::min(std::abs(diff), maxStep), diff);
        droneAltitudeScale = droneCurrentHeight / static_cast<double>(globals::defaultHeight);
    }

    update();
}

void CanvasWidget::startFlight()
{
    if (!hasRoute()) {
        return;
    }

    const CheckPoint startPoint = mission->path.getCheckPoint(0);
    const CheckPoint nextPoint = mission->path.getCheckPoint(1);

    activeSegmentIndex = 0;
    segmentProgress = 0.0;
    segmentCrossTrackOffset = 0.0;
    dronePosition = QPointF(startPoint.getPos().longitude, startPoint.getPos().latitude);
    droneCourse = calculateCourse(startPoint, nextPoint);
    droneVisible = true;
    flightInProgress = true;
    flightPaused = false;
    flightCompleted = false;
    lastFlightTickMs = 0;
    droneCurrentHeight = startPoint.getPos().altitude;
    droneTargetHeight  = droneCurrentHeight;
    droneAltitudeScale = droneCurrentHeight / static_cast<double>(globals::defaultHeight);
    waypointVisited = QVector<bool>(mission->path.size(), false);
    waypointVisited[0] = true;
    applyWaypointCommand(0);
    flightElapsedTimer.restart();
    flightTimer.start();

    emit flightPlaybackChanged(true);
    update();
}

void CanvasWidget::pauseFlight()
{
    if (!flightInProgress || flightPaused) {
        return;
    }

    flightPaused = true;
    flightTimer.stop();

    emit flightPlaybackChanged(false);
}

void CanvasWidget::resumeFlight()
{
    if (!flightInProgress || !flightPaused) {
        return;
    }

    flightPaused = false;
    lastFlightTickMs = 0;
    flightElapsedTimer.restart();
    flightTimer.start();

    emit flightPlaybackChanged(true);
}

void CanvasWidget::resetFlight()
{
    const bool stateChanged = flightInProgress || flightPaused || droneVisible;

    flightTimer.stop();
    lastFlightTickMs = 0;
    activeSegmentIndex = 0;
    segmentProgress = 0.0;
    segmentCrossTrackOffset = 0.0;
    droneVisible = false;
    flightInProgress = false;
    flightPaused = false;
    flightCompleted = false;
    droneCurrentHeight = globals::defaultHeight;
    droneTargetHeight  = globals::defaultHeight;
    droneAltitudeScale = 1.0;
    droneCurrentSpeed  = 0.0;
    waypointVisited.clear();

    if (stateChanged) {
        emit flightPlaybackChanged(false);
    }
}

void CanvasWidget::finishFlight()
{
    flightTimer.stop();
    lastFlightTickMs = 0;
    flightInProgress = false;
    flightPaused = false;
    flightCompleted = true;

    emit flightFinished();
    update();
}

void CanvasWidget::drawAltitudeLabel(QPainter &painter)
{
    if (!droneVisible)
        return;

    const QStringList lines = {
        QString("Высота:   %1 м").arg(qRound(droneCurrentHeight)),
        QString("Скорость: %1 км/ч").arg(droneCurrentSpeed, 0, 'f', 1),
        QString("Курс:     %1°").arg(qRound(std::fmod(360.0 - droneCourse, 360.0))),
    };

    painter.save();
    painter.resetTransform();

    QFont font = painter.font();
    font.setPixelSize(13);
    font.setBold(true);
    painter.setFont(font);

    const QFontMetrics fm(font);
    constexpr int margin    = 8;
    constexpr int padding   = 6;
    constexpr int lineSpacing = 3;

    int maxWidth = 0;
    for (const QString &line : lines)
        maxWidth = std::max(maxWidth, fm.horizontalAdvance(line));

    const int lineH   = fm.height();
    const int totalH  = lineH * lines.size() + lineSpacing * (lines.size() - 1);

    const QPoint visibleOrigin = visibleRegion().boundingRect().topLeft();
    const QRectF bgRect(visibleOrigin.x() + margin,
                        visibleOrigin.y() + margin,
                        maxWidth  + padding * 2,
                        totalH    + padding * 2);

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, 140));
    painter.drawRoundedRect(bgRect, 4, 4);

    painter.setPen(Qt::white);
    for (int i = 0; i < lines.size(); ++i) {
        const int y = visibleOrigin.y() + margin + padding + i * (lineH + lineSpacing) + fm.ascent();
        painter.drawText(QPointF(visibleOrigin.x() + margin + padding, y), lines[i]);
    }

    painter.restore();
}

void CanvasWidget::applyWaypointCommand(int waypointIndex)
{
    if (!mission || waypointIndex < 0 || waypointIndex >= mission->path.size())
        return;

    const CheckPoint& cp = mission->path.getCheckPoint(waypointIndex);
    const CmdEnum cmd = cp.getCmd().getAction();

    if (cmd == CmdEnum::ChangeAltitudeUp || cmd == CmdEnum::ChangeAltitudeDown) {
        constexpr double kAltitudeMin = 10.0;
        constexpr double kAltitudeMax = 200.0;
        droneTargetHeight = std::clamp(cp.getPos().altitude, kAltitudeMin, kAltitudeMax);
    }
}

int CanvasWidget::findWaypointNear(const QPointF &pos, double radius) const
{
    if (mission == nullptr) return -1;

    const auto& path = mission->path;

    for (int i = 0; i < path.size(); ++i) {
        const auto& p = path.getCheckPoint(i).getPos();
        QPointF wp(p.longitude, p.latitude);

        const double dist = std::hypot(wp.x() - pos.x(), wp.y() - pos.y());
        if (dist <= radius) {
            return i;
        }
    }

    return -1;
}

double CanvasWidget::resolveCruiseSpeed() const
{
    if (mission == nullptr) {
        return kFallbackCruiseSpeed;
    }

    const double boardSpeed =
        mission->board.speed > globals::EPS ? mission->board.speed : kFallbackCruiseSpeed;
    const double temperaturePenalty =
        std::clamp(std::abs(mission->env.temperature - 25.0) * 0.004, 0.0, 0.20);
    const double humidityPenalty =
        std::clamp(mission->env.humidity * 0.0015, 0.0, 0.15);
    const double envFactor =
        std::clamp(1.0 - temperaturePenalty - humidityPenalty, 0.65, 1.05);

    return std::max(kMinCruiseSpeed, boardSpeed * envFactor);
}

double CanvasWidget::telemetryNoiseLevel() const
{
    if (!mission) return 0.0;

    const Telemetry& t = mission->telemetry;

    double level = (20.0 - t.numSatellites) / 20.0;
    return std::clamp(level, 0.0, 1.0);
}

QPointF CanvasWidget::applyTelemetryNoise(const QPointF& pos) const
{
    if (!mission) return pos;

    const Telemetry& t = mission->telemetry;

    const bool perfectLink =
        (t.numSatellites >= 12 &&
         t.communicationFrequency >= 2.4 &&
         (t.modulation == globals::QPSK || t.modulation == globals::OQPSK));

    if (perfectLink)
        return pos;

    const double satError = std::max(0.0, 1.0 - t.numSatellites / 12.0);
    const double freqError = std::max(0.0, 1.0 - t.communicationFrequency / 2.4);

    double modulationError = 0.0;
    if (t.modulation == globals::BPSK)
        modulationError = 0.8;
    else if (t.modulation == globals::QPSK)
        modulationError = 0.3;
    else if (t.modulation == globals::OQPSK)
        modulationError = 0.2;

    double error = (satError * 0.5 +
                    freqError * 0.3 +
                    modulationError * 0.2);

    error = std::clamp(error, 0.0, 1.0);

    double maxDeviation = 8.0 * error;

    auto* rng = QRandomGenerator::global();

    double dx = (rng->generateDouble() * 2.0 - 1.0) * maxDeviation;
    double dy = (rng->generateDouble() * 2.0 - 1.0) * maxDeviation;

    return pos + QPointF(dx, dy);
}

bool CanvasWidget::shouldDropFrame() const
{
    if (!mission) return false;

    const Telemetry& t = mission->telemetry;

    double dropChance = 0.0;

    if (t.communicationFrequency < globals::minStableFrequencyMhz)
        dropChance = 0.25;
    else if (t.communicationFrequency < 2.0 * globals::minStableFrequencyMhz)
        dropChance = 0.1;

    if (t.modulation == globals::BPSK)
        dropChance *= globals::dropChanceFactor;

    return QRandomGenerator::global()->bounded(1.0) < dropChance;
}

void CanvasWidget::drawTelemetryNoise(QPainter &painter)
{
    if (!mission)
        return;

    const Telemetry& t = mission->telemetry;

    const double satFactor = std::clamp(t.numSatellites / 20.0, 0.0, 1.0);
    const double freqFactor = std::clamp(t.communicationFrequency / 5.8, 0.0, 1.0);

    double modulationFactor = 1.0;
    if (t.modulation == globals::QPSK || t.modulation == globals::OQPSK)
        modulationFactor = 0.9;
    else
        modulationFactor = 0.7;

    const double signalQuality =
        satFactor * 0.5 +
        freqFactor * 0.3 +
        modulationFactor * 0.2;

    const bool critical = (signalQuality < 0.25);
    const bool degraded = (signalQuality < 0.6);

    double baseNoise = telemetryNoiseLevel();
    double noise = baseNoise * (1.2 - signalQuality);

    if (signalQuality > 0.8)
        noise *= 0.2;

    noise = std::clamp(noise, 0.0, 1.0);

    painter.save();
    painter.resetTransform();

    const QRect rect = this->rect();
    auto* rng = QRandomGenerator::global();

    const bool digital =
        (t.modulation == globals::QPSK ||
         t.modulation == globals::OQPSK);

    const int pixelCount = static_cast<int>(noise * 600);

    if (digital)
    {
        double satNoise  = (1.0 - satFactor) * 0.7;
        double freqNoise = (1.0 - freqFactor) * 0.5;

        double blockChance = (satNoise + freqNoise) * 0.6;

        for (int i = 0; i < pixelCount; ++i)
        {
            int x = rng->bounded(rect.width());
            int y = rng->bounded(rect.height());
            int size = rng->bounded(1, 4);

            if (degraded && rng->bounded(1.0) < blockChance)
            {
                int base = (rng->bounded(3) == 0) ? 255 : 0;
                QColor color(base, base, base, 100 + noise * 120);
                painter.fillRect(x, y, size, size, color);
            }
        }

        if (critical && rng->bounded(1.0) < 0.4)
        {
            int y = rng->bounded(rect.height());
            painter.fillRect(0, y, rect.width(), 3, QColor(0, 0, 0, 180));
        }
    }

    else
    {
        for (int i = 0; i < pixelCount; ++i)
        {
            int x = rng->bounded(rect.width());
            int y = rng->bounded(rect.height());

            int gray = rng->bounded(0, 160);
            QColor color(gray, gray, gray, 70 + noise * 120);

            painter.setPen(color);
            painter.drawPoint(x, y);
        }

        if (critical && rng->bounded(1.0) < 0.25)
        {
            int y = rng->bounded(rect.height());
            painter.fillRect(0, y, rect.width(), 1, QColor(0, 0, 0, 120));
        }
    }

    painter.restore();
}
