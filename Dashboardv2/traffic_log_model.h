#pragma once

#include <QAbstractTableModel>
#include <QString>
#include <QVector>

struct LogEntry {
    QString timestamp;
    QString source;
    QString function;
    QString verdict;
    QString hexPayload;
    QString reason;
};

class TrafficLogModel : public QAbstractTableModel
{
public:
    explicit TrafficLogModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index,
                  int role = Qt::DisplayRole) const override;
    QVariant headerData(int section,
                        Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    void appendLog(const QString &time,
                   const QString &source,
                   const QString &function,
                   const QString &verdict,
                   const QString &hex,
                   const QString &reason);

private:
    QVector<LogEntry> logs;
};
