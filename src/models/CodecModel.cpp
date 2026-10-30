// SPDX-FileCopyrightText: 2026 Ihor Martseniuk and Silent Softphone Free contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "models/CodecModel.h"

#include "core/ISipService.h"

#include <QSettings>

#include <algorithm>

namespace {

constexpr auto kOrderKey = "codecs/order";
constexpr auto kDisabledKey = "codecs/disabled";
constexpr int kTopPriority = 200; // below PJMEDIA_CODEC_PRIO_HIGHEST, leaves room above

// Defaults: wideband G.722, then G.711 (what PBXs reliably support).
// Everything else is listed but off until the user enables it.
const QStringList &defaultCodecs()
{
    static const QStringList codecs = {
        QStringLiteral("G722/16000/1"),
        QStringLiteral("PCMA/8000/1"),
        QStringLiteral("PCMU/8000/1"),
    };
    return codecs;
}

// "PCMA/8000/1" -> "PCMA"
QString codecName(const QString &id)
{
    return id.section(u'/', 0, 0);
}

// "G722/16000/1" -> "16 kHz", "opus/48000/2" -> "48 kHz stereo"
QString codecRate(const QString &id)
{
    const double khz = id.section(u'/', 1, 1).toDouble() / 1000.0;
    const bool stereo = id.section(u'/', 2, 2).toInt() > 1;
    QString text = QStringLiteral("%1 kHz").arg(khz);
    if (stereo)
        text += QStringLiteral(" stereo");
    return text;
}

} // namespace

CodecModel::CodecModel(ISipService *sip, bool persistent, QObject *parent)
    : QAbstractListModel(parent)
    , m_sip(sip)
    , m_persistent(persistent)
{
    QList<SipCodec> codecs = sip->codecs();
    std::stable_sort(codecs.begin(), codecs.end(),
                     [](const SipCodec &a, const SipCodec &b) { return a.priority > b.priority; });
    for (const SipCodec &codec : codecs) {
        // PJSIP lists G.722.1 once per bitrate under the same id; priorities apply per id.
        if (findEntry(codec.id) < 0)
            m_entries.append({ codec.id, codec.priority > 0 });
    }

    if (!restoreSaved())
        applyDefaults();
    apply();
}

int CodecModel::findEntry(const QString &id) const
{
    for (int i = 0; i < m_entries.size(); ++i) {
        if (m_entries.at(i).id == id)
            return i;
    }
    return -1;
}

// Saved order first; codecs that are new to this build keep their default state at the end.
bool CodecModel::restoreSaved()
{
    if (!m_persistent)
        return false;
    const QSettings settings;
    const QStringList order = settings.value(kOrderKey).toStringList();
    if (order.isEmpty())
        return false;
    const QStringList disabled = settings.value(kDisabledKey).toStringList();

    QList<Entry> ordered;
    for (const QString &id : order) {
        const int at = findEntry(id);
        if (at >= 0) {
            ordered.append({ id, !disabled.contains(id) });
            m_entries.removeAt(at);
        }
    }
    m_entries = ordered + m_entries;
    return true;
}

void CodecModel::applyDefaults()
{
    QList<Entry> preferred;
    for (const QString &id : defaultCodecs()) {
        const int at = findEntry(id);
        if (at >= 0) {
            preferred.append({ id, true });
            m_entries.removeAt(at);
        }
    }
    if (preferred.isEmpty())
        return; // unfamiliar codec set: keep the stack's own defaults
    for (Entry &entry : m_entries)
        entry.enabled = false;
    m_entries = preferred + m_entries;
}

int CodecModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : int(m_entries.size());
}

QVariant CodecModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index, CheckIndexOption::IndexIsValid | CheckIndexOption::ParentIsInvalid))
        return {};

    const Entry &entry = m_entries.at(index.row());
    switch (role) {
    case CodecIdRole:      return entry.id;
    case Qt::DisplayRole:
    case NameRole:         return codecName(entry.id);
    case RateRole:         return codecRate(entry.id);
    case CodecEnabledRole: return entry.enabled;
    }
    return {};
}

QHash<int, QByteArray> CodecModel::roleNames() const
{
    // "codecEnabled", not "enabled": the latter would shadow Item.enabled in QML delegates.
    return {
        { CodecIdRole, "codecId" },
        { NameRole, "name" },
        { RateRole, "rate" },
        { CodecEnabledRole, "codecEnabled" },
    };
}

void CodecModel::setEnabled(int row, bool enabled)
{
    if (row < 0 || row >= m_entries.size() || m_entries.at(row).enabled == enabled)
        return;

    const QModelIndex changed = index(row);
    const auto enabledCount = std::count_if(m_entries.cbegin(), m_entries.cend(), [](const Entry &e) { return e.enabled; });
    if (!enabled && enabledCount <= 1) {
        emit dataChanged(changed, changed); // keep at least one codec; reset the checkbox
        return;
    }

    m_entries[row].enabled = enabled;
    emit dataChanged(changed, changed);
    apply();
    save();
}

void CodecModel::move(int from, int to)
{
    if (from == to || from < 0 || to < 0 || from >= m_entries.size() || to >= m_entries.size())
        return;
    beginMoveRows({}, from, from, {}, to > from ? to + 1 : to);
    m_entries.move(from, to);
    endMoveRows();
    apply();
    save();
}

void CodecModel::apply() const
{
    int priority = kTopPriority;
    for (const Entry &entry : m_entries)
        m_sip->setCodecPriority(entry.id, entry.enabled ? qMax(1, priority--) : 0);
}

void CodecModel::save() const
{
    if (!m_persistent)
        return;
    QStringList order;
    QStringList disabled;
    for (const Entry &entry : m_entries) {
        order.append(entry.id);
        if (!entry.enabled)
            disabled.append(entry.id);
    }
    QSettings settings;
    settings.setValue(kOrderKey, order);
    settings.setValue(kDisabledKey, disabled);
}
