#include "mainwindow.h"

#include <QAbstractItemView>
#include <QHeaderView>
#include <QHostAddress>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVBoxLayout>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      plotWidget(nullptr),
      logModel(nullptr),
      tableView(nullptr),
      udpSocket(new QUdpSocket(this))
{
    setupDarkTheme();
    setupUI();

    if (!udpSocket->bind(QHostAddress::LocalHost, 5556)) {
        qWarning() << "Could not listen for dashboard data on UDP port 5556:"
                   << udpSocket->errorString();
        return;
    }

    connect(udpSocket,
            &QUdpSocket::readyRead,
            this,
            &MainWindow::processPendingDatagrams);
}

MainWindow::~MainWindow() = default;

void MainWindow::setupDarkTheme()
{
    setWindowTitle("VoltGuard Dashboard");
    resize(1100, 750);

    setStyleSheet(
        "QMainWindow { background-color: #181B1F; color: #ECEFF1; }"
        "QTableView {"
        " background-color: #20242A;"
        " color: #ECEFF1;"
        " gridline-color: #343A40;"
        " selection-background-color: #263238;"
        "}"
        "QHeaderView::section {"
        " background-color: #263238;"
        " color: #ECEFF1;"
        " padding: 6px;"
        " border: 1px solid #343A40;"
        "}"
    );
}

void MainWindow::setupUI()
{
    QWidget *centralWidget = new QWidget(this);
    auto *layout = new QVBoxLayout(centralWidget);

    plotWidget = new PhysicsPlotWidget(centralWidget);
    logModel = new TrafficLogModel(this);
    tableView = new QTableView(centralWidget);

    tableView->setModel(logModel);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    layout->addWidget(plotWidget, 2);
    layout->addWidget(tableView, 1);

    setCentralWidget(centralWidget);
}

void MainWindow::processPendingDatagrams()
{
    while (udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(static_cast<int>(udpSocket->pendingDatagramSize()));

        if (udpSocket->readDatagram(datagram.data(), datagram.size()) < 0) {
            qWarning() << "Could not read dashboard datagram:"
                       << udpSocket->errorString();
            continue;
        }

        QJsonParseError parseError;
        const QJsonDocument document =
            QJsonDocument::fromJson(datagram, &parseError);

        if (parseError.error != QJsonParseError::NoError ||
            !document.isObject()) {
            qWarning() << "Ignoring invalid dashboard JSON:"
                       << parseError.errorString();
            continue;
        }

        const QJsonObject message = document.object();
        const QString type = message.value("type").toString();

        if (type == "packet_log") {
            logModel->appendLog(
                message.value("time").toString(),
                message.value("hex").toString(),
                message.value("verdict").toString(),
                message.value("reason").toString());
        } else if (type == "telemetry") {
            plotWidget->updateTelemetryPoints(
                message.value("time").toDouble(),
                message.value("p_actual").toDouble(),
                message.value("p_predicted").toDouble());
        }
    }
}