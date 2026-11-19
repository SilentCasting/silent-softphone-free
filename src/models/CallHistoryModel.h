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

// Newest-first call log, capped at the most recent records. Saved as JSON to storageFile
// after every change; an empty path keeps it in memory only (the mock backend's sandbox).
class CallHistoryModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Role { NameRole = Qt::UserRole + 1, NumberRole, KindRole, TimeTextRole, DurationTextRole };

    explicit CallHistoryModel(const QString &storageFile = {}, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void addRecord(const CallRecord &record);
    Q_INVOKABLE void clear();

private:
    void load();
    void save() const;

    const QString m_storageFile;
    QList<CallRecord> m_records;
};
