#ifndef CANVASWIDGET_H
#define CANVASWIDGET_H

#include "mission.h"
#include "globals.h"

#include <QWidget>
#include <QVector>
#include <QPointF>
#include <QPixmap>
#include <QTimer>
#include <QElapsedTimer>
#include <QMap>

class QPainter;
class QMouseEvent;
class QPaintEvent;
class QWheelEvent;

class CanvasWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CanvasWidget(QWidget *parent = nullptr);

    void clearWaypoints();

    void setMission(Mission& mission);
    bool setAreaImage(const QString &filePath);
    void setZoomFactor(double factor);
    double zoomFactor() const;
    void setSimulationSpeedFactor(double factor);
    double simulationSpeedFactor() const;
    bool toggleFlightPlayback();
    bool isFlightPlaying() const;
    bool hasRoute() const;
    QSize sizeHint() const override;

signals:
    void waypointAdded(const QPointF &pos);
    void waypointRemoved(int index);
    void flightPlaybackChanged(bool isPlaying);
    void flightFinished();
    void panRequested(QPoint delta);
    void zoomStepRequested(int steps);
    void pointClicked(int idx);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    void advanceFlight();
    void drawPath(QPainter &painter);
    void drawWaypoints(QPainter &painter);
    void drawDrone(QPainter &painter);
    void drawAltitudeLabel(QPainter &painter);
    void drawTelemetryNoise(QPainter &painter);

    QSize logicalCanvasSize() const;
    void updateCanvasSize();
    double overlaySize(double pixels, double maxBoost = 1.0) const;

    void startFlight();
    void pauseFlight();
    void resumeFlight();
    void resetFlight();
    void finishFlight();

    int findWaypointNear(const QPointF& pos, double radius) const;
    double resolveCruiseSpeed() const;
    void applyWaypointCommand(int waypointIndex);

    QPointF applyTelemetryNoise(const QPointF& pos) const;
    bool shouldDropFrame() const;
    double telemetryNoiseLevel() const;

private:
    Mission *mission = nullptr;

    QString pixmapPath = globals::unresolvedPath;
    QPixmap areaPixmap;
    QPixmap sprite;
    QMap<CmdEnum, QPixmap> actionSprites;

    QVector<bool> waypointVisited;

    QTimer flightTimer;
    QElapsedTimer flightElapsedTimer;
    qint64 lastFlightTickMs = 0;

    QPointF dronePosition;
    double droneCourse = 0.0;

    int activeSegmentIndex = 0;
    double segmentProgress = 0.0;
    double segmentCrossTrackOffset = 0.0;

    bool droneVisible = false;
    bool flightInProgress = false;
    bool flightPaused = false;
    bool flightCompleted = false;

    double currentZoomFactor = 1.0;
    double currentSimulationSpeedFactor = 1.0;

    double droneCurrentHeight = 50.0;
    double droneTargetHeight  = 50.0;
    double droneAltitudeScale = 1.0;
    double droneCurrentSpeed  = 0.0;

    bool rightDragging = false;
    bool rightDragMoved = false;
    QPoint rightDragLastPos;

    static constexpr int kDefaultCanvasSize = 500;
    static constexpr int kWpRadius = 6;
};

#endif // CANVASWIDGET_H
