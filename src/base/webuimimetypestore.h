#pragma once

#include <optional>

#include <QHash>
#include <QMimeDatabase>
#include <QMimeType>
#include <QObject>
#include <QString>
#include <QStringView>

#include "path.h"

class WebUIMimeTypeStore final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(WebUIMimeTypeStore)

public:
    struct OverrideEntry
    {
        QString contentType;
        QMimeType mimeType;
    };

    using OverridesMap = QHash<QString, QString>;

    static WebUIMimeTypeStore &instance();

    OverridesMap overrides() const;
    void setOverrides(const OverridesMap &overrides);

    std::optional<OverrideEntry> mimeTypeForPath(const Path &path) const;
    std::optional<OverrideEntry> mimeTypeForExtension(const QString &extension) const;
    static QString normalizeExtension(QStringView extension);

private:
    WebUIMimeTypeStore();

    void load();
    void store() const;

    static constexpr int MAX_CONFIG_FILESIZE = 128 * 1024;
    static const QString CONFIG_FILE_NAME;

    OverridesMap m_overrides;
    mutable QMimeDatabase m_mimeDatabase;
};
