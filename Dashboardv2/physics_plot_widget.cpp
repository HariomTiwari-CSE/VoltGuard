#include "physics_plot_widget.h"

#include <QColor>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QRectF>
#include <QSizePolicy>

#include <algorithm>
#include <cmath>

TrafficChart::TrafficChart(QWidget *parent)
    : QWidget(parent)
{
    setMinimumHeight(260);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void TrafficChart::addSample(double packetsPerSecond)
{
    packetRates.append(std::max(0.0, packetsPerSecond));

    constexpr int maxSamples = 60;
    if (packetRates.size() > maxSamples) {
        packetRates.removeFirst();
    }

    update();
}

void TrafficChart::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QColor("#111B24"));

    const QRectF plot(62, 38, width() - 88, height() - 82);
    if (plot.width() <= 0 || plot.height() <= 0) {
        return;
    }

    double peakRate = 5.0;
    for (double rate : packetRates) {
        peakRate = std::max(peakRate, rate);
    }
    const double axisMaximum = std::max(5.0, std::ceil(peakRate / 5.0) * 5.0);

    painter.setPen(QColor("#738791"));
    painter.drawText(QRectF(plot.left(), 8, plot.width(), 20),
                     Qt::AlignLeft | Qt::AlignVCenter,
                     "MODBUS/TCP PACKET RATE  /  PACKETS PER SECOND");

    painter.setPen(QColor("#263641"));
    for (int step = 0; step <= 4; ++step) {
        const double value = axisMaximum * step / 4.0;
        const qreal y =
            plot.bottom() - (value / axisMaximum) * plot.height();

        painter.drawLine(QPointF(plot.left(), y), QPointF(plot.right(), y));
        painter.setPen(QColor("#8DA2AE"));
        painter.drawText(QRectF(8, y - 9, 44, 18),
                         Qt::AlignRight | Qt::AlignVCenter,
                         QString::number(value, 'f', 0));
        painter.setPen(QColor("#263641"));
    }

    drawSeries(painter, plot);

    painter.setPen(QColor("#43D9E6"));
    painter.drawText(QPointF(plot.left(), height() - 12),
                     "— OBSERVED PACKETS / SECOND");
}

void TrafficChart::drawSeries(QPainter &painter, const QRectF &plot) const
{
    if (packetRates.isEmpty()) {
        painter.setPen(QColor("#738791"));
        painter.drawText(plot, Qt::AlignCenter,
                         "Waiting for packets on TCP port 502");
        return;
    }

    double peakRate = 5.0;
    for (double rate : packetRates) {
        peakRate = std::max(peakRate, rate);
    }
    const double axisMaximum = std::max(5.0, std::ceil(peakRate / 5.0) * 5.0);

    QPainterPath path;
    for (int i = 0; i < packetRates.size(); ++i) {
        const qreal x =
            packetRates.size() == 1
                ? plot.right()
                : plot.left() +
                      (static_cast<qreal>(i) / (packetRates.size() - 1)) *
                          plot.width();
        const qreal y =
            plot.bottom() - (packetRates.at(i) / axisMaximum) * plot.height();

        if (i == 0) {
            path.moveTo(x, y);
        } else {
            path.lineTo(x, y);
        }
    }

    painter.setPen(QPen(QColor("#43D9E6"), 2));
    painter.drawPath(path);
}
