// SPDX-FileCopyrightText: 2026 Ihor Martseniuk and Silent Softphone Free contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "models/CallHistoryModel.h"

#include "core/Format.h"

#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocale>
#include <QSaveFile>

namespace {

constexpr int kMaxRecords = 500;

QString kindName(CallRecord::Kind kind)
{
    switch (kind) {
    case CallRecord::Kind::Incoming: return QStringLiteral("incoming");
    case CallRecord::Kind::Outgoing: return QStringLiteral("outgoing");
    case CallRecord::Kind::Missed:   return QStringLiteral("missed");
    }
    return {};
}

CallRecord::Kind kindFromName(const QString &name)
{
    if (name == QLatin1String("incoming"))
        return CallRecord::Kind::Incoming;
    if (name == QLatin1String("missed"))
        return CallRecord::Kind::Missed;
    return CallRecord::Kind::Outgoing;
}

QString timeText(const QDateTime &when)
{
    const QLocale locale;
    if (when.date() == QDate::currentDate())
        return locale.toString(when.time(), QLocale::ShortFormat);
    return locale.toString(when, QLocale::ShortFormat);
}

} // namespace

CallHistoryModel::CallHistoryModel(const QString &storageFile, QObject *parent)
    : QAbstractListModel(parent)
    , m_storageFile(storageFile)
{
    load();
}

void CallHistoryModel::load()
{
    if (m_storageFile.isEmpty())
        return;
    QFile file(m_storageFile);
    if (!file.open(QIODevice::ReadOnly))
        return; // first run: no history yet
    const QJsonArray items = QJsonDocument::fromJson(file.readAll()).array();
    for (const QJsonValue &item : items) {
        const QJsonObject object = item.toObject();
        CallRecord record;
        record.name = object.value(QLatin1String("name")).toString();
        record.number = object.value(QLatin1String("number")).toString();
        record.kind = kindFromName(object.value(QLatin1String("kind")).toString());
        record.startedAt = QDateTime::fromString(object.value(QLatin1String("startedAt")).toString(), Qt::ISODate);
        record.durationSec = object.value(QLatin1String("duration")).toInt();
        if (!record.number.isEmpty() && m_records.size() < kMaxRecords)
            m_records.append(record);
    }
}

void CallHistoryModel::save() const
{
    if (m_storageFile.isEmpty())
        return;
    QJsonArray items;
    for (const CallRecord &record : m_records) {
        items.append(QJsonObject{
            { QStringLiteral("name"), record.name },
            { QStringLiteral("number"), record.number },
            { QStringLiteral("kind"), kindName(record.kind) },
            { QStringLiteral("startedAt"), record.startedAt.toString(Qt::ISODate) },
            { QStringLiteral("duration"), record.durationSec },
        });
    }
    QSaveFile file(m_storageFile);
    if (!file.open(QIODevice::WriteOnly) || file.write(QJsonDocument(items).toJson()) < 0 || !file.commit())
        qWarning() << "could not save call history to" << m_storageFile;
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
    if (m_records.size() > kMaxRecords) {
        beginRemoveRows({}, kMaxRecords, int(m_records.size()) - 1);
        m_records.resize(kMaxRecords);
        endRemoveRows();
    }
    save();
}

void CallHistoryModel::clear()
{
    if (m_records.isEmpty())
        return;
    beginResetModel();
    m_records.clear();
    endResetModel();
    save();
}
