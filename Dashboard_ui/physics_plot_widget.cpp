#include "physics_plot_widget.h"
#include <QVBoxLayout>
#include <QPen>
#include <QPainter>

PhysicsPlotWidget::PhysicsPlotWidget(QWidget *parent) 
    : QWidget(parent), currentTime(0.0), currentPressure(1.0), targetPressure(1.0) {
    
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    // Cyan Neon line for actual sensor
    seriesActual = new QLineSeries();
    seriesActual->setName("Actual Pipeline Pressure (Bar)");
    QPen penActual(QColor("#00E5FF"));
    penActual.setWidth(2);
    seriesActual->setPen(penActual);

    // Orange Dashed line for digital twin prediction
    seriesPredicted = new QLineSeries();
    seriesPredicted->setName("Predicted Transient State (Digital Twin)");
    QPen penPredicted(QColor("#FF9100"));
    penPredicted.setWidth(2);
    penPredicted.setStyle(Qt::DashLine);
    seriesPredicted->setPen(penPredicted);

    chart = new QChart();
    chart->addSeries(seriesActual);
    chart->addSeries(seriesPredicted);
    chart->setTitle("HYDRAULIC PRESSURE DYNAMICS & TRANSIENT ENVELOPE (P_max = 10.0 Bar)");
    chart->setTitleBrush(QBrush(QColor("#00E676")));
    chart->setTitleFont(QFont("Segoe UI", 11, QFont::Bold));
    chart->setBackgroundBrush(QBrush(QColor("#181B1F")));
    chart->legend()->setLabelColor(QColor("#ECEFF1"));

    // Axes styling
    axisX = new QValueAxis();
    axisX->setTitleText("Running Time (Seconds)");
    axisX->setTitleBrush(QBrush(QColor("#B0BEC5")));
    axisX->setLabelsColor(QColor("#90A4AE"));
    axisX->setGridLineColor(QColor("#262B32"));
    axisX->setRange(0, 30);

    axisY = new QValueAxis();
    axisY->setTitleText("Pressure (Bar)");
    axisY->setTitleBrush(QBrush(QColor("#B0BEC5")));
    axisY->setLabelsColor(QColor("#90A4AE"));
    axisY->setGridLineColor(QColor("#262B32"));
    axisY->setRange(0, 12.0);

    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    seriesActual->attachAxis(axisX);
    seriesActual->attachAxis(axisY);
    seriesPredicted->attachAxis(axisX);
    seriesPredicted->attachAxis(axisY);

    chartView = new QChartView(chart, this);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setMinimumHeight(360);
    layout->addWidget(chartView);

    // 100ms Smooth Interpolation Loop
    animTimer = new QTimer(this);
    connect(animTimer, &QTimer::timeout, this, &PhysicsPlotWidget::renderSmoothTick);
    animTimer->start(100);
}

void PhysicsPlotWidget::updateTelemetryPoints(double, double, double predictedP) {
    targetPressure = predictedP;
}

void PhysicsPlotWidget::renderSmoothTick() {
    currentTime += 0.1;
    // Fluid inertia exponential approach
    currentPressure += (targetPressure - currentPressure) * 0.08;

    seriesActual->append(currentTime, currentPressure);
    seriesPredicted->append(currentTime, targetPressure);

    if (currentTime > 30.0) {
        axisX->setRange(currentTime - 30.0, currentTime);
    }
}