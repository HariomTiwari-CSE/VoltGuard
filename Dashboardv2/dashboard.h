#pragma once

#include <QMainWindow>
#include <QVector>

class QColor;
class QLabel;
class QPainter;
class QPaintEvent;
class QRectF;
class QFrame;
class QTableWidget;
class QTimer;
class QWidget;

class PressureChart : public QWidget
{
public:
    explicit PressureChart(QWidget *parent = nullptr);
    void addSample(double actual, double predicted);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void drawSeries(QPainter &painter,
                    const QRectF &plot,
                    const QVector<double> &values,
                    const QColor &color,
                    bool dashed);

    QVector<double> actualValues;
    QVector<double> predictedValues;
};

class DashboardWindow : public QMainWindow
{
public:
    DashboardWindow();

private:
    static QFrame *makeMetricCard(const QString &title,
                                  const QString &accent,
                                  QLabel *&valueLabel);

    void buildInterface();
    void updateDemo();
    void addEvent(bool blocked, double actual, double predicted);

    QLabel *packetValue = nullptr;
    QLabel *allowedValue = nullptr;
    QLabel *blockedValue = nullptr;
    QLabel *pressureValue = nullptr;
    QLabel *clockValue = nullptr;
    QLabel *statusValue = nullptr;

    PressureChart *chart = nullptr;
    QTableWidget *eventTable = nullptr;
    QTimer *timer = nullptr;

    int sampleCount = 0;
    int allowedCount = 0;
    int blockedCount = 0;
};