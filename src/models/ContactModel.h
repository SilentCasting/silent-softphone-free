// SPDX-FileCopyrightText: 2026 Ihor Martseniuk and Silent Softphone Free contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QAbstractListModel>
#include <QList>
#include <QSortFilterProxyModel>

struct Contact
{
    QString name;
    QString number;
    QString presence; // "online" | "busy" | "away" | "offline"
};

class ContactModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Role { NameRole = Qt::UserRole + 1, NumberRole, PresenceRole };

    explicit ContactModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void addContact(const Contact &contact);
    void removeContact(int row);
    QString nameForNumber(const QString &number) const;

private:
    QList<Contact> m_contacts;
};

// Sorted, searchable view over ContactModel; this is what QML binds to.
class ContactFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(QString filterText READ filterText WRITE setFilterText NOTIFY filterTextChanged)

public:
    explicit ContactFilterModel(ContactModel *source, QObject *parent = nullptr);

    QString filterText() const { return m_filterText; }
    void setFilterText(const QString &text);

    Q_INVOKABLE void addContact(const QString &name, const QString &number);
    Q_INVOKABLE void removeAt(int row);

signals:
    void filterTextChanged();

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

private:
    ContactModel *m_source;
    QString m_filterText;
};
