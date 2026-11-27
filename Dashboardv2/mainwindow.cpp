#include "mainwindow.h"

#include "physics_plot_widget.h"
#include "traffic_log_model.h"

#include <QAbstractItemView>
#include <QDateTime>
#include <QFrame>
#include <QHeaderView>
#include <QHostAddress>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QPushButton>
#include <QTableView>
#include <QTimer>
#include <QUdpSocket>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QVariant>

namespace {
constexpr quint16 DashboardPort = 5556;
constexpr qint64 MonitorTimeoutMs = 5000;
}

DashboardWindow::DashboardWindow()
    : udpSocket(new QUdpSocket(this)),
      refreshTimer(new QTimer(this))
{
    setWindowTitle("VOLTGUARD | Passive Field Monitor");
    resize(1240, 820);
    setMinimumSize(900, 650);

    setStyleSheet(
        "QMainWindow { background: #0B1118; color: #E6EEF2; }"
        "QLabel { color: #E6EEF2; }"
        "QTableView { background: #111B24; color: #E6EEF2;"
        " gridline-color: #263641; border: 1px solid #263641; }"
        "QHeaderView::section { background: #17232D; color: #91A7B2;"
        " padding: 8px; border: 0; }"
        "QPushButton { background: transparent; color: #A9BBC4;"
        " border: 0; text-align: left; padding: 10px; }"
        "QPushButton:hover { background: #1C2A35; color: #43D9E6; }"
    );

    buildInterface();

    if (!udpSocket->bind(QHostAddress::LocalHost, DashboardPort)) {
        bindError = udpSocket->errorString();
    } else {
        connect(udpSocket, &QUdpSocket::readyRead, this, [this]() {
            processPendingDatagrams();
        });
    }

    connect(refreshTimer, &QTimer::timeout, this, [this]() {
        updateDashboard();
    });
    refreshTimer->start(1000);
    updateDashboard();
}

QFrame *DashboardWindow::makeMetricCard(const QString &title,
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

    auto *titleLabel = new QLabel(title, card);
    titleLabel->setStyleSheet(
        "color: #91A7B2; font-size: 10px; font-weight: bold;");

    valueLabel = new QLabel("0", card);
    valueLabel->setStyleSheet(
        QString("color: %1; font-size: 25px; font-weight: bold;")
            .arg(accent));

    layout->addWidget(titleLabel);
    layout->addWidget(valueLabel);
    return card;
}

void DashboardWindow::buildInterface()
{
    auto *central = new QWidget(this);
    auto *root = new QHBoxLayout(central);
    root->setContentsMargins(12, 12, 12, 12);
    root->setSpacing(12);

    auto *rail = new QFrame(central);
    rail->setFixedWidth(190);
    rail->setStyleSheet(
        "QFrame { background: #111B24; border: 1px solid #263641; }");
    auto *railLayout = new QVBoxLayout(rail);

    auto *brand = new QLabel("VOLT\nGUARD", rail);
    brand->setStyleSheet(
        "color: #43D9E6; font-size: 22px; font-weight: bold;"
        " letter-spacing: 2px; padding: 12px;");
    railLayout->addWidget(brand);

    auto *modeLabel = new QLabel("PASSIVE OBSERVER", rail);
    modeLabel->setStyleSheet(
        "color: #43D9E6; font-size: 10px; font-weight: bold; padding: 8px;");
    railLayout->addWidget(modeLabel);
    railLayout->addWidget(new QLabel("Modbus/TCP · port 502", rail));
    railLayout->addWidget(new QLabel("Loopback feed · UDP 5556", rail));

    auto *instructions = new QLabel(
        "Start the capture helper in a separate terminal. "
        "Capture requires Npcap and may require administrator privileges.",
        rail);
    instructions->setWordWrap(true);
    instructions->setStyleSheet(
        "color: #91A7B2; font-size: 10px; padding: 8px;");
    railLayout->addWidget(instructions);
    railLayout->addStretch();

    auto *readOnlyNote = new QLabel(
        "READ ONLY\nNo packet injection\nNo blocking or enforcement",
        rail);
    readOnlyNote->setStyleSheet(
        "color: #F5B84B; font-size: 10px; padding: 8px;");
    railLayout->addWidget(readOnlyNote);

    auto *content = new QWidget(central);
    auto *contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(12);

    auto *header = new QHBoxLayout;
    auto *heading = new QLabel("LIVE TRAFFIC OVERVIEW", content);
    heading->setStyleSheet(
        "font-size: 20px; font-weight: bold; letter-spacing: 1px;");

    statusValue = new QLabel("● WAITING FOR MONITOR", content);
    statusValue->setStyleSheet(
        "color: #F5B84B; font-weight: bold; padding: 8px;");
    clockValue = new QLabel(content);
    clockValue->setStyleSheet("color: #91A7B2; font-family: monospace;");

    header->addWidget(heading);
    header->addStretch();
    header->addWidget(statusValue);
    header->addWidget(clockValue);
    contentLayout->addLayout(header);

    auto *metrics = new QHBoxLayout;
    metrics->addWidget(makeMetricCard(
        "TCP/502 PACKETS OBSERVED", "#43D9E6", packetValue));
    metrics->addWidget(makeMetricCard(
        "COMPLETE ADUs", "#C9F27A", completeValue));
    metrics->addWidget(makeMetricCard(
        "PARTIAL SEGMENTS", "#F5B84B", partialValue));
    metrics->addWidget(makeMetricCard(
        "REVIEW REQUIRED", "#FF6678", reviewValue));
    contentLayout->addLayout(metrics);

    chart = new TrafficChart(content);
    chart->setStyleSheet(
        "border: 1px solid #263641; background: #111B24;");
    contentLayout->addWidget(chart, 3);

    auto *eventsTitle = new QLabel("RECENT OBSERVED TRAFFIC", content);
    eventsTitle->setStyleSheet(
        "color: #91A7B2; font-size: 11px; font-weight: bold;");
    contentLayout->addWidget(eventsTitle);

    logModel = new TrafficLogModel(this);
    eventTable = new QTableView(content);
    eventTable->setModel(logModel);
    eventTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    eventTable->horizontalHeader()->setStretchLastSection(true);
    eventTable->verticalHeader()->hide();
    eventTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    eventTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    eventTable->setAlternatingRowColors(true);
    eventTable->setMinimumHeight(180);
    contentLayout->addWidget(eventTable, 2);

    auto *footer = new QLabel(
        "PASSIVE CAPTURE ONLY  /  Observed TCP packets are not blocked or modified",
        content);
    footer->setStyleSheet(
        "color: #91A7B2; font-size: 10px; padding: 4px;");
    contentLayout->addWidget(footer);

    root->addWidget(rail);
    root->addWidget(content, 1);
    setCentralWidget(central);
}

