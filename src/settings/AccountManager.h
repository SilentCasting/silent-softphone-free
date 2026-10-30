// SPDX-FileCopyrightText: 2026 Ihor Martseniuk and Silent Softphone Free contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/SipAccount.h"

#include <QAbstractListModel>
#include <QList>
#include <QVariantMap>

// The list of SIP accounts and which one is active. Only the active account is
// registered; switching accounts re-registers (PhoneController listens to
// activeAccountChanged). Persisted in QSettings.
// TODO(stage 2): move passwords to Keychain / Windows Credential Manager.
class AccountManager : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(int currentIndex READ currentIndex WRITE activate NOTIFY currentChanged)
    Q_PROPERTY(QString displayName READ displayName NOTIFY currentChanged)
    Q_PROPERTY(QString uri READ uri NOTIFY currentChanged)
    Q_PROPERTY(bool valid READ isValid NOTIFY currentChanged)

public:
    enum class Storage {
        Persistent, // the user's accounts from QSettings
        Demo        // one in-memory demo account (mock backend), nothing is saved
    };

    enum Role { LabelRole = Qt::UserRole + 1, UriRole };

    explicit AccountManager(Storage storage, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    int count() const { return int(m_accounts.size()); }
    int currentIndex() const { return m_current; }
    SipAccount current() const;
    QString displayName() const;
    QString uri() const { return current().uri(); }
    bool isValid() const { return current().isValid(); }

    // { displayName, username, domain, password, transport } for the settings form.
    Q_INVOKABLE QVariantMap get(int index) const;
    // Appends an empty account and returns its index.
    Q_INVOKABLE int add();
    Q_INVOKABLE void update(int index, const QString &displayName, const QString &username,
                            const QString &domain, const QString &password, int transport);
    Q_INVOKABLE void remove(int index);
    Q_INVOKABLE void activate(int index);

signals:
    void countChanged();
    void currentChanged();
    // The active account switched or its settings changed: time to re-register.
    void activeAccountChanged();

private:
    static QString label(const SipAccount &account);
    void load();
    void save() const;

    const bool m_persistent;
    QList<SipAccount> m_accounts;
    int m_current = -1;
};
