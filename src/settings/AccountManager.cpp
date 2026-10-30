// SPDX-FileCopyrightText: 2026 Ihor Martseniuk and Silent Softphone Free contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "settings/AccountManager.h"

#include <QSettings>

namespace {

constexpr auto kArray = "accounts";
constexpr auto kCurrentKey = "accountsCurrent";

SipAccount::Transport toTransport(int value)
{
    switch (value) {
    case int(SipAccount::Transport::Tcp): return SipAccount::Transport::Tcp;
    case int(SipAccount::Transport::Tls): return SipAccount::Transport::Tls;
    default:                              return SipAccount::Transport::Udp;
    }
}

SipAccount readAccount(const QSettings &settings)
{
    SipAccount account;
    account.displayName = settings.value("displayName").toString();
    account.username = settings.value("username").toString();
    account.domain = settings.value("domain").toString();
    account.password = settings.value("password").toString();
    account.transport = toTransport(settings.value("transport", 0).toInt());
    return account;
}

} // namespace

AccountManager::AccountManager(Storage storage, QObject *parent)
    : QAbstractListModel(parent)
    , m_persistent(storage == Storage::Persistent)
{
    if (m_persistent) {
        load();
    } else {
        m_accounts = { { QStringLiteral("Demo User"), QStringLiteral("1001"),
                         QStringLiteral("pbx.example.com"), QString(), SipAccount::Transport::Udp } };
        m_current = 0;
    }
}

int AccountManager::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : count();
}

QVariant AccountManager::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index, CheckIndexOption::IndexIsValid | CheckIndexOption::ParentIsInvalid))
        return {};

    const SipAccount &account = m_accounts.at(index.row());
    switch (role) {
    case Qt::DisplayRole:
    case LabelRole: return label(account);
    case UriRole:   return account.isValid() ? account.uri() : QString();
    }
    return {};
}

QHash<int, QByteArray> AccountManager::roleNames() const
{
    return { { LabelRole, "label" }, { UriRole, "uri" } };
}

SipAccount AccountManager::current() const
{
    return m_current >= 0 && m_current < m_accounts.size() ? m_accounts.at(m_current) : SipAccount{};
}

QString AccountManager::displayName() const
{
    const SipAccount account = current();
    return account.displayName.isEmpty() ? account.username : account.displayName;
}

QVariantMap AccountManager::get(int index) const
{
    if (index < 0 || index >= m_accounts.size())
        return {};
    const SipAccount &account = m_accounts.at(index);
    return {
        { QStringLiteral("displayName"), account.displayName },
        { QStringLiteral("username"), account.username },
        { QStringLiteral("domain"), account.domain },
        { QStringLiteral("password"), account.password },
        { QStringLiteral("transport"), int(account.transport) },
    };
}

int AccountManager::add()
{
    const int row = count();
    beginInsertRows({}, row, row);
    m_accounts.append(SipAccount{ tr("New account"), {}, {}, {}, SipAccount::Transport::Udp });
    endInsertRows();
    emit countChanged();
    if (m_current < 0) {
        m_current = row;
        emit currentChanged();
    }
    save();
    return row;
}

void AccountManager::update(int index, const QString &displayName, const QString &username,
                            const QString &domain, const QString &password, int transport)
{
    if (index < 0 || index >= m_accounts.size())
        return;

    SipAccount &account = m_accounts[index];
    account.displayName = displayName.trimmed();
    account.username = username.trimmed();
    account.domain = domain.trimmed();
    account.password = password;
    account.transport = toTransport(transport);
    save();

    const QModelIndex changed = this->index(index);
    emit dataChanged(changed, changed);
    if (index == m_current) {
        emit currentChanged();
        emit activeAccountChanged();
    }
}

void AccountManager::remove(int index)
{
    if (index < 0 || index >= m_accounts.size())
        return;

    beginRemoveRows({}, index, index);
    m_accounts.removeAt(index);
    endRemoveRows();
    emit countChanged();

    if (index == m_current) {
        m_current = m_accounts.isEmpty() ? -1 : 0;
        emit currentChanged();
        emit activeAccountChanged();
    } else if (index < m_current) {
        --m_current;
        emit currentChanged();
    }
    save();
}

void AccountManager::activate(int index)
{
    if (index == m_current || index < 0 || index >= m_accounts.size())
        return;
    m_current = index;
    save();
    emit currentChanged();
    emit activeAccountChanged();
}

QString AccountManager::label(const SipAccount &account)
{
    if (!account.isValid())
        return account.displayName.isEmpty() ? tr("New account") : account.displayName;
    const QString address = QStringLiteral("%1@%2").arg(account.username, account.domain);
    return account.displayName.isEmpty() ? address : QStringLiteral("%1 (%2)").arg(account.displayName, address);
}

void AccountManager::load()
{
    QSettings settings;
    const int size = settings.beginReadArray(QLatin1String(kArray));
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        m_accounts.append(readAccount(settings));
    }
    settings.endArray();

    const int stored = settings.value(QLatin1String(kCurrentKey), 0).toInt();
    m_current = m_accounts.isEmpty() ? -1 : qBound(0, stored, int(m_accounts.size()) - 1);
}

void AccountManager::save() const
{
    if (!m_persistent)
        return;

    QSettings settings;
    settings.beginWriteArray(QLatin1String(kArray), int(m_accounts.size()));
    for (int i = 0; i < m_accounts.size(); ++i) {
        const SipAccount &account = m_accounts.at(i);
        settings.setArrayIndex(i);
        settings.setValue("displayName", account.displayName);
        settings.setValue("username", account.username);
        settings.setValue("domain", account.domain);
        settings.setValue("password", account.password);
        settings.setValue("transport", int(account.transport));
    }
    settings.endArray();
    settings.setValue(QLatin1String(kCurrentKey), m_current);
}
