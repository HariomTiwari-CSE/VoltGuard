#pragma once

#include <QVector>
#include <QWidget>

class QColor;
class QPainter;
class QPaintEvent;
class QRectF;

class TrafficChart : public QWidget
{
public:
    explicit TrafficChart(QWidget *parent = nullptr);
    void addSample(double packetsPerSecond);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void drawSeries(QPainter &painter, const QRectF &plot) const;

    QVector<double> packetRates;
};
