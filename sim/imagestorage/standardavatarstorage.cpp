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
#include "log.h"

#include "imagestorage.h"

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

static const QString AvatarScheme = "avatar://";

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
    log(L_DEBUG, "StandardAvatarStorage::addAvatar(%s)", qPrintable(contactId.toString()));
    saveImage(makeFilename(contactId, type), image);
}

QImage StandardAvatarStorage::getAvatar(const IMContactId& contactId, const QString& type)
{
    QImage img = getFile(makeUri(contactId, type));
    if(img.isNull())
        return getImageStorage()->image("defavatar");
    return img;
}

QString StandardAvatarStorage::id() const
{
    return "standard-avatar-storage";
}

bool StandardAvatarStorage::hasIcon(const QString& iconId)
{
    if(!iconId.startsWith(AvatarScheme))
        return false;
    return true;
}

QIcon StandardAvatarStorage::icon(const QString& iconId)
{
    return QIcon(pixmap(iconId));
}

QImage StandardAvatarStorage::image(const QString& iconId)
{
    if(!iconId.startsWith(AvatarScheme))
        return QImage();
    QString id = iconId;
    id.remove(0, AvatarScheme.length());

    QImage img = getFile(id);
    if(img.isNull())
        return getImageStorage()->image("defavatar");
    return img;
}

QPixmap StandardAvatarStorage::pixmap(const QString& iconId)
{
    return QPixmap::fromImage(image(iconId));
}


bool StandardAvatarStorage::saveImage(const QString& path, const QImage& image)
{
    log(L_DEBUG, "StandardAvatarStorage::saveImage(%s)", qPrintable(path));
    return image.save(path);
}

QImage StandardAvatarStorage::loadImage(const QString& path)
{
    QImage img;
    img.load(path);
    return img;
}

QImage StandardAvatarStorage::getFile(const QString& id)
{
    QString cacheEntryId = id;
    if(d->hasImage(cacheEntryId))
        return d->get(cacheEntryId);
    QImage image = loadImage(basePath() + id);
    d->insert(cacheEntryId, image);
    return image;
}

QString StandardAvatarStorage::makeFilename(const IMContactId& id, const QString& type)
{
    return basePath() + makeUri(id, type) + ".png"; // FIXME extension hardcoded
}

QString StandardAvatarStorage::makeUri(const IMContactId& id, const QString& type)
{
    QString str = id.toString();
    str.replace('/', '.');
    str = str.toLower();
    if(!type.isEmpty())
        str.prepend(type + '.');
    return str;
}

QString StandardAvatarStorage::basePath() const
{
    return getProfileManager()->profilePath() + QDir::separator() + "pictures" + QDir::separator();
}

} /* namespace SIM */
