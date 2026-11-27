#include "traffic_log_model.h"

#include <QBrush>
#include <QColor>
#include <QFont>

TrafficLogModel::TrafficLogModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

int TrafficLogModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : logs.size();
}

int TrafficLogModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : 6;
}

QVariant TrafficLogModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() ||
        index.row() < 0 ||
        index.row() >= logs.size() ||
        index.column() < 0 ||
        index.column() >= columnCount(index.parent())) {
        return {};
    }

    const LogEntry &entry = logs.at(index.row());

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case 0: return entry.timestamp;
        case 1: return entry.source;
        case 2: return entry.function;
        case 3: return entry.verdict;
        case 4: return entry.hexPayload;
        case 5: return entry.reason;
        default: return {};
        }
    }

    if (role == Qt::ForegroundRole) {
        const QColor statusColor =
            entry.verdict == "COMPLETE"
                ? QColor("#C9F27A")
                : entry.verdict == "PARTIAL"
                      ? QColor("#F5B84B")
                      : QColor("#FF6678");

        switch (index.column()) {
        case 0:
        case 1: return QBrush(QColor("#90CAF9"));
        case 2:
        case 4: return QBrush(QColor("#CFD8DC"));
        case 3: return QBrush(statusColor);
        case 5: return QBrush(statusColor);
        default: return {};
        }
    }

    if (role == Qt::FontRole &&
        (index.column() == 2 || index.column() == 4)) {
        return QFont("Segoe UI", 9, QFont::Bold);
    }

    return {};
}

QVariant TrafficLogModel::headerData(int section,
                                     Qt::Orientation orientation,
                                     int role) const
{
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal) {
        return {};
    }

    switch (section) {
    case 0: return "Timestamp";
    case 1: return "Source";
    case 2: return "Function";
    case 3: return "Frame Status";
    case 4: return "Payload (hex)";
    case 5: return "Inspection Note";
    default: return {};
    }
}

void TrafficLogModel::appendLog(const QString &time,
                                const QString &source,
                                const QString &function,
                                const QString &verdict,
                                const QString &hex,
                                const QString &reason)
{
    const int row = logs.size();
    beginInsertRows(QModelIndex(), row, row);
    logs.append({time, source, function, verdict, hex, reason});
    endInsertRows();

    constexpr int maxRows = 300;
    if (logs.size() > maxRows) {
        beginRemoveRows(QModelIndex(), 0, 0);
        logs.removeFirst();
        endRemoveRows();
    }
}
