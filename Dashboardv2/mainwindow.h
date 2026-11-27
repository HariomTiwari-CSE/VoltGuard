#pragma once

#include <QDateTime>
#include <QMainWindow>
#include <QString>

class QLabel;
class QFrame;
class QTableView;
class QTimer;
class TrafficChart;
class TrafficLogModel;
class QUdpSocket;

class DashboardWindow : public QMainWindow
{
public:
    DashboardWindow();

private:
    static QFrame *makeMetricCard(const QString &title,
                                  const QString &accent,
                                  QLabel *&valueLabel);

    void buildInterface();
    void processPendingDatagrams();
    void updateDashboard();
    void updateStatus();

    QLabel *packetValue = nullptr;
    QLabel *completeValue = nullptr;
    QLabel *partialValue = nullptr;
    QLabel *reviewValue = nullptr;
    QLabel *clockValue = nullptr;
    QLabel *statusValue = nullptr;

    TrafficChart *chart = nullptr;
    TrafficLogModel *logModel = nullptr;
    QTableView *eventTable = nullptr;
    QUdpSocket *udpSocket = nullptr;
    QTimer *refreshTimer = nullptr;

    qint64 packetsObserved = 0;
    qint64 completeFrames = 0;
    qint64 partialFrames = 0;
    qint64 reviewFrames = 0;
    qint64 packetsSinceRefresh = 0;
    QDateTime lastMonitorMessage;
    QString bindError;
};
