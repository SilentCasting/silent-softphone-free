// SPDX-FileCopyrightText: 2026 Ihor Martseniuk and Silent Softphone Free contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QAbstractListModel>
#include <QList>

class ISipService;

// Audio codecs offered by the SIP stack, in preference order. Enabling, disabling
// and reordering is applied to the stack as priorities and persisted in QSettings.
class CodecModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Role { CodecIdRole = Qt::UserRole + 1, NameRole, RateRole, CodecEnabledRole };

    CodecModel(ISipService *sip, bool persistent, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void setEnabled(int row, bool enabled);
    Q_INVOKABLE void move(int from, int to);

private:
    struct Entry
    {
        QString id;
        bool enabled = true;
    };

    int findEntry(const QString &id) const;
    bool restoreSaved();
    void applyDefaults();
    void apply() const;
    void save() const;

    ISipService *m_sip;
    const bool m_persistent;
    QList<Entry> m_entries;
};
