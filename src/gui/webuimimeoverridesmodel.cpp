#include "webuimimeoverridesmodel.h"

#include <algorithm>

using namespace Qt::StringLiterals;

namespace
{
    constexpr int ColumnExtension = 0;
    constexpr int ColumnContentType = 1;
}

WebUIMimeOverridesModel::WebUIMimeOverridesModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    load();
}

int WebUIMimeOverridesModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_entries.size();
}

int WebUIMimeOverridesModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return 2;
}

QVariant WebUIMimeOverridesModel::data(const QModelIndex &index, const int role) const
{
    if (!index.isValid() || (index.row() >= m_entries.size()))
        return {};

    const Entry &entry = m_entries.at(index.row());

    if ((role == Qt::DisplayRole) || (role == Qt::EditRole))
    {
        switch (index.column())
        {
        case ColumnExtension:
            return (entry.extension.isEmpty() ? QString() : (u"."_s + entry.extension));
        case ColumnContentType:
            return entry.contentType;
        default:
            break;
        }
    }

    return {};
}

QVariant WebUIMimeOverridesModel::headerData(const int section, const Qt::Orientation orientation, const int role) const
{
    if ((orientation != Qt::Horizontal) || (role != Qt::DisplayRole))
        return {};

    switch (section)
    {
    case ColumnExtension:
        return tr("Extension");
    case ColumnContentType:
        return tr("Content type");
    default:
        break;
    }

    return {};
}

Qt::ItemFlags WebUIMimeOverridesModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

void WebUIMimeOverridesModel::load()
{
    beginResetModel();

    m_entries.clear();
    const auto overrides = WebUIMimeTypeStore::instance().overrides();
    m_entries.reserve(overrides.size());
    for (auto it = overrides.cbegin(); it != overrides.cend(); ++it)
        m_entries.append({it.key(), it.value()});

    std::sort(m_entries.begin(), m_entries.end(), [](const Entry &left, const Entry &right)
    {
        return left.extension < right.extension;
    });

    endResetModel();
    setDirty(false);
}

bool WebUIMimeOverridesModel::apply()
{
    if (!m_dirty)
        return true;

    WebUIMimeTypeStore::OverridesMap overrides;
    overrides.reserve(m_entries.size());
    for (const Entry &entry : std::as_const(m_entries))
        overrides.insert(entry.extension, entry.contentType);

    WebUIMimeTypeStore::instance().setOverrides(overrides);
    setDirty(false);
    return true;
}

bool WebUIMimeOverridesModel::isDirty() const
{
    return m_dirty;
}

bool WebUIMimeOverridesModel::addEntry(const Entry &entry)
{
    const Entry normalized = normalizedEntry(entry);
    if (normalized.extension.isEmpty() || normalized.contentType.isEmpty())
        return false;

    if (hasExtension(normalized.extension))
        return false;

    const int newRow = m_entries.size();
    beginInsertRows({}, newRow, newRow);
    m_entries.append(normalized);
    endInsertRows();
    setDirty(true);
    return true;
}

bool WebUIMimeOverridesModel::updateEntry(const int row, const Entry &entry)
{
    if ((row < 0) || (row >= m_entries.size()))
        return false;

    const Entry normalized = normalizedEntry(entry);
    if (normalized.extension.isEmpty() || normalized.contentType.isEmpty())
        return false;

    if (hasExtension(normalized.extension, row))
        return false;

    m_entries[row] = normalized;
    const QModelIndex topLeft = index(row, 0);
    const QModelIndex bottomRight = index(row, columnCount() - 1);
    emit dataChanged(topLeft, bottomRight);
    setDirty(true);
    return true;
}

bool WebUIMimeOverridesModel::removeRows(const int row, const int count, const QModelIndex &parent)
{
    if (parent.isValid() || (row < 0) || (count <= 0) || ((row + count) > m_entries.size()))
        return false;

    beginRemoveRows(parent, row, row + count - 1);
    for (int i = 0; i < count; ++i)
        m_entries.removeAt(row);
    endRemoveRows();
    setDirty(true);
    return true;
}

WebUIMimeOverridesModel::Entry WebUIMimeOverridesModel::entryAt(const int row) const
{
    if ((row < 0) || (row >= m_entries.size()))
        return {};
    return m_entries.at(row);
}

bool WebUIMimeOverridesModel::hasExtension(const QString &extension, const int ignoreRow) const
{
    for (int i = 0; i < m_entries.size(); ++i)
    {
        if ((i == ignoreRow) || m_entries.at(i).extension.compare(extension, Qt::CaseInsensitive))
            continue;
        return true;
    }

    return false;
}

WebUIMimeOverridesModel::Entry WebUIMimeOverridesModel::normalizedEntry(const Entry &entry) const
{
    Entry normalized;
    normalized.extension = WebUIMimeTypeStore::normalizeExtension(entry.extension);
    normalized.contentType = entry.contentType.trimmed();
    return normalized;
}

void WebUIMimeOverridesModel::setDirty(const bool dirty)
{
    if (m_dirty == dirty)
        return;

    m_dirty = dirty;
    emit dirtyChanged(m_dirty);
}