void DashboardWindow::processPendingDatagrams()
{
    while (udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(static_cast<int>(udpSocket->pendingDatagramSize()));

        if (udpSocket->readDatagram(datagram.data(), datagram.size()) < 0) {
            statusValue->setText("● UDP READ ERROR");
            statusValue->setToolTip(udpSocket->errorString());
            continue;
        }

        QJsonParseError parseError;
        const QJsonDocument document =
            QJsonDocument::fromJson(datagram, &parseError);
        if (parseError.error != QJsonParseError::NoError ||
            !document.isObject()) {
            continue;
        }

        const QJsonObject message = document.object();
        const QString type = message.value("type").toString();
        lastMonitorMessage = QDateTime::currentDateTime();

        if (type == "status") {
            const QString interfaceName =
                message.value("interface").toString("default");
            statusValue->setText("● MONITOR CONNECTED");
            statusValue->setToolTip(
                QString("Passive capture interface: %1").arg(interfaceName));
            continue;
        }

        if (type != "packet_log") {
            continue;
        }

        ++packetsObserved;
        ++packetsSinceRefresh;

        const QString frameStatus = message.value("verdict").toString();
        if (frameStatus == "COMPLETE") {
            ++completeFrames;
        } else if (frameStatus == "PARTIAL") {
            ++partialFrames;
        } else {
            ++reviewFrames;
        }

        const QString timestamp = message.value("time").toString();
        const QString source = message.value("source").toString();
        const QString function = message.value("function").toString();
        const QString hex = message.value("hex").toString();
        const QString reason = message.value("reason").toString();
        logModel->appendLog(timestamp, source, function,
                            frameStatus, hex, reason);
    }
}

void DashboardWindow::updateDashboard()
{
    packetValue->setText(QString::number(packetsObserved));
    completeValue->setText(QString::number(completeFrames));
    partialValue->setText(QString::number(partialFrames));
    reviewValue->setText(QString::number(reviewFrames));
    clockValue->setText(QDateTime::currentDateTime().toString("HH:mm:ss"));

    chart->addSample(static_cast<double>(packetsSinceRefresh));
    packetsSinceRefresh = 0;
    updateStatus();
}

void DashboardWindow::updateStatus()
{
    if (!bindError.isEmpty()) {
        statusValue->setText("● LISTENER ERROR");
        statusValue->setToolTip(
            QString("Could not bind UDP port %1: %2")
                .arg(DashboardPort)
                .arg(bindError));
        return;
    }

    if (!lastMonitorMessage.isValid()) {
        statusValue->setText("● WAITING FOR MONITOR");
        statusValue->setToolTip(
            "Run live_modbus_monitor.py to start passive packet observation.");
        return;
    }

    const qint64 ageMs =
        lastMonitorMessage.msecsTo(QDateTime::currentDateTime());
    if (ageMs > MonitorTimeoutMs) {
        statusValue->setText("● MONITOR OFFLINE");
        statusValue->setToolTip("No monitor message received in the last 5 seconds.");
        return;
    }

    statusValue->setText("● MONITOR CONNECTED");
}
