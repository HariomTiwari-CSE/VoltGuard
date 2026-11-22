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
    return parent.isValid() ? 0 : 4;
}

QVariant TrafficLogModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() ||
        index.row() < 0 ||
        index.row() >= logs.size() ||
        index.column() < 0 ||
        index.column() >= columnCount()) {
        return {};
    }

    const LogEntry &entry = logs.at(index.row());

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case 0: return entry.timestamp;
        case 1: return entry.hexPayload;
        case 2: return entry.verdict;
        case 3: return entry.reason;
        default: return {};
        }
    }

    if (role == Qt::ForegroundRole) {
        const bool blocked =
            entry.verdict.contains("DROP", Qt::CaseInsensitive) ||
            entry.verdict.contains("BLOCK", Qt::CaseInsensitive);

        switch (index.column()) {
        case 0: return QBrush(QColor("#90CAF9"));
        case 1: return QBrush(QColor("#CFD8DC"));
        case 2:
            return QBrush(blocked ? QColor("#FF6678") : QColor("#C9F27A"));
        case 3:
            return QBrush(blocked ? QColor("#FF8A80") : QColor("#B9F6CA"));
        default: return {};
        }
    }

    if (role == Qt::FontRole && index.column() == 2) {
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
    case 1: return "Hex Payload";
    case 2: return "Verdict";
    case 3: return "Inspection Note";
    default: return {};
    }
}

void TrafficLogModel::appendLog(const QString &time,
                                const QString &hex,
                                const QString &verdict,
                                const QString &reason)
{
    const int row = logs.size();
    beginInsertRows(QModelIndex(), row, row);

    LogEntry entry;
    entry.timestamp = time;
    entry.hexPayload = hex;
    entry.verdict = verdict;
    entry.reason = reason;
    logs.append(entry);

    endInsertRows();
}