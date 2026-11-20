#include <QApplication>
#include <QDateTime>
#include <QFrame>
#include <QHeaderView>
#include <QLabel>
#include <QMainWindow>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QTableWidget>
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QVector>

#include <cmath>

class PressureChart : public QWidget
{
public:
    explicit PressureChart(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        setMinimumHeight(260);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }

    void addSample(double actual, double predicted)
    {
        actualValues.append(actual);
        predictedValues.append(predicted);

        const int maxSamples = 40;
        if (actualValues.size() > maxSamples) {
            actualValues.removeFirst();
            predictedValues.removeFirst();
        }

        update();
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.fillRect(rect(), QColor("#111B24"));

        const QRectF plot(62, 34, width() - 88, height() - 76);
        if (plot.width() <= 0 || plot.height() <= 0) {
            return;
        }

        painter.setPen(QColor("#263641"));
        for (int value = 0; value <= 12; value += 2) {
            const qreal y = plot.bottom() - (value / 12.0) * plot.height();
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
                         "PRESSURE / BAR  -  SIMULATED TELEMETRY");

        const qreal limitY = plot.bottom() - (10.0 / 12.0) * plot.height();
        QPen limitPen(QColor("#F5B84B"), 1, Qt::DashLine);
        painter.setPen(limitPen);
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

private:
    void drawSeries(QPainter &painter,
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
            const qreal x = values.size() == 1
                                ? plot.right()
                                : plot.left() +
                                      (static_cast<qreal>(i) /
                                       (values.size() - 1)) * plot.width();

            const qreal pressure = qBound(0.0, values[i], 12.0);
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

    QVector<double> actualValues;
    QVector<double> predictedValues;
};

class DashboardWindow : public QMainWindow
{
public:
    DashboardWindow()
    {
        setWindowTitle("VOLTGUARD | Field Operations");
        resize(1240, 820);
        setMinimumSize(900, 650);

        setStyleSheet(
            "QMainWindow { background: #0B1118; color: #E6EEF2; }"
            "QLabel { color: #E6EEF2; }"
            "QTableWidget { background: #111B24; color: #E6EEF2;"
            " gridline-color: #263641; border: 1px solid #263641; }"
            "QHeaderView::section { background: #17232D; color: #91A7B2;"
            " padding: 8px; border: 0; }"
            "QPushButton { background: transparent; color: #A9BBC4;"
            " border: 0; text-align: left; padding: 10px; }"
            "QPushButton:hover { background: #1C2A35; color: #43D9E6; }"
        );

        buildInterface();

        timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, [this]() { updateDemo(); });
        timer->start(1100);
    }

private:
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

    static QFrame *makeMetricCard(const QString &title,
                                  const QString &accent,
                                  QLabel *&valueLabel)
    {
        auto *card = new QFrame;
        card->setStyleSheet(
            QString("QFrame { background: #111B24;"
                    " border: 1px solid #263641;"
                    " border-top: 2px solid %1; }").arg(accent));

        auto *layout = new QVBoxLayout(card);
        layout->setContentsMargins(14, 12, 14, 12);

        auto *titleLabel = new QLabel(title);
        titleLabel->setStyleSheet(
            "color: #91A7B2; font-size: 10px; font-weight: bold;");

        valueLabel = new QLabel("0");
        valueLabel->setStyleSheet(
            QString("color: %1; font-size: 25px; font-weight: bold;")
                .arg(accent));

        layout->addWidget(titleLabel);
        layout->addWidget(valueLabel);
        return card;
    }

    void buildInterface()
    {
        auto *central = new QWidget;
        auto *root = new QHBoxLayout(central);
        root->setContentsMargins(12, 12, 12, 12);
        root->setSpacing(12);

        auto *rail = new QFrame;
        rail->setFixedWidth(170);
        rail->setStyleSheet(
            "QFrame { background: #111B24; border: 1px solid #263641; }");
        auto *railLayout = new QVBoxLayout(rail);

        auto *brand = new QLabel("VOLT\nGUARD");
        brand->setStyleSheet(
            "color: #43D9E6; font-size: 22px; font-weight: bold;"
            " letter-spacing: 2px; padding: 12px;");
        railLayout->addWidget(brand);

        auto *demoTag = new QLabel("FIELD CONSOLE  /  DEMO");
        demoTag->setStyleSheet(
            "color: #F5B84B; font-size: 9px; padding: 8px;");
        railLayout->addWidget(demoTag);

        for (const QString &name :
             {"Overview", "Traffic ledger", "Alerts", "Telemetry"}) {
            auto *button = new QPushButton(name);
            railLayout->addWidget(button);
        }

        railLayout->addStretch();
        auto *railNote = new QLabel("NO LIVE CAPTURE\nSIMULATION ONLY");
        railNote->setStyleSheet(
            "color: #91A7B2; font-size: 9px; padding: 8px;");
        railLayout->addWidget(railNote);

        auto *content = new QWidget;
        auto *contentLayout = new QVBoxLayout(content);
        contentLayout->setContentsMargins(0, 0, 0, 0);
        contentLayout->setSpacing(12);

        auto *header = new QHBoxLayout;
        auto *heading = new QLabel("SYSTEM OVERVIEW");
        heading->setStyleSheet(
            "font-size: 20px; font-weight: bold; letter-spacing: 1px;");

        statusValue = new QLabel("●  SIMULATED");
        statusValue->setStyleSheet(
            "color: #F5B84B; font-weight: bold; padding: 8px;");

        clockValue = new QLabel;
        clockValue->setStyleSheet("color: #91A7B2; font-family: monospace;");

        header->addWidget(heading);
        header->addStretch();
        header->addWidget(statusValue);
        header->addWidget(clockValue);
        contentLayout->addLayout(header);

        auto *metrics = new QHBoxLayout;
        metrics->addWidget(makeMetricCard(
            "PACKETS INSPECTED", "#43D9E6", packetValue));
        metrics->addWidget(makeMetricCard(
            "ALLOWED", "#C9F27A", allowedValue));
        metrics->addWidget(makeMetricCard(
            "BLOCKED (DEMO)", "#FF6678", blockedValue));
        metrics->addWidget(makeMetricCard(
            "PREDICTED PEAK", "#F5B84B", pressureValue));
        contentLayout->addLayout(metrics);

        chart = new PressureChart;
        chart->setStyleSheet(
            "border: 1px solid #263641; background: #111B24;");
        contentLayout->addWidget(chart, 3);

        auto *eventsTitle = new QLabel("RECENT SIMULATED EVENTS");
        eventsTitle->setStyleSheet(
            "color: #91A7B2; font-size: 11px; font-weight: bold;");
        contentLayout->addWidget(eventsTitle);

        eventTable = new QTableWidget(0, 5);
        eventTable->setHorizontalHeaderLabels(
            {"TIME", "SOURCE", "FUNCTION", "VERDICT", "INSPECTION NOTE"});
        eventTable->horizontalHeader()->setSectionResizeMode(
            QHeaderView::Stretch);
        eventTable->verticalHeader()->hide();
        eventTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
        eventTable->setSelectionBehavior(QAbstractItemView::SelectRows);
        eventTable->setMinimumHeight(180);
        contentLayout->addWidget(eventTable, 2);

        auto *footer = new QLabel(
            "DEMO DATA ONLY  /  No live industrial network is connected");
        footer->setStyleSheet(
            "color: #91A7B2; font-size: 10px; padding: 4px;");
        contentLayout->addWidget(footer);

        root->addWidget(rail);
        root->addWidget(content, 1);
        setCentralWidget(central);
    }

    void updateDemo()
    {
        ++sampleCount;

        const double phase = sampleCount * 0.42;
        const double actual = 5.2 + std::sin(phase) * 1.1;
        const bool blocked = sampleCount % 8 == 0;
        const double predicted = blocked ? 10.7 : actual + 1.0;

        if (blocked) {
            ++blockedCount;
        } else {
            ++allowedCount;
        }

        chart->addSample(actual, predicted);

        packetValue->setText(QString::number(sampleCount));
        allowedValue->setText(QString::number(allowedCount));
        blockedValue->setText(QString::number(blockedCount));
        pressureValue->setText(
            QString::number(predicted, 'f', 1) + " bar");
        clockValue->setText(
            QDateTime::currentDateTime().toString("HH:mm:ss"));

        addEvent(blocked, actual, predicted);
    }

    void addEvent(bool blocked, double actual, double predicted)
    {
        eventTable->insertRow(0);

        const QString time =
            QDateTime::currentDateTime().toString("HH:mm:ss");
        const QString verdict = blocked ? "BLOCKED / DEMO" : "ALLOWED / DEMO";
        const QString reason = blocked
                                   ? "Predicted pressure exceeds 10 bar limit"
                                   : QString("Pressure within demo envelope (%1 bar)")
                                         .arg(actual, 0, 'f', 1);

        const QStringList values = {
            time,
            "192.0.2.10",
            "0x10  Write Multiple Registers",
            verdict,
            reason
        };

        const QColor verdictColor =
            blocked ? QColor("#FF6678") : QColor("#C9F27A");

        for (int column = 0; column < values.size(); ++column) {
            auto *item = new QTableWidgetItem(values[column]);
            if (column == 3) {
                item->setForeground(verdictColor);
            }
            eventTable->setItem(0, column, item);
        }

        if (eventTable->rowCount() > 30) {
            eventTable->removeRow(eventTable->rowCount() - 1);
        }

        Q_UNUSED(predicted);
    }
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    DashboardWindow window;
    window.show();

    return app.exec();
}