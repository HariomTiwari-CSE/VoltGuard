#pragma once

#include <QMainWindow>
#include <QTableView>
#include <QUdpSocket>
#include "physics_plot_widget.h"
#include "traffic_log_model.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void processPendingDatagrams();

private:
    void setupDarkTheme();
    void setupUI();

    PhysicsPlotWidget *plotWidget;
    TrafficLogModel *logModel;
    QTableView *tableView;
    QUdpSocket *udpSocket;
};
