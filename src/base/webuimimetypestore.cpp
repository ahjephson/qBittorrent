#include "webuimimetypestore.h"

#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QStringList>
#include <QStringView>
#include <algorithm>

#include "logger.h"
#include "profile.h"
#include "utils/io.h"

using namespace Qt::StringLiterals;

const QString WebUIMimeTypeStore::CONFIG_FILE_NAME = u"webui_mime_types.json"_s;

WebUIMimeTypeStore &WebUIMimeTypeStore::instance()
{
    static WebUIMimeTypeStore store;
    return store;
}

WebUIMimeTypeStore::OverridesMap WebUIMimeTypeStore::overrides() const
{
    return m_overrides;
}

void WebUIMimeTypeStore::setOverrides(const OverridesMap &overrides)
{
    OverridesMap normalizedOverrides;
    normalizedOverrides.reserve(overrides.size());

    for (auto it = overrides.cbegin(); it != overrides.cend(); ++it)
    {
        const QString extension = normalizeExtension(it.key());
        const QString contentType = it.value().trimmed();

        if (extension.isEmpty() || contentType.isEmpty())
            continue;

        normalizedOverrides.insert(extension, contentType);
    }

    if (normalizedOverrides == m_overrides)
        return;

    m_overrides = normalizedOverrides;
    store();
}

std::optional<WebUIMimeTypeStore::OverrideEntry> WebUIMimeTypeStore::mimeTypeForPath(const Path &path) const
{
    const QFileInfo fileInfo {path.data()};
    if (!fileInfo.exists())
        return std::nullopt;

    return mimeTypeForExtension(fileInfo.suffix());
}

std::optional<WebUIMimeTypeStore::OverrideEntry> WebUIMimeTypeStore::mimeTypeForExtension(const QString &extension) const
{
    const QString normalizedExtension = normalizeExtension(extension);
    if (normalizedExtension.isEmpty())
        return std::nullopt;

    const auto it = m_overrides.constFind(normalizedExtension);
    if (it == m_overrides.constEnd())
        return std::nullopt;

    OverrideEntry entry;
    entry.contentType = it.value();
    entry.mimeType = m_mimeDatabase.mimeTypeForName(entry.contentType);
    return entry;
}

WebUIMimeTypeStore::WebUIMimeTypeStore()
{
    load();
}

void WebUIMimeTypeStore::load()
{
    const Path path = specialFolderLocation(SpecialFolder::Config) / Path(CONFIG_FILE_NAME);
    const auto readResult = Utils::IO::readFile(path, MAX_CONFIG_FILESIZE);

    if (!readResult)
    {
        if (readResult.error().status != Utils::IO::ReadError::NotExist)
        {
            LogMsg(tr("Failed to load WebUI MIME overrides. %1").arg(readResult.error().message), Log::WARNING);
        }
        return;
    }

    QJsonParseError jsonError;
    const QJsonDocument jsonDoc = QJsonDocument::fromJson(readResult.value(), &jsonError);
    if (jsonError.error != QJsonParseError::NoError)
    {
        LogMsg(tr("Failed to parse WebUI MIME overrides. %1").arg(jsonError.errorString()), Log::WARNING);
        return;
    }

    if (!jsonDoc.isObject())
    {
        LogMsg(tr("Failed to load WebUI MIME overrides. Invalid data format."), Log::WARNING);
        return;
    }

    OverridesMap overrides;
    const QJsonObject jsonObj = jsonDoc.object();
    for (auto it = jsonObj.constBegin(); it != jsonObj.constEnd(); ++it)
    {
        const QString extension = normalizeExtension(it.key());
        const QString contentType = it.value().toString().trimmed();
        if (extension.isEmpty() || contentType.isEmpty())
            continue;

        overrides.insert(extension, contentType);
    }

    m_overrides = overrides;
}

void WebUIMimeTypeStore::store() const
{
    QJsonObject jsonObj;
    QStringList keys = m_overrides.keys();
    std::sort(keys.begin(), keys.end());
    for (const QString &extension : keys)
        jsonObj[extension] = m_overrides.value(extension);

    const Path path = specialFolderLocation(SpecialFolder::Config) / Path(CONFIG_FILE_NAME);
    const QByteArray data = QJsonDocument(jsonObj).toJson(QJsonDocument::Indented);
    if (const auto saveResult = Utils::IO::saveToFile(path, data); !saveResult)
    {
        LogMsg(tr("Couldn't store WebUI MIME overrides to %1. Error: %2")
            .arg(path.toString(), saveResult.error()), Log::WARNING);
    }
}

QString WebUIMimeTypeStore::normalizeExtension(QStringView extension)
{
    QString normalized = extension.toString().trimmed().toLower();
    while (normalized.startsWith(u'.'))
        normalized.remove(0, 1);
    return normalized;
}
