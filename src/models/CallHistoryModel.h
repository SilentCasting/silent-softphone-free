// SPDX-FileCopyrightText: 2026 Ihor Martseniuk and Silent Softphone Free contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QAbstractListModel>
#include <QDateTime>
#include <QList>

struct CallRecord
{
    enum class Kind { Incoming, Outgoing, Missed };

    QString name;
    QString number;
    Kind kind = Kind::Outgoing;
    QDateTime startedAt;
    int durationSec = 0;
};

// Newest-first call log.
// TODO(stage 3): persist to SQLite.
class CallHistoryModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Role { NameRole = Qt::UserRole + 1, NumberRole, KindRole, TimeTextRole, DurationTextRole };

    explicit CallHistoryModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void addRecord(const CallRecord &record);
    Q_INVOKABLE void clear();

private:
    QList<CallRecord> m_records;
};
