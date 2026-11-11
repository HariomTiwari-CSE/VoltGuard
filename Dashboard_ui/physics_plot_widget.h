#pragma once

#include <QWidget>
#include <QChartView>
#include <QLineSeries>
#include <QValueAxis>
#include <QTimer>

class PhysicsPlotWidget : public QWidget {
    Q_OBJECT

public:
    explicit PhysicsPlotWidget(QWidget *parent = nullptr);
    void updateTelemetryPoints(double time, double actualP, double predictedP);

private slots:
    void renderSmoothTick();

private:
    QLineSeries *seriesActual;
    QLineSeries *seriesPredicted;
    QChart *chart;
    QChartView *chartView;
    QValueAxis *axisX;
    QValueAxis *axisY;
    QTimer *animTimer;

    double currentTime;
    double currentPressure;
    double targetPressure;
};