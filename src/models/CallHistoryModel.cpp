// SPDX-FileCopyrightText: 2026 Ihor Martseniuk and Silent Softphone Free contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "models/CallHistoryModel.h"

#include "core/Format.h"

#include <QLocale>

namespace {

QString kindName(CallRecord::Kind kind)
{
    switch (kind) {
    case CallRecord::Kind::Incoming: return QStringLiteral("incoming");
    case CallRecord::Kind::Outgoing: return QStringLiteral("outgoing");
    case CallRecord::Kind::Missed:   return QStringLiteral("missed");
    }
    return {};
}

QString timeText(const QDateTime &when)
{
    const QLocale locale;
    if (when.date() == QDate::currentDate())
        return locale.toString(when.time(), QLocale::ShortFormat);
    return locale.toString(when, QLocale::ShortFormat);
}

} // namespace

CallHistoryModel::CallHistoryModel(QObject *parent)
    : QAbstractListModel(parent)
{
    const QDateTime now = QDateTime::currentDateTime();
    m_records = {
        { QStringLiteral("Olena Kovalenko"), QStringLiteral("1002"), CallRecord::Kind::Incoming, now.addSecs(-25 * 60), 184 },
        { QString(), QStringLiteral("+380441234567"), CallRecord::Kind::Missed, now.addSecs(-3 * 3600), 0 },
        { QStringLiteral("Support Line"), QStringLiteral("700"), CallRecord::Kind::Outgoing, now.addDays(-1), 452 },
    };
}

int CallHistoryModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : int(m_records.size());
}

QVariant CallHistoryModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index, CheckIndexOption::IndexIsValid | CheckIndexOption::ParentIsInvalid))
        return {};

    const CallRecord &record = m_records.at(index.row());
    switch (role) {
    case Qt::DisplayRole:
    case NameRole:         return record.name;
    case NumberRole:       return record.number;
    case KindRole:         return kindName(record.kind);
    case TimeTextRole:     return timeText(record.startedAt);
    case DurationTextRole: return record.kind == CallRecord::Kind::Missed ? QString() : formatDuration(record.durationSec);
    }
    return {};
}

QHash<int, QByteArray> CallHistoryModel::roleNames() const
{
    return {
        { NameRole, "name" },
        { NumberRole, "number" },
        { KindRole, "kind" },
        { TimeTextRole, "timeText" },
        { DurationTextRole, "durationText" },
    };
}

void CallHistoryModel::addRecord(const CallRecord &record)
{
    beginInsertRows({}, 0, 0);
    m_records.prepend(record);
    endInsertRows();
}

void CallHistoryModel::clear()
{
    if (m_records.isEmpty())
        return;
    beginResetModel();
    m_records.clear();
    endResetModel();
}
