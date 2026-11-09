#include "traffic_log_model.h"
#include <QColor>
#include <QBrush>
#include <QFont>

TrafficLogModel::TrafficLogModel(QObject *parent) : QAbstractTableModel(parent) {}

int TrafficLogModel::rowCount(const QModelIndex &) const {
    return logs.size();
}

int TrafficLogModel::columnCount(const QModelIndex &) const {
    return 4;
}

QVariant TrafficLogModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= logs.size()) return QVariant();

    const LogEntry &entry = logs[index.row()];
    bool isDrop = entry.verdict.contains("DROP");

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case 0: return entry.timestamp;
            case 1: return entry.hexPayload;
            case 2: return entry.verdict;
            case 3: return entry.reason;
        }
    } 
    else if (role == Qt::ForegroundRole) {
        switch (index.column()) {
            case 0: return QBrush(QColor("#90CAF9")); // Cool Cyan-Grey Timestamp
            case 1: return QBrush(QColor("#CFD8DC")); // Monospace Hex Grey
            case 2: return isDrop ? QBrush(QColor("#FF5252")) : QBrush(QColor("#69F0AE")); // Bright Verdict
            case 3: return isDrop ? QBrush(QColor("#FF8A80")) : QBrush(QColor("#B9F6CA")); // Glowing Reason
        }
    }
    else if (role == Qt::FontRole) {
        if (index.column() == 2) {
            QFont f("Segoe UI", 9, QFont::Bold);
            return f;
        }
    }
    return QVariant();
}
QVariant TrafficLogModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal) return QVariant();

    switch (section) {
        case 0: return "Timestamp";
        case 1: return "Hex Payload";
        case 2: return "Verdict";
        case 3: return "Mitigation / Physics Reason";
        default: return QVariant();
    }
}
