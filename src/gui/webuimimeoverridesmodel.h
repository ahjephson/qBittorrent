#pragma once

#include <QAbstractTableModel>

#include "base/webuimimetypestore.h"

class WebUIMimeOverridesModel final : public QAbstractTableModel
{
    Q_OBJECT

public:
    struct Entry
    {
        QString extension;
        QString contentType;
    };

    explicit WebUIMimeOverridesModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = {}) const override;
    int columnCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    void load();
    bool apply();

    bool isDirty() const;
    bool addEntry(const Entry &entry);
    bool updateEntry(int row, const Entry &entry);
    bool removeRows(int row, int count, const QModelIndex &parent = {}) override;
    Entry entryAt(int row) const;
    bool hasExtension(const QString &extension, int ignoreRow = -1) const;

signals:
    void dirtyChanged(bool dirty);

private:
    Entry normalizedEntry(const Entry &entry) const;
    void setDirty(bool dirty);

    QList<Entry> m_entries;
    bool m_dirty = false;
};
