#include "mainwindow.h"

#include <QColor>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QRectF>
#include <QSizePolicy>

PressureChart::PressureChart(QWidget *parent)
    : QWidget(parent)
{
    setMinimumHeight(260);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void PressureChart::addSample(double actual, double predicted)
{
    actualValues.append(actual);
    predictedValues.append(predicted);

    constexpr int maxSamples = 40;
    if (actualValues.size() > maxSamples) {
        actualValues.removeFirst();
        predictedValues.removeFirst();
    }

    update();
}

void PressureChart::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QColor("#111B24"));

    const QRectF plot(62, 34, width() - 88, height() - 76);
    if (plot.width() <= 0 || plot.height() <= 0) {
        return;
    }

    painter.setPen(QColor("#263641"));
    for (int value = 0; value <= 12; value += 2) {
        const qreal y =
            plot.bottom() - (static_cast<qreal>(value) / 12.0) * plot.height();

        painter.drawLine(QPointF(plot.left(), y), QPointF(plot.right(), y));

        painter.setPen(QColor("#8DA2AE"));
        painter.drawText(QRectF(8, y - 9, 44, 18),
                         Qt::AlignRight | Qt::AlignVCenter,
                         QString::number(value));
        painter.setPen(QColor("#263641"));
    }

    painter.setPen(QColor("#738791"));
    painter.drawText(QRectF(plot.left(), 6, plot.width(), 20),
                     Qt::AlignLeft | Qt::AlignVCenter,
                     "PRESSURE / BAR - SIMULATED TELEMETRY");

    const qreal limitY = plot.bottom() - (10.0 / 12.0) * plot.height();
    painter.setPen(QPen(QColor("#F5B84B"), 1, Qt::DashLine));
    painter.drawLine(QPointF(plot.left(), limitY),
                     QPointF(plot.right(), limitY));

    painter.setPen(QColor("#F5B84B"));
    painter.drawText(QPointF(plot.right() - 100, limitY - 6),
                     "10 BAR LIMIT");

    drawSeries(painter, plot, actualValues, QColor("#43D9E6"), false);
    drawSeries(painter, plot, predictedValues, QColor("#C9F27A"), true);

    painter.setPen(QColor("#43D9E6"));
    painter.drawText(QPointF(plot.left(), height() - 12), "— ACTUAL");

    painter.setPen(QColor("#C9F27A"));
    painter.drawText(QPointF(plot.left() + 112, height() - 12),
                     "– – PREDICTED");
}

void PressureChart::drawSeries(QPainter &painter,
                               const QRectF &plot,
                               const QVector<double> &values,
                               const QColor &color,
                               bool dashed)
{
    if (values.isEmpty()) {
        return;
    }

    QPainterPath path;
    for (int i = 0; i < values.size(); ++i) {
        const qreal x =
            values.size() == 1
                ? plot.right()
                : plot.left() +
                      (static_cast<qreal>(i) / (values.size() - 1)) *
                          plot.width();

        const qreal pressure = qBound(0.0, values.at(i), 12.0);
        const qreal y = plot.bottom() - (pressure / 12.0) * plot.height();

        if (i == 0) {
            path.moveTo(x, y);
        } else {
            path.lineTo(x, y);
        }
    }

    QPen pen(color, 2);
    if (dashed) {
        pen.setStyle(Qt::DashLine);
    }

    painter.setPen(pen);
    painter.drawPath(path);
}