/*
 * standardavatarstorage.cpp
 *
 *  Created on: Jul 2, 2011
 *      Author: todin
 */

#include <QDir>
#include <QImage>

#include "standardavatarstorage.h"
#include "profilemanager.h"

namespace SIM
{

class StandardAvatarStoragePimpl
{
    QMap<QString, QImage> m_cache;
public:

    void insert(const QString& id, const QImage& image)
    {
        // TODO implement eviction
        m_cache.insert(id, image);
    }

    bool hasImage(const QString& id)
    {
        QMap<QString, QImage>::iterator it = m_cache.find(id);
        if(it == m_cache.end())
            return false;
        return true;
    }

    QImage get(const QString& id)
    {
        QMap<QString, QImage>::iterator it = m_cache.find(id);
        if(it == m_cache.end())
            return QImage();
        return it.value();
    }
};

StandardAvatarStorage::StandardAvatarStorage()
{
    d = new StandardAvatarStoragePimpl();
}

StandardAvatarStorage::~StandardAvatarStorage()
{
    delete d;
}

void StandardAvatarStorage::addAvatar(const IMContactId& contactId, const QImage& image, const QString& type)
{
    saveImage(makeFilename(contactId, type), image);
}

QImage StandardAvatarStorage::getAvatar(const IMContactId& contactId, const QString& type)
{
    QString cacheEntryId = contactId.toString() + type;
    if(d->hasImage(cacheEntryId))
        return d->get(cacheEntryId);
    QImage image = loadImage(makeFilename(contactId, type));
    d->insert(cacheEntryId, image);
    return image;
}

QString StandardAvatarStorage::id() const
{
    return "standard-avatar-storage";
}

bool StandardAvatarStorage::hasIcon(const QString& iconId)
{
    return false;
}

QIcon StandardAvatarStorage::icon(const QString& iconId)
{
    return QIcon();
}

QImage StandardAvatarStorage::image(const QString& iconId)
{
    return QImage();
}

QPixmap StandardAvatarStorage::pixmap(const QString& iconId)
{
    return QPixmap();
}

QString StandardAvatarStorage::parseSmiles(const QString& input)
{
    return QString();
}

bool StandardAvatarStorage::saveImage(const QString& path, const QImage& image)
{
    return true;
}

QImage StandardAvatarStorage::loadImage(const QString& path)
{
    return QImage();
}

QString StandardAvatarStorage::makeFilename(const IMContactId& id, const QString& type)
{
    QString basePath = getProfileManager()->profilePath() + QDir::separator() + "pictures" + QDir::separator();
    QString str = id.toString();
    str.replace('/', '.');
    str = str.toLower();
    if(!type.isEmpty())
        str.prepend(type + '.');
    return basePath + str;
}

} /* namespace SIM */
