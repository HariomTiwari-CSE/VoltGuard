#pragma once

#include <QAbstractTableModel>
#include <QVector>
#include <QString>

struct LogEntry {
    QString timestamp;
    QString hexPayload;
    QString verdict;
    QString reason;
};

class TrafficLogModel : public QAbstractTableModel {
    Q_OBJECT

public:
    explicit TrafficLogModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void appendLog(const QString &time, const QString &hex, const QString &verdict, const QString &reason);

private:
    QVector<LogEntry> logs;
};
