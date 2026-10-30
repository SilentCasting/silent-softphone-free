// SPDX-FileCopyrightText: 2026 Ihor Martseniuk and Silent Softphone Free contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "models/ContactModel.h"

ContactModel::ContactModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_contacts{
          { QStringLiteral("Olena Kovalenko"), QStringLiteral("1002"), QStringLiteral("online") },
          { QStringLiteral("Andrii Shevchenko"), QStringLiteral("1003"), QStringLiteral("busy") },
          { QStringLiteral("Reception"), QStringLiteral("100"), QStringLiteral("online") },
          { QStringLiteral("Support Line"), QStringLiteral("700"), QStringLiteral("away") },
          { QStringLiteral("Maria Bondar"), QStringLiteral("+380501112233"), QStringLiteral("offline") },
      }
{
}

int ContactModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : int(m_contacts.size());
}

QVariant ContactModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index, CheckIndexOption::IndexIsValid | CheckIndexOption::ParentIsInvalid))
        return {};

    const Contact &contact = m_contacts.at(index.row());
    switch (role) {
    case Qt::DisplayRole:
    case NameRole:     return contact.name;
    case NumberRole:   return contact.number;
    case PresenceRole: return contact.presence;
    }
    return {};
}

QHash<int, QByteArray> ContactModel::roleNames() const
{
    return {
        { NameRole, "name" },
        { NumberRole, "number" },
        { PresenceRole, "presence" },
    };
}

void ContactModel::addContact(const Contact &contact)
{
    const int row = int(m_contacts.size());
    beginInsertRows({}, row, row);
    m_contacts.append(contact);
    endInsertRows();
}

void ContactModel::removeContact(int row)
{
    if (row < 0 || row >= m_contacts.size())
        return;
    beginRemoveRows({}, row, row);
    m_contacts.removeAt(row);
    endRemoveRows();
}

QString ContactModel::nameForNumber(const QString &number) const
{
    for (const Contact &contact : m_contacts) {
        if (contact.number == number)
            return contact.name;
    }
    return {};
}

ContactFilterModel::ContactFilterModel(ContactModel *source, QObject *parent)
    : QSortFilterProxyModel(parent)
    , m_source(source)
{
    setSourceModel(source);
    setSortRole(ContactModel::NameRole);
    setSortCaseSensitivity(Qt::CaseInsensitive);
    setDynamicSortFilter(true);
    sort(0);
}

void ContactFilterModel::setFilterText(const QString &text)
{
    if (m_filterText == text)
        return;
    m_filterText = text;
    invalidateFilter();
    emit filterTextChanged();
}

void ContactFilterModel::addContact(const QString &name, const QString &number)
{
    const QString trimmedNumber = number.trimmed();
    if (trimmedNumber.isEmpty())
        return;
    const QString trimmedName = name.trimmed();
    m_source->addContact({ trimmedName.isEmpty() ? trimmedNumber : trimmedName,
                           trimmedNumber, QStringLiteral("offline") });
}

void ContactFilterModel::removeAt(int row)
{
    const QModelIndex sourceIndex = mapToSource(index(row, 0));
    if (sourceIndex.isValid())
        m_source->removeContact(sourceIndex.row());
}

bool ContactFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    if (m_filterText.isEmpty())
        return true;
    const QModelIndex idx = sourceModel()->index(sourceRow, 0, sourceParent);
    const QString name = idx.data(ContactModel::NameRole).toString();
    const QString number = idx.data(ContactModel::NumberRole).toString();
    return name.contains(m_filterText, Qt::CaseInsensitive)
        || number.contains(m_filterText, Qt::CaseInsensitive);
}
